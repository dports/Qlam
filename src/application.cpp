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

Application * Application::s_instance = nullptr;

Application::Application(int & argc, char ** argv)
: QApplication(argc, argv),
  m_scanProfiles(),
  m_clamavInit(0),
  m_scanEngine(nullptr),
  m_engineLockCount(0),
  m_engineDisposeTimer(0),
  m_settings(nullptr) {
	qRegisterMetaType<Qlam::DatabaseInfo>("DatabaseInfo");

	if(s_instance) {
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
	connect(this, &Application::aboutToQuit, this, &Application::writeScanProfiles);
}

Application::~Application() {
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

int Application::exec() {
	readScanProfiles();
	return QApplication::exec();
}

struct cl_engine * Application::acquireEngine() {
	if(!clamAvInitialised()) {
		return nullptr;
	}

	if(m_engineDisposeTimer) {
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

		int ret = cl_load(path.data(), m_scanEngine, &sigs, CL_DB_STDOPT); // NOLINT(hicpp-signed-bitwise)

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

void Application::releaseEngine() {
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

void Application::timerEvent(QTimerEvent * ev) {
	if(0 != m_engineDisposeTimer && ev->timerId() == m_engineDisposeTimer) {
		disposeEngine();
	}
}

void Application::disposeEngine() {
	Q_ASSERT(0 == m_engineLockCount);

	if(m_engineDisposeTimer) {
		killTimer(m_engineDisposeTimer);
		m_engineDisposeTimer = 0;
	}

	if(m_scanEngine) {
qDebug() << "disposing scan engine" << ((void *) m_scanEngine);
		cl_engine_free(m_scanEngine);
	}

	m_scanEngine = nullptr;
}

void Application::addScanProfile(ScanProfile * profile) {
	m_scanProfiles.append(profile);
	Q_EMIT scanProfileAdded(profile->name());
	Q_EMIT scanProfileAdded(m_scanProfiles.count() - 1);
}

ScanProfile Application::scanProfile(int idx) const {
	Q_ASSERT(idx >= 0 && idx < m_scanProfiles.count());
	return *(m_scanProfiles.at(idx));
}

QString Application::clamAvVersion() {
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

bool Application::clamAvInitialised() const {
	return CL_SUCCESS == m_clamavInit;
}

QString Application::systemDatabasePath() {
	return QString::fromLatin1(cl_retdbdir());
}

QList<DatabaseInfo> Application::databases() {
	QString path = settings()->databasePath();

	if(path.isEmpty()) {
		path = Application::systemDatabasePath();
	}

	QList<DatabaseInfo> ret;

	if(!path.isEmpty()) {
		for(const auto & fi : QDir(path).entryInfoList(QStringList() << "*.cvd" << "*.cld", QDir::Files | QDir::NoDotAndDotDot | QDir::Readable)) {
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

void Application::readScanProfiles() {
	QSettings settings;

	if(!settings.value("haveProfiles").isValid()) {
		/* no attempt has ever been made to write a set of user profiles (even an
		 * empty one, so we set up just one default one for the user's home
		 * directory */
		auto * profile = new ScanProfile("Home directory");
		profile->addPath(QDir::homePath());
		addScanProfile(profile);
	}
	else {
		int profileCount = settings.beginReadArray("scanprofiles");

		for(int idx = 0; idx < profileCount; ++idx) {
			settings.setArrayIndex(idx);
			auto * profile = new ScanProfile(settings.value("name").toString());
			profile->setPaths(settings.value("paths").toStringList());
			addScanProfile(profile);
		}

		settings.endArray();
	}
}

void Application::writeScanProfiles() {
	QSettings settings;
	settings.setValue("haveProfiles", "1");
	settings.beginWriteArray("scanprofiles");

	for(int idx = 1; idx < m_scanProfiles.count(); ++idx) {
		settings.setArrayIndex(idx - 1);
		ScanProfile * profile = m_scanProfiles.at(idx);
		settings.setValue("name", profile->name());
		settings.setValue("paths", profile->paths());
	}

	settings.endArray();
}
