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
	class ScanProfile;
	enum class ScannerHeuristicMatch;

	class ScanWidget
	: public QWidget {

		Q_OBJECT

		public:
			static const int IndeterminateProgress;

			explicit ScanWidget(QWidget * = nullptr);
			~ScanWidget() override;

			inline QString scanPath(int idx = 0) const {
				if(0 > idx || scanPathCount() <= idx) {
					return QString();
				}

				return scanPaths().at(idx);
			}

			inline void setScanPath(const QString & path) {
				clearScanPaths();
				addScanPath(path);
			}

			bool setScanPath(int, const QString &);

			QStringList scanPaths() const;
			void clearScanPaths();
			void addScanPath(const QString &);
			int scanPathCount() const;

			void setScanProfile(const ScanProfile &);

		Q_SIGNALS:
			void scanPathsChanged();
			void scanButtonClicked();
			void saveProfileButtonClicked();
			void scanStarted();
			void scanFinished();

		protected:
            void updateScanDuration();
			void dragEnterEvent(QDragEnterEvent *) override;
			void dropEvent(QDropEvent *) override;
			void timerEvent(QTimerEvent *) override;

		public Q_SLOTS:
			void chooseScanFiles();
			void chooseScanDirectory();
			void removeSelectedScanPaths();

			void doScan();
			void abortScan();
			void showScanOutput() {
				 setScanOutputVisible(true);
			}

			void hideScanOutput() {
				 setScanOutputVisible(false);
			}

			void setScanOutputVisible(bool vis);

			void setScanStatus(const QString &);
			void clearScanOutput();
			void setScanProgress(int);
			void addIssue(const QString &path, const QString &virus);
			void addMatchedHeuristic(const QString &, ScannerHeuristicMatch);

		private Q_SLOTS:
			void addFailedFileScan(const QString &);
			void slotScannerScannedFile();
			void slotScanSucceeded();
			void slotScanFailed();
			void slotScanAborted();
			void slotScanFinished();
			void slotScanPathsSelectionChanged();

		private:
			std::unique_ptr<Ui::ScanWidget> m_ui;
			Scanner * m_scanner;
			int m_scanDuration;
			int m_scanDurationTimer;

        QString currentDurationString() const;
    };
}

#endif // QLAM_SCANSETTINGSWIDGET_H
