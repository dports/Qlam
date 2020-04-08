#ifndef QLAM_CLAMSCAN_H
#define QLAM_CLAMSCAN_H

#include <QtCore/QThread>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QFileInfo>
#include <QtCore/QList>
#include <QtCore/QMap>

#include "infectedfile.h"
#include "treeitem.h"

class QProcess;
struct cl_engine;

namespace Qlam {

    enum class ScannerHeuristicMatch;

	class Scanner
	: public QThread {

		Q_OBJECT

		public:
			using IssueList = QList<FileWithIssues>;

			explicit Scanner( const QString & = QString(), QObject * = nullptr );
			explicit Scanner( const QStringList &, QObject * = nullptr );
			~Scanner() override;

			const QStringList & scanPaths() const {
				return m_scanPaths;
			}

			const QString & scanPath(int idx = 0) const {
				static QString empty;

				if(0 > idx || m_scanPaths.count() <= idx) {
					return empty;
				}

				return m_scanPaths.at(idx);
			}

			void setScanPath( const QString & path ) {
				m_scanPaths.clear();
				m_scanPaths.append(path);
				m_fileCount = FileCountNotCalculated;
			}

			void setScanPaths( const QStringList & paths ) {
				m_scanPaths = paths;
				m_fileCount = FileCountNotCalculated;
			}

			bool isValid() const;

			static Scanner * startScan(const QString & scanPath) {
				return startScan(QStringList() << scanPath);
			}

			static Scanner * startScan(const QStringList & scanPaths) {
				auto * ret = new Scanner(scanPaths);

				if(ret->isValid()) {
					ret->startScan();
				} else {
					delete ret;
					ret = nullptr;
				}

				return ret;
			}

			void reset();
			std::future<int> fileCount() const;

			int infectedFileCount() const {
				return m_issues.count();
			}

			int scannedFileCount() const {
				 return m_scannedFileCount;
			}

			const IssueList & infectedFiles() const {
				return m_issues;
			}

			long long dataScanned() const {
				return (long long) m_scannedDataSize;
			}

        // TODO provide more information in fileInfected() signals (e.g. if heuristic rather than actual virus)
		Q_SIGNALS:
			/* emitted when a scan starts */
			void scanStarted();

			/* emitted when a file is scanned */
			void fileScanned( const QString & path );

			/* emitted when a file is scanned and is found to be clean */
			void fileClean( const QString & path );

			/* emitted when a file is scanned and is found to be infected */
			void fileInfected( const QString & path );

			/* emitted when a file is scanned and is found to be infected */
			void fileInfected( const QString & path, const QString & infection );

			/* emitted when a file is scanned and is found to match a heuristic rule */
			void fileMatchedHeuristic(const QString &, ScannerHeuristicMatch);

			/* emitted when a file could not be scanned for some reason */
			void fileScanFailed( const QString & path );

			/* emitted when a scan completes successfully */
			void scanComplete();

			/* emitted when a scan completes successfully */
			void scanComplete( int infectedFiles );

			/* emitted when scan completes successfully and it found no infections */
			void scanClean();

			/* emitted when scan completes successfully and it found infections */
			void scanFoundInfections();

			/* scan did not complete successfully (NOT issued by abort())*/
			void scanFailed();

			/* scan finished because abort() was called */
			void scanAborted();

			/* for whetever reason, the scan is over */
			void scanFinished();

		public Q_SLOTS:
			bool startScan();
			void abort();

		protected:
			void run() override;

		private:
			static int countFiles(const QFileInfo &);
			void scanEntity(const QFileInfo &);
			void scanFile(const QFileInfo &);

			static const int FileCountNotCalculated;

			QStringList m_scanPaths;
			TreeItem m_scannedDirs;
			static TreeItem s_countedDirs;

			IssueList m_issues;
			mutable int m_fileCount;
			int m_scannedFileCount;
			int m_failedScanCount;
			unsigned long m_scannedDataSize;
			struct cl_engine * m_scanEngine;
			bool m_abortFlag;
	};
}

#endif // QLAM_CLAMSCAN_H
