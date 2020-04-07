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

	class Scanner
	: public QThread {

		Q_OBJECT

		public:
			typedef QList<InfectedFile> InfectionList;

			Scanner( const QString & scanPath = QString(), QObject * parent = nullptr );
			Scanner( const QStringList & scanPaths, QObject * parent = nullptr );
			virtual ~Scanner( void );

			const QStringList & scanPaths( void ) const {
				return m_scanPaths;
			}

			const QString & scanPath( int i = 0 ) const {
				static QString empty;

				if(0 > i || m_scanPaths.count() <= i) {
					return empty;
				}

				return m_scanPaths.at(i);
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

			bool isValid( void ) const;

			static Scanner * startScan( const QString & scanPath ) {
				return startScan(QStringList() << scanPath);
			}

			static Scanner * startScan( const QStringList & scanPaths ) {
				Scanner * ret = new Scanner(scanPaths);

				if(ret->isValid()) {
					ret->startScan();
				}
				else {
					delete ret;
					ret = (Scanner *) 0;
				}

				return ret;
			}

			void reset( void );

			int fileCount( void ) const;
			int infectedFileCount( void ) const {
				return m_infections.count();
			}

			int scannedFileCount( void ) const {
				 return m_scannedFileCount;
			}

			const InfectionList & infectedFiles( void ) const {
				return m_infections;
			}

			long int dataScanned( void ) const {
				return (long int) m_scannedDataSize;
			}

		Q_SIGNALS:
			/* emitted when a scan starts */
			void scanStarted( void );

			/* emitted when the async task to count the files to be scanned has completed */
			void fileCountComplete(int);

			/* emitted when a file is scanned */
			void fileScanned( const QString & path );

			/* emitted when a file is scanned and is found to be clean */
			void fileClean( const QString & path );

			/* emitted when a file is scanned and is found to be infected */
			void fileInfected( const QString & path );

			/* emitted when a file is scanned and is found to be infected */
			void fileInfected( const QString & path, const QString & infection );

			/* emitted when a file could not be scanned for some reason */
			void fileScanFailed( const QString & path );

			/* emitted when a scan completes successfully */
			void scanComplete( void );

			/* emitted when a scan completes successfully */
			void scanComplete( int infectedFiles );

			/* emitted when scan completes successfully and it found no infections */
			void scanClean( void );

			/* emitted when scan completes successfully and it found infections */
			void scanFoundInfections( void );

			/* scan did not complete successfully (NOT issued by abort())*/
			void scanFailed( void );

			/* scan finished because abort() was called */
			void scanAborted( void );

			/* for whetever reason, the scan is over */
			void scanFinished( void );

		public Q_SLOTS:
			bool startScan( void );
			void abort( void );

		protected:
//			void disposeScanEngine( void );
			virtual void run( void );

		protected Q_SLOTS:
//			virtual void timerEvent( QTimerEvent * ev );

		private:
	        void startFileCounter();
			static int countFiles( const QFileInfo & path );
			void scanEntity( const QFileInfo & path );
			void scanFile( const QFileInfo & path );

			static const int FileCountNotCalculated;
//			static int s_clamavInit;

			QStringList m_scanPaths;
			TreeItem m_scannedDirs;
			static TreeItem s_countedDirs;

			InfectionList m_infections;
			mutable int m_fileCount;
			int m_scannedFileCount;
			int m_failedScanCount;
			unsigned long int m_scannedDataSize;
			struct cl_engine * m_scanEngine;
			bool m_abortFlag;
			std::future<int> m_counter;
//			int m_engineDisposeTimer;
	};
}

#endif // QLAM_CLAMSCAN_H
