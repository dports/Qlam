#ifndef QLAM_SCANSETTINGSWIDGET_H
#define QLAM_SCANSETTINGSWIDGET_H

#include <QtGlobal>

#include <QtCore/QTimer>
#include <QtWidgets/QWidget>

class QDragEnterEvent;
class QDropEvent;
class QTimerEvent;

namespace Ui {
	class ScanWidget;
}

namespace Qlam {
	class Scanner;
	class TextElider;
	class ScanProfile;

	class ScanWidget
	: public QWidget {

		Q_OBJECT

		public:
			static const int IndeterminateProgress;

			explicit ScanWidget(QWidget *parent = nullptr);
			~ScanWidget();

			inline QString scanPath( int i = 0 ) const {
				if(0 > i || scanPathCount() <= i) {
					return QString();
				}

				return scanPaths().at(i);
			}

			inline void setScanPath( const QString & path ) {
				clearScanPaths();
				addScanPath(path);
			}

			bool setScanPath( int i, const QString & path );

			QStringList scanPaths( void ) const;
			void clearScanPaths( void );
			void addScanPath( const QString & path );
			int scanPathCount( void ) const;

			void setScanProfile( const ScanProfile & profile );

		Q_SIGNALS:
			void scanPathsChanged( void );
			void scanButtonClicked( void );
			void saveProfileButtonClicked( void );
			void scanStarted( void );
			void scanFinished( void );

		protected:
			virtual void dragEnterEvent( QDragEnterEvent * ev );
			virtual void dropEvent( QDropEvent * ev );
			virtual void timerEvent( QTimerEvent * ev );

		public Q_SLOTS:
			void chooseScanFiles( void );
			void chooseScanDirectory( void );
			void removeSelectedScanPaths();

			void doScan( void );
			void abortScan( void );
			void showScanOutput( void ) {
				 setScanOutputVisible(true);
			}

			void hideScanOutput( void ) {
				 setScanOutputVisible(false);
			}

			void setScanOutputVisible( bool vis );

			void setScanStatus( const QString & text );
			void clearScanOutput( void );
			void setScanProgress( int pc );
			void addInfection( const QString & path, const QString & virus );

		private Q_SLOTS:
			void addFailedFileScan( const QString & path );
			void slotScannerScannedFile( void );
			void slotScanSucceeded( void );
			void slotScanFailed( void );
			void slotScanAborted( void );
			void slotScanFinished( void );
			void slotScanPathsSelectionChanged( void );

		private:
			static QString encodeNameForSaving( QString name );
			static QString deodeNameFromFile( QString name );

			Ui::ScanWidget *ui;
			Scanner * m_scanner;
			int m_scanDuration;
			int m_scanDurationTimer;
	};
}

#endif // QLAM_SCANSETTINGSWIDGET_H
