/**
 * @file scanwidget.cpp
  *
  * TODO option to automatically save scan report
  */

#include "scanwidget.h"
#include "ui/ui_scanwidget.h"

#include <QtGlobal>
#include <QtCore/QDebug>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtGui/QDragEnterEvent>
#include <QtCore/QMimeData>
#include <QtCore/QUrl>
#include <cmath>
#include <QtCore/QCoreApplication>

#include "qlam.h"
#include "application.h"
#include "scanner.h"
#include "scanprofile.h"
#include "scannerheuristicmatch.h"
#include "timedactiondialogue.h"

using namespace Qlam;

const int ScanWidget::IndeterminateProgress = -1;

ScanWidget::ScanWidget(QWidget *parent)
	: QWidget(parent),
      m_ui(std::make_unique<Ui::ScanWidget>()),
      m_scanner(QStringLiteral()),
      m_scanDuration(0),
      m_scanDurationTimer(0) {
	m_ui->setupUi(this);
	setAcceptDrops(true);
	hideScanOutput();
	slotScanPathsSelectionChanged();

	QFont f(m_ui->title->font());
	f.setPointSizeF(f.pointSizeF() * QLAM_TITLE_LABEL_FONT_POINT_SIZE_RATIO);
#if QLAM_TITLE_LABEL_FONT_BOLD
	f.setBold(true);
#endif
	m_ui->title->setFont(f);
	m_ui->issuesList->setHeaderLabels(QStringList() << tr("File path") << tr("Issue"));

	connect(m_ui->scanButton, &QPushButton::clicked, this, &ScanWidget::doScan);
	connect(m_ui->scanButton, &QPushButton::clicked, this, &ScanWidget::scanButtonClicked);
	connect(m_ui->abortButton, &QPushButton::clicked, this, &ScanWidget::abortScan);
	connect(m_ui->chooseScanDirectory, &QPushButton::clicked, this, &ScanWidget::chooseScanDirectory);
	connect(m_ui->chooseScanFiles, &QPushButton::clicked, this, &ScanWidget::chooseScanFiles);
	connect(m_ui->scanPaths, &QListWidget::itemSelectionChanged, this, &ScanWidget::slotScanPathsSelectionChanged);
	connect(m_ui->removeScanPath, &QPushButton::clicked, this, &ScanWidget::removeSelectedScanPaths);
	connect(m_ui->saveScanProfile, &QPushButton::clicked, this, &ScanWidget::saveProfileButtonClicked);

	// we use a blocking queued connection because all signals originate in the scanner thread (all are emitted after
	// Scanner::run() has been called and before it exits) and we need the slots to be called immediately otherwise they
	// can be called in an incorrect order (using a non-blocking queued connection). we also must ensure that the slots
	// are invoked in the receiver's thread (the GUI thread) because they update GUI components and this is not safe
	// outside the GUI thread.
	//
	// see http://qt-project.org/doc/qt-4.8/qt.html#ConnectionType-enum for details of the different connection types
	connect(&m_scanner, qOverload<>(&Scanner::scanComplete), this, &ScanWidget::slotScanSucceeded, Qt::BlockingQueuedConnection);
	connect(&m_scanner, &Scanner::scanFailed, this, &ScanWidget::slotScanFailed, Qt::BlockingQueuedConnection);
	connect(&m_scanner, &Scanner::scanAborted, this, &ScanWidget::slotScanAborted, Qt::BlockingQueuedConnection);
	connect(&m_scanner, &Scanner::scanFinished, this, &ScanWidget::slotScanFinished, Qt::BlockingQueuedConnection);
	connect(&m_scanner, &Scanner::scanFinished, this, &ScanWidget::scanFinished, Qt::BlockingQueuedConnection);
	connect(&m_scanner, &Scanner::pathNotFound, this, &ScanWidget::addPathNotFound, Qt::BlockingQueuedConnection);
	connect(&m_scanner, &Scanner::fileScanned, this, &ScanWidget::setScanStatus, Qt::BlockingQueuedConnection);
	connect(&m_scanner, &Scanner::fileScanned, this, &ScanWidget::slotScannerScannedFile, Qt::BlockingQueuedConnection);
	connect(&m_scanner, &Scanner::fileInfected, this, &ScanWidget::addIssue, Qt::BlockingQueuedConnection);
	connect(&m_scanner, &Scanner::fileMatchedHeuristic, this, &ScanWidget::addMatchedHeuristic, Qt::BlockingQueuedConnection);
	connect(&m_scanner, &Scanner::fileScanFailed, this, &ScanWidget::addFailedFileScan, Qt::BlockingQueuedConnection);
}

QStringList ScanWidget::scanPaths() const {
	QStringList paths;

	for(int idx = 0; idx < m_ui->scanPaths->count(); ++idx) {
		paths.append(m_ui->scanPaths->item(idx)->text());
	}

	return paths;
}

void ScanWidget::clearScanPaths() {
	m_ui->scanPaths->clear();
	Q_EMIT scanPathsChanged();
}

void ScanWidget::addScanPath( const QString & path ) {
	m_ui->scanPaths->addItem(path);
	Q_EMIT scanPathsChanged();
}

int ScanWidget::scanPathCount() const {
	return m_ui->scanPaths->count();
}

void ScanWidget::setScanProfile( const ScanProfile & profile ) {
	bool block = blockSignals(true);
	clearScanOutput();
	hideScanOutput();
	clearScanPaths();

	for(const auto & path : profile.paths()) {
		addScanPath(path);
	}

	m_ui->title->setText(tr("Scan: %1").arg(profile.name()));
	blockSignals(block);
	Q_EMIT scanPathsChanged();
}

void ScanWidget::dragEnterEvent( QDragEnterEvent * event ) {
	if(event->mimeData()->hasUrls()) {
		QList<QUrl> urls = event->mimeData()->urls();

		for(const auto & url : urls) {
			QString path = url.toLocalFile();

			if(!path.isEmpty()) {
				event->acceptProposedAction();
				return;
			}
		}

		/* if we don't find any local paths, the drag'n'drop is rejected - only
		 * local paths can be used for scan */
	}
}

void ScanWidget::dropEvent( QDropEvent * event ) {
	QList<QUrl> urls = event->mimeData()->urls();

	for(const auto & url : urls) {
		QString path = url.toLocalFile();
		if(!path.isEmpty()) {
			addScanPath(path);
		}
	}
}

void ScanWidget::timerEvent(QTimerEvent * event) {
	if(event->timerId() == m_scanDurationTimer) {
	    updateScanDuration();
	}
}

void ScanWidget::chooseScanFiles() {
	QString initialPath;

	if(!m_ui->scanPaths->selectedItems().empty()) {
        initialPath = m_ui->scanPaths->selectedItems()[0]->text();
	}

	QStringList paths = QFileDialog::getOpenFileNames(this, tr("Scan file"), initialPath);

	if(!paths.isEmpty()) {
        {
            QSignalBlocker block(this);

            for (const auto &path : paths) {
                addScanPath(path);
            }
        }

        Q_EMIT scanPathsChanged();
	}
}

void ScanWidget::chooseScanDirectory() {
	QString path;

	if(!m_ui->scanPaths->selectedItems().empty()) {
		path = m_ui->scanPaths->selectedItems()[0]->text();
	}

	path = QFileDialog::getExistingDirectory(this, tr("Scan directory"), path);

	if(!path.isEmpty()) {
		addScanPath(path);
	}
}

void ScanWidget::removeSelectedScanPaths() {
	QList<QListWidgetItem *> selectedItems = m_ui->scanPaths->selectedItems();

	if(0 < selectedItems.count()) {
		bool block = m_ui->scanPaths->blockSignals(true);

		for(auto * it : selectedItems) {
            delete it;
		}

		m_ui->scanPaths->blockSignals(block);

		if(!block) {
			slotScanPathsSelectionChanged();
		}

		Q_EMIT scanPathsChanged();
	}
}

void ScanWidget::doScan() {
	m_scanner.setScanPaths(scanPaths());
	clearScanOutput();
	showScanOutput();
	setScanProgress(ScanWidget::IndeterminateProgress);
	setScanStatus(tr("Initialising scan"));
	m_ui->timer->setText("--");
	m_scanDuration = 0;
	m_scanDurationTimer = startTimer(1000);

	if(m_scanner.startScan()) {
		m_ui->scanButton->setEnabled(false);
		m_ui->abortButton->setEnabled(true);
		Q_EMIT scanStarted();
	}
	else {
		qCritical() << "scanner failed to start";
		QMessageBox::critical(this, tr("Error"), tr("The scan could not be started."), tr("OK"));
		return;
	}
}

void ScanWidget::abortScan() {
	if(!m_scanner.isRunning()) {
		qDebug() << "scanner is not running";
		return;
	}

	m_scanner.abort();
}

void ScanWidget::setScanOutputVisible( bool vis ) {
    m_ui->scanProgress->setEnabled(vis);
    m_ui->abortButton->setEnabled(vis);
    m_ui->scanStatus->setEnabled(vis);
    m_ui->issuesListLabel->setEnabled(vis);
    m_ui->issuesList->setEnabled(vis);
}

void ScanWidget::setScanStatus( const QString & text ) {
	m_ui->scanStatus->setText(text);
}

void ScanWidget::clearScanOutput() {
	m_ui->scanProgress->setValue(0);
	m_ui->scanStatus->clear();
	m_ui->issuesList->clear();
	m_ui->issuesList->setHeaderHidden(true);
}

void ScanWidget::setScanProgress( int pc ) {
	if(IndeterminateProgress == pc) {
		m_ui->scanProgress->setMaximum(0);
	}
	else {
		m_ui->scanProgress->setMaximum(100);
		m_ui->scanProgress->setValue((0 > pc ? 0 : (100 < pc ? 100 : pc)));
	}
}

void ScanWidget::addIssue(const QString & path, const QString & virus ) {
    m_ui->issuesList->setHeaderHidden(false);
    m_ui->issuesList->addTopLevelItem(new QTreeWidgetItem(QStringList() << path << tr("Infection: %1").arg(virus)));
    m_ui->issuesList->resizeColumnToContents(0);
    m_ui->issuesList->resizeColumnToContents(1);
}

void ScanWidget::addPathNotFound(const QString & path) {
    m_ui->issuesList->setHeaderHidden(false);
    m_ui->issuesList->addTopLevelItem(new QTreeWidgetItem(QStringList() << path << tr("Not found.")));
    m_ui->issuesList->resizeColumnToContents(0);
    m_ui->issuesList->resizeColumnToContents(1);
}

void ScanWidget::addMatchedHeuristic(const QString & path, ScannerHeuristicMatch heuristic) {
    m_ui->issuesList->setHeaderHidden(false);
    QString heuristicName;
    
    switch (heuristic) {
        case ScannerHeuristicMatch::Generic:
            heuristicName = tr("Generic heuristic");
            break;

        case ScannerHeuristicMatch::BrokenExecutable:
            heuristicName = tr("Broken executable file");
            break;

        case ScannerHeuristicMatch::ExceedsMaximum:
            heuristicName = tr("Maximum file size, recursive scan depth or scan size exceeded");
            break;

        case ScannerHeuristicMatch::InvalidPartitionTableSize:
            heuristicName = tr("Invalid partition table size");
            break;

        case ScannerHeuristicMatch::PhishingEmailSpoofedDomain:
            heuristicName = tr("Spoofed domain in email (potential phishing attack)");
            break;

        case ScannerHeuristicMatch::PhishingSslMismatch:
            heuristicName = tr("SSL mismatch (potential phishing attack)");
            break;

        case ScannerHeuristicMatch::PhishingCloak:
            heuristicName = tr("Cloaked URL found (potential phishing attack)");
            break;

        case ScannerHeuristicMatch::PhishingGeneric:
            heuristicName = tr("Potential phishing attack");
            break;

        case ScannerHeuristicMatch::OleGeneric:
            heuristicName = tr("OLE");
            break;

        case ScannerHeuristicMatch::OleMacros:
            heuristicName = tr("OLE2 macros found in file");
            break;

        case ScannerHeuristicMatch::EncryptedArchive:
            heuristicName = tr("Password-protected archive could not be scanned");
            break;

        case ScannerHeuristicMatch::EncryptedDoc:
            heuristicName = tr("Password-protected document could not be scanned");
            break;

        case ScannerHeuristicMatch::EncryptedGeneric:
            heuristicName = tr("Password-protected file could not be scanned");
            break;

        case ScannerHeuristicMatch::StructuredCreditCardNumber:
            heuristicName = tr("Possible credit card number found in file");
            break;

        case ScannerHeuristicMatch::StructuredSsnNormal:
            heuristicName = tr("Possible social security number found in file");
            break;

        case ScannerHeuristicMatch::StructuredSsnStripped:
            heuristicName = tr("Possible stripped social security number found in file");
            break;

        case ScannerHeuristicMatch::StructuredGeneric:
            heuristicName = tr("Generic heuristic");
            break;
    }
    
    m_ui->issuesList->addTopLevelItem(new QTreeWidgetItem(QStringList() << path << tr("Heuristic match: %1").arg(heuristicName)));
    m_ui->issuesList->resizeColumnToContents(0);
    m_ui->issuesList->resizeColumnToContents(1);
}

void ScanWidget::addFailedFileScan( const QString & path ) {
	m_ui->issuesList->setHeaderHidden(false);
	m_ui->issuesList->addTopLevelItem(new QTreeWidgetItem(QStringList() << path << tr("Unable to scan")));
	m_ui->issuesList->resizeColumnToContents(0);
	m_ui->issuesList->resizeColumnToContents(1);
}

void ScanWidget::slotScannerScannedFile() {
	std::optional<int> fileCount = m_scanner.fileCount();

	if (!fileCount) {
	    setScanProgress(ScanWidget::IndeterminateProgress);
	    return;
	}

	int pc = int(100 * (static_cast<double>(m_scanner.scannedFileCount()) / static_cast<double>(fileCount.value())));
	setScanProgress(pc);
}

void ScanWidget::slotScanSucceeded() {
	long long kb = m_scanner.dataScanned();
    QLocale currentLocale;
	QString sizeDisplay;

	if(kb < 5120) {
		/* less than 5MB, express in KB */
		sizeDisplay = tr("%1 Kb").arg(currentLocale.toString(kb));
	}
	else if(kb < 1048576 * 5) {
		/* less than 5GB, express in MB */
		sizeDisplay = tr("%1 Mb").arg(currentLocale.toString(static_cast<double>(kb) / 1024, 'f', 2));
	}
	else {
		/* express in GB */
		sizeDisplay = tr("%1 Gb").arg(currentLocale.toString(static_cast<double>(kb) / 1048576, 'f', 2));
	}

	setScanStatus(tr("Scan finished in %4 (%1 issues found in %2 of data in %3 files)")
        .arg(currentLocale.toString(m_scanner.issueCount()))
        .arg(sizeDisplay)
        .arg(currentLocale.toString(m_scanner.scannedFileCount()))
        .arg(currentDurationString()));
    setScanProgress(100);

    if (m_ui->quitOnClean->isChecked() && 0 == m_scanner.issueCount()) {
        TimedActionDialogue::Action action = [this]() {
            if (m_scanner.isRunning()) {
                connect(&m_scanner, &QThread::finished, Application::instance(), &Application::quit);
            } else {
                Application::quit();
            }
        };

        auto * dlg = new TimedActionDialogue(tr("The scan was clean. Qlam will exit in %1s."), action, this);
        connect(dlg, &QDialog::finished, dlg, &QDialog::deleteLater);
        dlg->setWindowTitle(tr("Scan Clean"));
        dlg->setTimeout(10000);
        dlg->enableNow();
        dlg->setNowLabel(tr("Quit now"));
        dlg->show();
        dlg->start();
    }
}

void ScanWidget::slotScanFailed() {
	setScanStatus(tr("Scan failed"));
	addIssue("", tr("Scan failed."));
	setScanProgress(0);
}

void ScanWidget::slotScanAborted() {
	setScanStatus(tr("Scan aborted"));
	setScanProgress(0);
}

void ScanWidget::slotScanFinished() {
	m_ui->scanButton->setEnabled(true);
	m_ui->abortButton->setEnabled(false);
	killTimer(m_scanDurationTimer);
	m_scanDurationTimer = 0;
}

void ScanWidget::slotScanPathsSelectionChanged() {
	m_ui->removeScanPath->setEnabled(0 < m_ui->scanPaths->selectedItems().count());
}

void ScanWidget::updateScanDuration() {
    ++m_scanDuration;
    m_ui->timer->setText(currentDurationString());
}

QString ScanWidget::currentDurationString() const {
    static constexpr const int secondsPerHour = 60 * 60;
    static constexpr const int secondsPerMinute = 60;

    QString durationText;
    QLocale currentLocale;

    if (secondsPerHour <= m_scanDuration) {
        int seconds = m_scanDuration;
        int hours = static_cast<int>(floor(static_cast<double>(seconds) / secondsPerHour));
        seconds %= secondsPerHour;
        int minutes = static_cast<int>(floor(static_cast<double>(seconds) / secondsPerMinute));
        seconds %= secondsPerMinute;

        if (1 < hours || 10 < minutes) {
            durationText = QStringLiteral("%1h %2m").arg(currentLocale.toString(hours)).arg(minutes);
        } else {
            durationText = QStringLiteral("%1h %2m %3s").arg(currentLocale.toString(hours)).arg(minutes).arg(seconds);
        }
    } else if (secondsPerMinute <= m_scanDuration) {
        int seconds = m_scanDuration;
        int minutes = static_cast<int>(floor(static_cast<double>(seconds) / secondsPerMinute));
        seconds %= secondsPerMinute;
        durationText = QStringLiteral("%1m %2s").arg(minutes).arg(seconds);
    } else {
        durationText = QStringLiteral("%1s").arg(m_scanDuration);
    }
    return durationText;
}

ScanWidget::~ScanWidget() = default;
