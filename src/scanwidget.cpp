#include "scanwidget.h"
#include "ui_scanwidget.h"

/** \file scanwidget.cpp
  *
  * \todo
  * - options:
  *   - close program if scan is clean
  *   - automatically save scan report
  * - file count happens in gui thread which blocks on large scans, make it
  *   happen in a separate thread
  * - allow adding of scan path by text-entry (i.e. don't require paths to be
  *   added by file/dir dialogue)
  */
#include <QtGlobal>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QSettings>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtCore/QMimeData>
#include <QtCore/QUrl>

#include "qlam.h"
#include "scanner.h"
#include "scanprofile.h"
#include "elidinglabel.h"

using namespace Qlam;

const int ScanWidget::IndeterminateProgress = -1;

ScanWidget::ScanWidget(QWidget *parent)
	: QWidget(parent),
	  ui(new Ui::ScanWidget),
	  m_scanner(new Scanner(QString(), this)),
	  m_scanDurationTimer(0) {
	ui->setupUi(this);
	setAcceptDrops(true);
	hideScanOutput();
	slotScanPathsSelectionChanged();

	QFont f(ui->title->font());
	f.setPointSizeF(f.pointSizeF() * QLAM_TITLE_LABEL_FONT_POINT_SIZE_RATIO);
#if QLAM_TITLE_LABEL_FONT_BOLD
	f.setBold(true);
#endif
	ui->title->setFont(f);
	ui->issuesList->setHeaderLabels(QStringList() << tr("File path") << tr("Issue"));

	connect(ui->scanButton, SIGNAL(clicked()), this, SIGNAL(scanButtonClicked()));
	connect(ui->saveScanProfile, SIGNAL(clicked()), this, SIGNAL(saveProfileButtonClicked()));

	/* we use a blocking queued connection because all signals originate in the scanner
	 * thread (all are emitted after Scanner::run() has been called and before it exits)
	 * and we need the slots to be called immediately otherwise they can be called in an
	 * incorrect order (using a non-blocking queued connection). we also must ensure that
	 * the slots are invoked in the receiver's thread (the GUI thread) because they
	 * update GUI components and this is not safe outside the GUI thread.
	 *
	 * see http://qt-project.org/doc/qt-4.8/qt.html#ConnectionType-enum for details of
	 * the different connection types
	 */
	connect(m_scanner, SIGNAL(scanComplete()), this, SLOT(slotScanSucceeded()), Qt::BlockingQueuedConnection);
	connect(m_scanner, SIGNAL(scanFailed()), this, SLOT(slotScanFailed()), Qt::BlockingQueuedConnection);
	connect(m_scanner, SIGNAL(scanAborted()), this, SLOT(slotScanAborted()), Qt::BlockingQueuedConnection);
	connect(m_scanner, SIGNAL(scanFinished()), this, SLOT(slotScanFinished()), Qt::BlockingQueuedConnection);
	connect(m_scanner, SIGNAL(scanFinished()), this, SIGNAL(scanFinished()), Qt::BlockingQueuedConnection);
	connect(m_scanner, SIGNAL(fileScanned(QString)), this, SLOT(setScanStatus(QString)), Qt::BlockingQueuedConnection);
	connect(m_scanner, SIGNAL(fileScanned(QString)), this, SLOT(slotScannerScannedFile()), Qt::BlockingQueuedConnection);
	connect(m_scanner, SIGNAL(fileInfected(QString,QString)), this, SLOT(addInfection(QString,QString)), Qt::BlockingQueuedConnection);
	connect(m_scanner, SIGNAL(fileScanFailed(QString)), this, SLOT(addFailedFileScan(QString)), Qt::BlockingQueuedConnection);
}


ScanWidget::~ScanWidget() {
	delete ui;
}


bool ScanWidget::setScanPath( int i, const QString & path ) {
	if(0 > i || scanPathCount() <= i) {
		return false;
	}

	ui->scanPaths->item(i)->setText(path);
	Q_EMIT(scanPathsChanged());
	return true;
}


QStringList ScanWidget::scanPaths() const {
	QStringList paths;

	for(int i = 0; i < ui->scanPaths->count(); ++i) {
		paths.append(ui->scanPaths->item(i)->text());
	}

	return paths;
}


void ScanWidget::clearScanPaths() {
	ui->scanPaths->clear();
	Q_EMIT(scanPathsChanged());
}


void ScanWidget::addScanPath( const QString & path ) {
	ui->scanPaths->addItem(path);
	Q_EMIT(scanPathsChanged());
}


int ScanWidget::scanPathCount() const {
	return ui->scanPaths->count();
}


void ScanWidget::setScanProfile( const ScanProfile & profile ) {
	bool block = blockSignals(true);
	clearScanOutput();
	hideScanOutput();
	clearScanPaths();

	for(const auto & path : profile.paths()) {
		addScanPath(path);
	}

	ui->title->setText(tr("Scan: %1").arg(profile.name()));
	blockSignals(block);
	Q_EMIT(scanPathsChanged());
}


void ScanWidget::dragEnterEvent( QDragEnterEvent * ev ) {
	if(ev->mimeData()->hasUrls()) {
		QList<QUrl> urls = ev->mimeData()->urls();

		for(const auto & url : urls) {
			QString path = url.toLocalFile();

			if(!path.isEmpty()) {
				ev->acceptProposedAction();
				return;
			}
		}

		/* if we don't find any local paths, the drag'n'drop is rejected - only
		 * local paths can be used for scan */
	}
}

void ScanWidget::dropEvent( QDropEvent * ev ) {
	QList<QUrl> urls = ev->mimeData()->urls();

	for(const auto & url : urls) {
		QString path = url.toLocalFile();
		if(!path.isEmpty()) {
			addScanPath(path);
		}
	}
}

void ScanWidget::timerEvent( QTimerEvent * ev ) {
	if(ev->timerId() == m_scanDurationTimer) {
		++m_scanDuration;
		/* TODO update display widget */
	}
}

void ScanWidget::chooseScanFiles() {
	QString path;

	if(!ui->scanPaths->selectedItems().empty()) {
		path = ui->scanPaths->selectedItems()[0]->text();
	}

	QStringList paths = QFileDialog::getOpenFileNames(this, tr("Scan file"), path);

	if(!paths.isEmpty()) {
		bool block = this->blockSignals(true);

		for(const auto & path : paths) {
			addScanPath(path);
		}

		this->blockSignals(block);

		if(!block) {
			Q_EMIT(scanPathsChanged());
		}
	}
}

void ScanWidget::chooseScanDirectory() {
	QString path;

	if(!ui->scanPaths->selectedItems().empty()) {
		path = ui->scanPaths->selectedItems()[0]->text();
	}

	path = QFileDialog::getExistingDirectory(this, tr("Scan directory"), path);

	if(!path.isEmpty()) {
		addScanPath(path);
	}
}

void ScanWidget::removeSelectedScanPaths() {
	QList<QListWidgetItem *> selectedItems = ui->scanPaths->selectedItems();

	if(0 < selectedItems.count()) {
		bool block = ui->scanPaths->blockSignals(true);

		for(auto * it : selectedItems) {
			if(it) {
				delete it;
			}
		}

		ui->scanPaths->blockSignals(block);

		if(!block) {
			slotScanPathsSelectionChanged();
		}

		Q_EMIT(scanPathsChanged());
	}
}


void ScanWidget::doScan() {
	m_scanner->setScanPaths(scanPaths());

	if(!m_scanner->isValid()) {
		QMessageBox::critical(this, tr("Error"), tr("The scan cannot proceed. Please ensure that libclamav is installed and that all the scan paths are valid."), tr("OK"));
		return;
	}

	clearScanOutput();
	showScanOutput();
	setScanProgress(ScanWidget::IndeterminateProgress);
	setScanStatus(tr("Initialising scan"));
	m_scanDuration = 0;
	m_scanDurationTimer = startTimer(1000);

	if(m_scanner->startScan()) {
		ui->scanButton->setEnabled(false);
		ui->abortButton->setEnabled(true);
		Q_EMIT(scanStarted());
	}
	else {
		qCritical() << "scanner failed to start";
		QMessageBox::critical(this, tr("Error"), tr("The scan could not be started."), tr("OK"));
		return;
	}
}


void ScanWidget::abortScan() {
	if(!m_scanner->isRunning()) {
		qDebug() << "scanner is not running";
		return;
	}

	m_scanner->abort();
}


void ScanWidget::setScanOutputVisible( bool vis ) {
	ui->scanProgress->setEnabled(vis);
	ui->abortButton->setEnabled(vis);
	ui->scanStatus->setEnabled(vis);
	ui->issuesListLabel->setEnabled(vis);
	ui->issuesList->setEnabled(vis);
}


void ScanWidget::setScanStatus( const QString & text ) {
	ui->scanStatus->setText(text);
}


void ScanWidget::clearScanOutput() {
	ui->scanProgress->setValue(0);
	ui->scanStatus->clear();
	ui->issuesList->clear();
	ui->issuesList->setHeaderHidden(true);
}


void ScanWidget::setScanProgress( int pc ) {
	if(IndeterminateProgress == pc) {
		ui->scanProgress->setMaximum(0);
	}
	else {
		ui->scanProgress->setMaximum(100);
		ui->scanProgress->setValue((0 > pc ? 0 : (100 < pc ? 100 : pc)));
	}
}


void ScanWidget::addInfection(const QString & path, const QString & virus ) {
	ui->issuesList->setHeaderHidden(false);
	ui->issuesList->addTopLevelItem(new QTreeWidgetItem(QStringList() << path << tr("Infection: %1").arg(virus)));
	ui->issuesList->resizeColumnToContents(0);
	ui->issuesList->resizeColumnToContents(1);
}

void ScanWidget::addFailedFileScan( const QString & path ) {
	ui->issuesList->setHeaderHidden(false);
	ui->issuesList->addTopLevelItem(new QTreeWidgetItem(QStringList() << path << tr("Unable to scan")));
	ui->issuesList->resizeColumnToContents(0);
	ui->issuesList->resizeColumnToContents(1);
}


void ScanWidget::slotScannerScannedFile() {
	Q_ASSERT(!!m_scanner);
	int pc = int(100 * (double(m_scanner->scannedFileCount()) / double(m_scanner->fileCount())));
	setScanProgress(pc);
}


void ScanWidget::slotScanSucceeded() {
	Q_ASSERT_X(m_scanner, "ScanWidget::slotScanSucceeded()", "method called not in response to signal from ClamScan object");
	unsigned long long kb = m_scanner->dataScanned();
	QString sizeDisplay;

	if(kb < 5120) {
		/* less than 5MB, express in KB */
		sizeDisplay = tr("%1 Kb").arg(kb);
	}
	else if(kb < 1048576 * 5) {
		/* less than 5GB, express in MB */
		sizeDisplay = tr("%1 Mb").arg(double(kb) / 1024, 0, 'f', 2);
	}
	else {
		/* express in GB */
		sizeDisplay = tr("%1 Gb").arg(double(kb) / 1048576, 0, 'f', 2);
	}

	QString timeDisplay;

	if(m_scanDuration < 60) {
		timeDisplay = tr("%1s").arg(m_scanDuration);
	}
	else if(m_scanDuration < 3600) {
		timeDisplay = tr("%1m %2s").arg(m_scanDuration / 60).arg(m_scanDuration % 60);
	}
	else {
		timeDisplay = tr("%1h %2m %3s").arg(m_scanDuration / 3600).arg(m_scanDuration / 60).arg(m_scanDuration % 60);
	}

	setScanStatus(tr("Scan finished in %4 (%1 infections found in %2 of data in %3 files)").arg(m_scanner->infectedFileCount()).arg(sizeDisplay).arg(m_scanner->scannedFileCount()).arg(timeDisplay));
	setScanProgress(100);
}


void ScanWidget::slotScanFailed() {
	setScanStatus(tr("Scan failed"));
	/* TODO add "failed" message to issues ? */
	setScanProgress(0);
}


void ScanWidget::slotScanAborted() {
	setScanStatus(tr("Scan aborted"));
	setScanProgress(0);
}


void ScanWidget::slotScanFinished() {
	ui->scanButton->setEnabled(true);
	ui->abortButton->setEnabled(false);
	killTimer(m_scanDurationTimer);
	m_scanDurationTimer = 0;
}


void ScanWidget::slotScanPathsSelectionChanged() {
	ui->removeScanPath->setEnabled(0 < ui->scanPaths->selectedItems().count());
}
