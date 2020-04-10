#include "scanner.h"

#include <QtGlobal>
#include <QtCore/QDebug>
#include <QtCore/QThread>
#include <QtCore/QStringLiteral>
#include <QtCore/QStringList>
#include <QtCore/QProcess>
#include <QtCore/QFileInfo>
#include <QtCore/QRegExp>
#include <QtCore/QDir>
#include <QtCore/QTimerEvent>
#include <clamav.h>
#include "application.h"
#include "infectedfile.h"
#include "scannerheuristicmatch.h"

// how long to wait for a running scan to abort before forcing it in the destructor - comes into play when the
// application closes (i.e. user clicks close button) while a scan is in progress
#define QLAM_SCANNER_DESTROY_WAIT_TIMEOUT 20000

using namespace Qlam;

static constexpr uint32_t DefaultGeneralScanOptions =
    static_cast<uint32_t>(CL_SCAN_GENERAL_ALLMATCHES) |                    // scan in all-match mode
    static_cast<uint32_t>(CL_SCAN_GENERAL_COLLECT_METADATA) |              // collect metadata (--gen-json)
    static_cast<uint32_t>(CL_SCAN_GENERAL_HEURISTICS) |                    // option to enable heuristic alerts
//    static_cast<uint32_t>(CL_SCAN_GENERAL_HEURISTIC_PRECEDENCE) |        // allow heuristic match to take precedence.
    static_cast<uint32_t>(CL_SCAN_GENERAL_UNPRIVILEGED)                    // scanner will not have read access to files.
    ;

static constexpr uint32_t DefaultParseScanOptions =
    static_cast<uint32_t>(CL_SCAN_PARSE_ARCHIVE) |
    static_cast<uint32_t>(CL_SCAN_PARSE_ELF) |
    static_cast<uint32_t>(CL_SCAN_PARSE_PDF) |
    static_cast<uint32_t>(CL_SCAN_PARSE_SWF) |
    static_cast<uint32_t>(CL_SCAN_PARSE_HWP3) |
    static_cast<uint32_t>(CL_SCAN_PARSE_XMLDOCS) |
    static_cast<uint32_t>(CL_SCAN_PARSE_MAIL) |
    static_cast<uint32_t>(CL_SCAN_PARSE_OLE2) |
    static_cast<uint32_t>(CL_SCAN_PARSE_HTML) |
    static_cast<uint32_t>(CL_SCAN_PARSE_PE)
    ;

static constexpr uint32_t DefaultHeuristicScanOptions =
    static_cast<uint32_t>(CL_SCAN_HEURISTIC_BROKEN) |                     // alert on broken PE and broken ELF files
    static_cast<uint32_t>(CL_SCAN_HEURISTIC_EXCEEDS_MAX) |                // alert when files exceed scan limits (filesize, max scansize, or max recursion depth)
    static_cast<uint32_t>(CL_SCAN_HEURISTIC_PHISHING_SSL_MISMATCH) |      // alert on SSL mismatches
    static_cast<uint32_t>(CL_SCAN_HEURISTIC_PHISHING_CLOAK) |             // alert on cloaked URLs in emails
    static_cast<uint32_t>(CL_SCAN_HEURISTIC_MACROS) |                     // alert on OLE2 files containing macros
    static_cast<uint32_t>(CL_SCAN_HEURISTIC_ENCRYPTED_ARCHIVE) |          // alert if archive is encrypted (rar, zip, etc)
    static_cast<uint32_t>(CL_SCAN_HEURISTIC_ENCRYPTED_DOC) |              // alert if a document is encrypted (pdf, docx, etc)
    static_cast<uint32_t>(CL_SCAN_HEURISTIC_PARTITION_INTXN)              // alert if partition table size doesn't make sense
//    static_cast<uint32_t>(CL_SCAN_HEURISTIC_STRUCTURED) |               // data loss prevention options, i.e. alert when detecting personal information
//    static_cast<uint32_t>(CL_SCAN_HEURISTIC_STRUCTURED_SSN_NORMAL) |    // alert when detecting social security numbers
//    static_cast<uint32_t>(CL_SCAN_HEURISTIC_STRUCTURED_SSN_STRIPPED)    // alert when detecting stripped social security numbers
    ;

static constexpr uint32_t DefaultMailScanOptions = static_cast<uint32_t>(CL_SCAN_MAIL_PARTIAL_MESSAGE);

static const auto HeuristicMatchPrefix = QStringLiteral("Heuristics."); // NOLINT(cert-err58-cpp)

// error return codes for countFiles()
static const int ErrPathDoesNotExist = -1;
static const int ErrUncountablePath = -2;

Scanner::Scanner( const QString & scanPath, QObject * parent )
: Scanner(QStringList() << scanPath, parent) {
}

Scanner::Scanner( const QStringList & scanPaths, QObject * parent )
: QThread(parent),
  m_scanPaths(),
  m_scannedDirs(),
  m_countedDirs(),
  m_issues(),
  m_fileCount(),
  m_scannedFileCount(0),
  m_failedScanCount(0),
  m_scannedDataSize(0),
  m_scanEngine(nullptr),
  m_abortFlag(false) {
	setScanPaths(scanPaths);
    connect(Application::instance(), &Application::aboutToQuit, this, &Scanner::abort);
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
				m_scanEngine = nullptr;
				Application::instance()->releaseEngine();
			}
		}
	}
}


void Scanner::scanEntity(const QFileInfo & path) {
	if(!path.exists()) {
		Q_EMIT pathNotFound(path.filePath());
		return;
	}

	if(path.isDir()) {
		QFileInfo myPath(path);

		while(myPath.isSymLink()) {
			myPath.setFile(myPath.symLinkTarget());
		}

		if(m_scannedDirs.containsPath(myPath.absoluteFilePath())) {
			return;
		}

		m_scannedDirs.addPath(myPath.absoluteFilePath());
		QFileInfoList entries = QDir(path.filePath()).entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files, QDir::DirsFirst | QDir::Name | QDir::IgnoreCase | QDir::LocaleAware);

		for(const auto & entry : entries) {
			if(m_abortFlag) {
				Q_EMIT scanAborted();
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
	Q_ASSERT_X(m_scanEngine != nullptr, "Scanner::scanFile()", "called with no scan engine");
	const char * virusName;

	struct cl_scan_options opts {
	    DefaultGeneralScanOptions,
	    DefaultParseScanOptions,
	    DefaultHeuristicScanOptions,
	    DefaultMailScanOptions,
	    0,  // disable all dev-only options
	};

	int ret = cl_scanfile(QDir::toNativeSeparators(path.canonicalFilePath()).toUtf8(), &virusName, &m_scannedDataSize, m_scanEngine, &opts);
    Q_EMIT fileScanned(path.filePath());

	if(CL_CLEAN == ret) {
		Q_EMIT fileClean(path.filePath());
		++m_scannedFileCount;
	}
	else if(CL_VIRUS == ret) {
		FileWithIssues inf(path.filePath());
		QString qstrVirusName = QString::fromUtf8(virusName);
		inf.addIssue(qstrVirusName);
		m_issues.append(FileWithIssues(path.filePath()));

		if (qstrVirusName.startsWith(HeuristicMatchPrefix)) {
		    ScannerHeuristicMatch heuristic = ScannerHeuristicMatch::Generic;

            if (QStringLiteral("Heuristics.Limits.Exceeded") == qstrVirusName) {
                heuristic = ScannerHeuristicMatch::ExceedsMaximum;
            } else if (qstrVirusName.startsWith(QStringLiteral("Heuristics.Broken."))) {
                heuristic = ScannerHeuristicMatch::BrokenExecutable;
            } else if (QStringLiteral("Heuristics.Encrypted.Zip") == qstrVirusName) {
                heuristic = ScannerHeuristicMatch::EncryptedArchive;
            } else if (QStringLiteral("Heuristics.OLE2.ContainsMacros") == qstrVirusName) {
                heuristic = ScannerHeuristicMatch::OleMacros;
            } else if (qstrVirusName.startsWith(QStringLiteral("Heuristics.OLE2."))) {
                heuristic = ScannerHeuristicMatch::OleGeneric;
            } else if (QStringLiteral("Heuristics.Phishing.Email.SpoofedDomain") == qstrVirusName) {
                heuristic = ScannerHeuristicMatch::PhishingEmailSpoofedDomain;
            } else if (qstrVirusName.startsWith(QStringLiteral("Heuristics.Phishing."))) {
                heuristic = ScannerHeuristicMatch::PhishingGeneric;
            } else if (QStringLiteral("Heuristics.Structured.CreditCardNumber") == qstrVirusName) {
                heuristic = ScannerHeuristicMatch::StructuredCreditCardNumber;
            } else if (QStringLiteral("Heuristics.Structured.SSN") == qstrVirusName) {
                heuristic = ScannerHeuristicMatch::StructuredSsnNormal;
            } else if (qstrVirusName.startsWith(QStringLiteral("Heuristics.Structured."))) {
                heuristic = ScannerHeuristicMatch::StructuredGeneric;
            } else {
                qDebug() << "Unrecognised heuristic issue string" << qstrVirusName << "please file a bug report";
            }

            Q_EMIT
            fileMatchedHeuristic(path.filePath(), heuristic);
        } else {
            Q_EMIT fileInfected(path.filePath(), qstrVirusName);
		}

		++m_scannedFileCount;
	}
	else {
qDebug() << "failure when scanning" << path.filePath();
		++m_failedScanCount;
	}
}

int Scanner::countFiles(const QFileInfo & path) {
    if (!path.exists()) {
        qDebug() << "path" << path.filePath() << "does not exist";
        return ErrPathDoesNotExist;
    }

    if (path.isDir()) {
        QFileInfo myPath(path);

        while (myPath.isSymLink()) {
            myPath.setFile(myPath.symLinkTarget());
        }

        if (m_countedDirs.containsPath(myPath.absoluteFilePath())) {
            qDebug() << "ultimate target of" << path.filePath() << "(" << myPath.absoluteFilePath()
                     << ") already counted";
            return 0;
        }

        m_countedDirs.addPath(myPath.absoluteFilePath());
        QFileInfoList entries = QDir(path.filePath()).entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files,
                                                                    QDir::DirsFirst | QDir::Name |
                                                                    QDir::IgnoreCase | QDir::LocaleAware);

        int count = 0;
        for(const auto & entry : entries) {
            count += countFiles(entry);

            if (m_abortFlag) {
                return 0;
            }
        }

        return count;
    } else if (path.isFile()) {
        return 1;
    }

qDebug() << "unknown path" << path.filePath() << "(" << path.canonicalFilePath() << ")";
    return  ErrUncountablePath;
}

bool Scanner::startScan() {
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

	Q_EMIT scanStarted();
	m_scanEngine = app->acquireEngine();

	if(!m_scanEngine) {
		Q_EMIT scanFailed();
		Q_EMIT scanFinished();
		return;
	}

	for(const auto & path : scanPaths()) {
		scanEntity(QFileInfo(path));
	}

	if(m_abortFlag) {
		Q_EMIT scanAborted();
	}
	else if(0 < m_failedScanCount) {
		Q_EMIT scanFailed();
	}
	else {
		Q_EMIT scanComplete();
		Q_EMIT scanComplete(m_issues.count());

		// we only emit clean scan signal if scan completed successfully and there were no infections found.
		// if scan fails or is aborted, we don't emit this signal.
		if(0 == m_issues.count()) {
			Q_EMIT scanClean();
		}
	}

	if(0 < m_issues.count()) {
		Q_EMIT scanFoundInfections();
	}

	/* we don't need the tree of scanned dirs any more, it's only to prevent
	 * recursive scanning of circular symlinks */
	m_scannedDirs.clear();

	// file counter watches abort flag, so wait for it to resolve
	m_counter.wait();
	m_counter = {};
	m_abortFlag = false;

	Q_EMIT scanFinished();
	m_scanEngine = nullptr;
	app->releaseEngine();
}


void Scanner::abort() {
	m_abortFlag = true;
}


void Scanner::reset() {
	m_scannedDirs.clear();
	m_countedDirs.clear();
	m_issues.clear();
	m_scannedFileCount = 0;
	m_failedScanCount = 0;
	m_scannedDataSize = 0;
}


std::optional<int> Scanner::fileCount() const {
	return m_fileCount;
}


/**
 * Start the asynchronous task to count the files to scan.
 *
 * While this method includes code to try to gracefully exit any previous count task, it's not guaranteed. You should
 * try to be sure that any previous count has stopped before calling this.
 */
void Scanner::startFileCounter() {
    if (m_counter.valid() && std::future_status::timeout == m_counter.wait_for(std::chrono::seconds(3))) {
        qWarning() << "previous asynchronous task to calculate the file count did not exit gracefully.";
    }

    m_counter = std::async(std::launch::async, [this] () -> int {
        m_fileCount.reset();
        int count = 0;
        m_countedDirs.clear();

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
        m_countedDirs.clear();

        if (!m_abortFlag) {
            m_fileCount = count;
            Q_EMIT fileCountComplete(count);
        }

        return count;
    });
}
