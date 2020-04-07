#include "scanner.h"

/** \todo
 * - after first use of scanner (i.e. first call to start()),
 *   subsequent scans (using the same paths) appear to scan all files but report
 *   that zero data was scanned. Is this just because the engine
 *   knows that nothing has changed? The engine is definitely reporting that
 *   each scanned file is clean and it is definitely being asked to scan all
 *   the same files.
 */
#include <QtGlobal>

#include <QtCore/QDebug>
#include <QtCore/QThread>
#include <QtCore/QByteArray>
#include <QtCore/QStringList>
#include <QtCore/QProcess>
#include <QtCore/QFileInfo>
#include <QtCore/QRegExp>
#include <QtCore/QDir>
#include <QtCore/QTimerEvent>

#include <clamav.h>

#include "application.h"
#include "infectedfile.h"
#include "treeitem.h"

/* how long to wait for a running scan to abort before forcing it
 * in the destructor - comes into play when the application closes
 * (i.e. user clicks close button) while a scan is in progress */
#define QLAM_SCANNER_DESTROY_WAIT_TIMEOUT 20000

using namespace Qlam;

// TODO use an optional instead
const int Scanner::FileCountNotCalculated = -1;
TreeItem Scanner::s_countedDirs;

Scanner::Scanner( const QString & scanPath, QObject * parent )
: QThread(parent),
  m_scanPaths(),
  m_scannedDirs(),
  m_infections(),
  m_fileCount(FileCountNotCalculated),
  m_scannedFileCount(0),
  m_failedScanCount(0),
  m_scannedDataSize(0),
  m_scanEngine(0),
  m_abortFlag(false) {
	setScanPath(scanPath);
	connect(Application::instance(), SIGNAL(aboutToQuit()), this, SLOT(abort()));
}

Scanner::Scanner( const QStringList & scanPaths, QObject * parent )
: QThread(parent),
  m_scanPaths(),
  m_scannedDirs(),
  m_infections(),
  m_fileCount(-1),
  m_scannedFileCount(0),
  m_failedScanCount(0),
  m_scannedDataSize(0),
  m_scanEngine(0),
  m_abortFlag(false) {
	setScanPaths(scanPaths);
	connect(Application::instance(), SIGNAL(aboutToQuit()), this, SLOT(abort()));
}

Scanner::~Scanner() {
	if(isRunning()) {
qDebug() << "scan in progress - attempting to abort";
		abort();

		/* wait up to 20 sec. for graceful exit */
		if(!wait(QLAM_SCANNER_DESTROY_WAIT_TIMEOUT)) {
qDebug() << "scan did not abort after" << QLAM_SCANNER_DESTROY_WAIT_TIMEOUT << "ms - destroying anyway (program will probably crash)";

			if(m_scanEngine) {
qDebug() << "forcing release of scan engine";
				m_scanEngine = 0;
				Application::instance()->releaseEngine();
			}
		}
	}
}


void Scanner::scanEntity( const QFileInfo & path ) {
	if(!path.exists()) {
		qDebug() << "path" << path.filePath() << "does not exist";
		return;
	}

//	qDebug() << "scanning" << path.filePath();

	if(path.isDir()) {
		QFileInfo myPath(path);

		while(myPath.isSymLink()) {
			myPath.setFile(myPath.symLinkTarget());
		}

		if(m_scannedDirs.containsPath(myPath.absoluteFilePath())) {
qDebug() << "ultimate target of" << path.filePath() << "(" << myPath.absoluteFilePath() << ") already scanned";
			return;
		}

		m_scannedDirs.addPath(myPath.absoluteFilePath());
		QFileInfoList entries = QDir(path.filePath()).entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files, QDir::DirsFirst | QDir::Name | QDir::IgnoreCase | QDir::LocaleAware);

		for(const auto & entry : entries) {
			if(m_abortFlag) {
				Q_EMIT(scanAborted());
				return;
			}

			scanEntity(entry);
		}
	}
	else if(path.isFile()) {
		scanFile(path);
	}
	else {
		qDebug() << "unknown path" << path.filePath() << "(" << path.canonicalFilePath() << ")";
	}
}

void Scanner::scanFile( const QFileInfo & path ) {
	Q_ASSERT_X(!!m_scanEngine, "Scanner::scanFile()", "called with no scan engine");
	const char * virusName;
	struct cl_scan_options opts {
	    static_cast<uint32_t>(~0),
	    static_cast<uint32_t>(~0),
	    static_cast<uint32_t>(~0),
	    static_cast<uint32_t>(~0),
	    static_cast<uint32_t>(~0),
	};

	int ret = cl_scanfile(QDir::toNativeSeparators(path.canonicalFilePath()).toUtf8(), &virusName, &m_scannedDataSize, m_scanEngine, &opts);

	if(CL_CLEAN == ret) {
		Q_EMIT(fileScanned(path.filePath()));
		Q_EMIT(fileClean(path.filePath()));
		++m_scannedFileCount;
	}
	else if(CL_VIRUS == ret) {
		InfectedFile inf(path.filePath());
		QString myVirusName = QString::fromUtf8(virusName);
		inf.addVirus(myVirusName);
		m_infections.append(InfectedFile(path.filePath()));
		Q_EMIT(fileScanned(path.filePath()));
		Q_EMIT(fileInfected(path.filePath()));
		Q_EMIT(fileInfected(path.filePath(), myVirusName));

		++m_scannedFileCount;
	}
	else {
qDebug() << "failure when scanning" << path.filePath();
		Q_EMIT(fileScanFailed(path.filePath()));
		++m_failedScanCount;
	}
}


int Scanner::countFiles( const QFileInfo & path ) {
	if(!path.exists()) {
qDebug() << "path" << path.filePath() << "does not exist";
		return -1;
	}

	if(path.isDir()) {
		/* TODO this is a temporary fix for the file count blocking in the GUI
		 * thread. this is no way to resolve the issue properly! */
		qApp->processEvents();
		QFileInfo myPath(path);

		while(myPath.isSymLink()) {
			myPath.setFile(myPath.symLinkTarget());
		}

		if(s_countedDirs.containsPath(myPath.absoluteFilePath())) {
qDebug() << "ultimate target of" << path.filePath() << "(" << myPath.absoluteFilePath() << ") already counted";
			return 0;
		}

		s_countedDirs.addPath(myPath.absoluteFilePath());
		QFileInfoList entries = QDir(path.filePath()).entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files, QDir::DirsFirst | QDir::Name | QDir::IgnoreCase | QDir::LocaleAware);
		int count = 0;

		for(QFileInfo entry : entries) {
			int myCount = countFiles(entry);

			if(0 < myCount) {
				count += myCount;
			}
		}

		return count;
	}
	else if(path.isFile()) {
		return 1;
	}

qDebug() << "unknown path" << path.filePath() << "(" << path.canonicalFilePath() << ")";
	return -2;
}

bool Scanner::isValid() const {
	/* TODO do we want to do this or just emit an invalidPath() signal during scan if a path
	 * does not exist? */

	/* check scan paths are valid */
	for(const auto & path : scanPaths()) {
		QFileInfo fi(path);

		if(fi.path().isEmpty() || !fi.exists()) {
qDebug() << "empty or non-existent scan path" << path;
			return false;
		}
	}

	/* TODO check any other options are valid */
	return true;
}


bool Scanner::startScan() {
	if(!isValid()) {
qDebug() << "scan is not valid";
		return false;
	}

	if(isRunning()) {
qDebug() << "scan is already running";
		return false;
	}

	start();
	return true;
}


void Scanner::run() {
	reset();
	startFileCounter();
	Application * app = Application::instance();

	Q_EMIT(scanStarted());
	m_scanEngine = app->acquireEngine();

	if(!m_scanEngine) {
qDebug() << "failed to acquire scan engine";
		Q_EMIT(scanFailed());
		Q_EMIT(scanFinished());
		return;
	}

	for(const auto & path : scanPaths()) {
		scanEntity(QFileInfo(path));
	}

	if(m_abortFlag) {
		Q_EMIT(scanAborted());
	}
	else if(0 < m_failedScanCount) {

		Q_EMIT(scanFailed());
	}
	else {
qDebug() << "emitting scanComplete()";
		Q_EMIT(scanComplete());
		Q_EMIT(scanComplete(m_infections.count()));

		/* we only emit clean scan signal if scan completed successfully and there
		 * were no infections found. if scan fails or is aborted, we don't emit
		 * this signal. */
		if(0 == m_infections.count()) {
			Q_EMIT(scanClean());
		}
	}

	if(0 < m_infections.count()) {
		Q_EMIT(scanFoundInfections());
	}

	/* we don't need the tree of scanned dirs any more, it's only to prevent
	 * recursive scanning of circular symlinks */
	m_scannedDirs.clear();

	// file counter watches abort flag, so wait for it to resolve
	m_counter.wait();
	m_counter = {};
	m_abortFlag = false;

	Q_EMIT(scanFinished());
	m_scanEngine = 0;
	app->releaseEngine();
}


void Scanner::abort() {
	m_abortFlag = true;
}


void Scanner::reset() {
	m_scannedDirs.clear();
	s_countedDirs.clear();
	m_infections.clear();
	m_scannedFileCount = 0;
	m_failedScanCount = 0;
	m_scannedDataSize = 0;
}


int Scanner::fileCount() const {
	return m_fileCount;
}


/**
 * Start the asynchronous task to count the files to scan.
 *
 * While this method includes code to try to gracefully exit any previous count task, it's not guaranteed. You should
 * try to be sure that any previous count has stopped before calling this.
 */
void Scanner::startFileCounter() {
    if (m_counter.valid() && std::future_status::timeout != m_counter.wait_for(std::chrono::seconds(3))) {
        // TODO log a warning that we're releasing a running asynchronous task
    }

    m_counter = std::async(std::launch::async, [this] () -> int {
        m_fileCount = FileCountNotCalculated;
        int count = 0;
        s_countedDirs.clear();

        for(const auto & path : scanPaths()) {
            int countForPath = countFiles(QFileInfo(path));

            if(0 < countForPath) {
                count += countForPath;
            }

            if (m_abortFlag) {
                break;
            }
        }

        // these are only required while the count is in progress,  so we can safely clear here
        s_countedDirs.clear();

        if (m_abortFlag) {
            m_fileCount = FileCountNotCalculated;
        } else {
            m_fileCount = count;
            Q_EMIT fileCountComplete(count);
        }
    });
}
