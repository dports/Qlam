/** \file application.cpp
 *
 * \brief The main application object.
 *
 * The application object provides access to a shared scan engine. This is
 * because the scan engine uses a fair amount of system resources (e.g. RAM)
 * so having an engine shared between several (potential) concurrent scans
 * is more efficient than each scan creating its own instance of a scan
 * engine. Scanner objects gain access to the engine by calling
 * acquireEngine() and must release it by calling releaseEngine() when they
 * no longer need to use it. This ensures that the application can keep track
 * of how the engine is being used and release its resources if it hasn't
 * been used for a while. The acquireEngine() method will take care of
 * creating a new engine if there is currently no active engine. Scanner
 * objects are entitled to assume that the acquired engine will remain valid
 * until they call releaseEngine(). Scanner objects MUST call releaseEngine()
 * when they have finished using it. From the point at which releaseEngine()
 * is called, Scanner objects can no longer use the engine and must assume
 * that it has been deallocated.
 */
#include "application.h"

#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtCore/QDir>
#include <QtCore/QProcess>

#include "scanprofile.h"
#include <clamav.h>

using namespace Qlam;

/* time in ms between final lock on scan engine being released
 * the engine resources being freed (unless another lock is acquired).
 * this ought to stay in the region of minutes or more unless resources
 * are very scarce on the target platform */
#define QLAM_APPLICATION_SCAN_ENGINE_DISPOSE_TIMEOUT 300000 /* 5 mins */


Application * Application::s_instance = (Application *) nullptr;


Application::Application( int & argc, char ** argv )
: QApplication(argc, argv),
  m_scanProfiles(),
  m_clamavInit(0),
  m_scanEngine(0),
  m_engineLockCount(0),
  m_engineDisposeTimer(0),
  m_settings(0) {
	qRegisterMetaType<Qlam::DatabaseInfo>("DatabaseInfo");

	if(!!s_instance) {
		qCritical() << "Application instance already created (" << ((void *) s_instance) << ")";
	}

	s_instance = this;

	setOrganizationName("Equit");
	setOrganizationDomain("equituk.net");
	setApplicationName("Qlam");
	setApplicationDisplayName("Qlam");

	/* need to do this after setting the org name, domain and app name otherwise
	 * the settings will be read from a file whose path is based on the wrong org
	 * name/domain/appname */
	m_settings = new Settings(this);

	m_clamavInit = cl_init(CL_INIT_DEFAULT);

	if(CL_SUCCESS != m_clamavInit) {
		qDebug() << "failed to initialise libclamav";
	}

	m_scanProfiles.append(new ScanProfile(tr("Custom scan")));
	connect(this, SIGNAL(aboutToQuit()), this, SLOT(writeScanProfiles()));
}


Application::~Application( void ) {
	writeScanProfiles();
	qDeleteAll(m_scanProfiles);
	m_scanProfiles.clear();

	if(m_engineDisposeTimer) {
		killTimer(m_engineDisposeTimer);
		m_engineDisposeTimer = 0;
	}

	if(0 < m_engineLockCount) {
		qDebug() << "application destroyed before scan engine lock count reached 0";
		m_engineLockCount = 0;
	}

	disposeEngine();
}


int Application::exec( void ) {
	readScanProfiles();
	return QApplication::exec();
}


struct cl_engine * Application::acquireEngine( void ) {
	if(!clamAvInitialised()) {
		return (cl_engine *) 0;
	}

	if(!!m_engineDisposeTimer) {
qDebug() << "engine lock requested, stopping dispose timer";
		killTimer(m_engineDisposeTimer);
		m_engineDisposeTimer = 0;
	}

	if(!m_scanEngine) {
		Q_ASSERT(0 == m_engineLockCount);
		m_scanEngine = cl_engine_new();
		unsigned int sigs;
		QByteArray path;

		if(settings()->databasePath().isEmpty()) {
			path = cl_retdbdir();
		}
		else {
			path = QDir::toNativeSeparators(settings()->databasePath()).toLocal8Bit();
		}

		int ret = cl_load(path.data(), m_scanEngine, &sigs, CL_DB_STDOPT);

		if(CL_SUCCESS != ret) {
			qDebug() << "failed to load databases:" << cl_strerror(ret);
			disposeEngine();
			return m_scanEngine;
		}

		ret = cl_engine_compile(m_scanEngine);

		if(CL_SUCCESS != ret) {
			qDebug() << "failed to compile databases:" << cl_strerror(ret);
			disposeEngine();
			return m_scanEngine;
		}

qDebug() << "created scan engine at" << ((void *) m_scanEngine);
	}

	++m_engineLockCount;
qDebug() << "scan engine has" << m_engineLockCount << "locks";
qDebug() << "scan engine dispose timer is" << (0 == m_engineDisposeTimer ? "not" : "") << "running";
	return m_scanEngine;
}


void Application::releaseEngine( void ) {
	Q_ASSERT(0 < m_engineLockCount);
qDebug() << "releasing one lock on scan engine" << ((void *) m_scanEngine);
	--m_engineLockCount;

qDebug() << "scan engine has" << m_engineLockCount << "locks";
qDebug() << "scan engine dispose timer is" << (0 == m_engineDisposeTimer ? "not" : "") << "running";
	if(0 == m_engineLockCount) {
		Q_ASSERT(0 == m_engineDisposeTimer);

qDebug() << "no remaining engine locks, starting dispose timer";
		m_engineDisposeTimer = startTimer(QLAM_APPLICATION_SCAN_ENGINE_DISPOSE_TIMEOUT);
	}
}


void Application::timerEvent( QTimerEvent * ev ) {
	if(0 != m_engineDisposeTimer && ev->timerId() == m_engineDisposeTimer) {
		disposeEngine();
	}
}


void Application::disposeEngine( void ) {
	Q_ASSERT(0 == m_engineLockCount);

	if(m_engineDisposeTimer) {
		killTimer(m_engineDisposeTimer);
		m_engineDisposeTimer = 0;
	}

	if(m_scanEngine) {
qDebug() << "disposing scan engine" << ((void *) m_scanEngine);
		cl_engine_free(m_scanEngine);
	}

	m_scanEngine = (struct cl_engine *) 0;
}


void Application::addScanProfile( ScanProfile * profile ) {
	m_scanProfiles.append(profile);
	Q_EMIT(scanProfileAdded(profile->name()));
	Q_EMIT(scanProfileAdded(m_scanProfiles.count() - 1));
}


ScanProfile Application::scanProfile( int i ) const {
	Q_ASSERT(i >= 0 && i < m_scanProfiles.count());
	return *(m_scanProfiles.at(i));
}


QString Application::clamAvVersion( void ) {
	static QString s_version;

	if(s_version.isEmpty()) {
		QString clamav_config("clamav-config");
		QProcess p;
		p.start(clamav_config, QStringList() << "--version");
		p.waitForFinished();

		if(0 == p.exitCode()) {
			s_version = QString::fromUtf8(p.readAllStandardOutput().trimmed());
			qDebug() << "read" << s_version << "from clamav-config process";
		}
		else {
			qDebug() << "failed to execute" << clamav_config;
		}
	}

	return (s_version.isEmpty() ? tr("<unknown>") : s_version);
}


bool Application::clamAvInitialised( void ) const {
	return CL_SUCCESS == m_clamavInit;
}


QString Application::systemDatabasePath( void ) {
	return QString::fromLatin1(cl_retdbdir());
}


QList<DatabaseInfo> Application::databases( void ) {
	QString path = settings()->databasePath();

	if(path.isEmpty()) {
		path = systemDatabasePath();
	}

	QList<DatabaseInfo> ret;

	if(!path.isEmpty()) {
		for(QFileInfo fi : QDir(path).entryInfoList(QStringList() << "*.cvd" << "*.cld", QDir::Files | QDir::NoDotAndDotDot | QDir::Readable)) {
			DatabaseInfo inf(fi.absoluteFilePath());

			if(inf.isValid()) {
				ret.append(inf);
			}
			else {
				qDebug() << "database" << fi.absoluteFilePath() << "could not be read into DatabaseInfo object";
			}
		}
	}
	else {
		qDebug() << "no database path";
	}

	return ret;
}


void Application::readScanProfiles( void ) {
	QSettings settings;

	if(!settings.value("haveProfiles").isValid()) {
		/* no attempt has ever been made to write a set of user profiles (even an
		 * empty one, so we set up just one default one for the user's home
		 * directory */
		ScanProfile * p = new ScanProfile("Home directory");
		p->addPath(QDir::homePath());
		addScanProfile(p);
	}
	else {
		int n = settings.beginReadArray("scanprofiles");

		for(int i = 0; i < n; ++i) {
			settings.setArrayIndex(i);
			ScanProfile * p = new ScanProfile(settings.value("name").toString());
			p->setPaths(settings.value("paths").toStringList());
			addScanProfile(p);
		}

		settings.endArray();
	}
}

void Application::writeScanProfiles( void ) {
	QSettings settings;
	settings.setValue("haveProfiles", "1");
	settings.beginWriteArray("scanprofiles");

	for(int i = 1; i < m_scanProfiles.count(); ++i) {
		settings.setArrayIndex(i - 1);
		ScanProfile * p = m_scanProfiles.at(i);
		settings.setValue("name", p->name());
		settings.setValue("paths", p->paths());
	}

	settings.endArray();
}
