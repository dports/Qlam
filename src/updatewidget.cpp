#include "updatewidget.h"
#include "ui_updatewidget.h"

/* TODO
 * when the database path in the settings is changed this widget needs to
 * update its database information. perhaps a signal from qlamApp?
 */
#include <QtGlobal>

#include "qlam.h"
#include "application.h"
#include "updatewidgetdatabaseinfohelperthread.h"
#include "databaseinfo.h"
#include "updater.h"
#include <QtWidgets/QTreeWidgetItem>
#include <QtWidgets/QMessageBox>

#include <QtCore/QDebug>

using namespace Qlam;

UpdateWidget::UpdateWidget(QWidget *parent)
: QWidget(parent),
  ui(new Ui::UpdateWidget),
	m_updater(0) {
	ui->setupUi(this);
	QFont f(ui->title->font());
	f.setPointSizeF(f.pointSizeF() * QLAM_TITLE_LABEL_FONT_POINT_SIZE_RATIO);
#if QLAM_TITLE_LABEL_FONT_BOLD
	f.setBold(true);
#endif
	ui->title->setFont(f);
	ui->libraryVersion->setText(tr("ClamAV library version: %1").arg(Application::clamAvVersion()));
	ui->databases->setHeaderLabels(QStringList() << tr("Database") << QString());
	ui->updateProgress->setVisible(false);

	listDatabases();

	Settings * s = qlamApp->settings();

	connect(s, SIGNAL(databasePathChanged(QString)), this, SLOT(listDatabases()));
}


UpdateWidget::~UpdateWidget( void ) {
	delete ui;
}

void UpdateWidget::doUpdate( void ) {
	if(!m_updater) {
		m_updater = new Updater(this);
		connect(m_updater, SIGNAL(checkFailed(QString)), this, SLOT(slotUpdateFailed(QString)));
		connect(m_updater, SIGNAL(updatingMainDatabase(int)), this, SLOT(slotUpdatingMainDatabase(int)));
		connect(m_updater, SIGNAL(updatingDailyDatabase(int)), this, SLOT(slotUpdatingDailyDatabase(int)));
		connect(m_updater, SIGNAL(updatingBytecodeDatabase(int)), this, SLOT(slotUpdatingBytecodeDatabase(int)));
		connect(m_updater, SIGNAL(updateFailed(QString)), this, SLOT(slotUpdateFailed(QString)));
		connect(m_updater, SIGNAL(updateSucceeded()), this, SLOT(slotUpdateSucceeded()));
		connect(m_updater, SIGNAL(finished()), this, SLOT(slotUpdaterFinished()));
		connect(m_updater, SIGNAL(upToDate()), this, SLOT(slotAlreadyUpToDate()));
		connect(m_updater, SIGNAL(updatesFound()), this, SLOT(slotUpdatesFound()));
		connect(m_updater, SIGNAL(updateProgress(int)), ui->updateProgress, SLOT(setValue(int)));
	}

	if(m_updater->isRunning()) {
		qDebug() << "update already in progress";
		QMessageBox::critical(this, tr("Update"), tr("An update is already in progress."));
	}

	ui->updateNowButton->setEnabled(false);
	ui->updateProgress->setValue(0);
	ui->updateProgress->setMinimum(0);
	ui->updateProgress->setMaximum(0);
	ui->updateProgress->setVisible(true);
	ui->statusLabel->setText(tr("Checking for updates..."));
	m_updater->start();
}


void UpdateWidget::addDatabase( const DatabaseInfo & db ) {
	QTreeWidgetItem * dbItem = new QTreeWidgetItem(QStringList() << QFileInfo(db.path()).fileName() << tr("v%1").arg(db.version()));
	dbItem->addChild(new QTreeWidgetItem(QStringList() << tr("Path") << db.path()));
	dbItem->addChild(new QTreeWidgetItem(QStringList() << tr("Build time") << db.buildTime()));
	dbItem->addChild(new QTreeWidgetItem(QStringList() << tr("Version") << db.version()));
	dbItem->addChild(new QTreeWidgetItem(QStringList() << tr("Signatures") << QString::number(db.signatureCount())));
	dbItem->addChild(new QTreeWidgetItem(QStringList() << tr("Built by") << db.builderName()));
	dbItem->addChild(new QTreeWidgetItem(QStringList() << tr("MD5 sum") << db.md5()));
	ui->databases->addTopLevelItem(dbItem);
	dbItem->setExpanded(true);
	ui->databases->resizeColumnToContents(0);
	dbItem->setExpanded(false);
}


void UpdateWidget::slotDatabaseInfoThreadFinished( void ) {
	ui->updateProgress->setVisible(false);
	ui->updateProgress->setMinimum(0);
	ui->updateProgress->setMaximum(100);
	ui->updateProgress->setValue(0);

	setUpdateButtonEnabledState();

	if(0 == ui->databases->topLevelItemCount()) {
		ui->statusLabel->setText(tr("No databases found. Check your database path in the settings."));
	}
	else {
		ui->statusLabel->setText("");
	}
}


void UpdateWidget::slotUpdatesFound( void ) {
	ui->updateProgress->setMinimum(0);
	ui->updateProgress->setMaximum(100);
	ui->updateProgress->setValue(0);
	ui->statusLabel->setText(tr("Found updates."));
}


void UpdateWidget::slotUpdateFailed( const QString & msg ) {
	ui->statusLabel->setText(tr("Update failed: %1").arg(msg));
}


void UpdateWidget::slotUpdatingMainDatabase( int newVersion ) {
	ui->statusLabel->setText(tr("Updating main database to version %1.").arg(newVersion));
}


void UpdateWidget::slotUpdatingDailyDatabase( int newVersion ) {
	ui->statusLabel->setText(tr("Updating daily database to version %1.").arg(newVersion));
}


void UpdateWidget::slotUpdatingBytecodeDatabase( int newVersion ) {
	ui->statusLabel->setText(tr("Updating bytecode database to version %1.").arg(newVersion));
}


void UpdateWidget::slotUpdateSucceeded( void ) {
	ui->statusLabel->setText(tr("Update completed successfully."));
	listDatabases();
}


void UpdateWidget::slotUpdaterFinished( void ) {
	ui->updateProgress->setVisible(false);
	ui->updateProgress->setMinimum(0);
	ui->updateProgress->setMaximum(100);
	ui->updateProgress->setValue(0);
	setUpdateButtonEnabledState();
}


void UpdateWidget::slotAlreadyUpToDate( void ) {
	ui->statusLabel->setText(tr("Already up to date."));
}


void UpdateWidget::listDatabases( void ) {
qDebug() << "listing databases";
	ui->databases->clear();
	ui->updateProgress->setVisible(true);
	ui->updateProgress->setMinimum(0);
	ui->updateProgress->setMaximum(0);
	ui->statusLabel->setText(tr("Reading database details..."));
	ui->updateNowButton->setDisabled(true);

	/* do this in a separate thread as it's a relatively expensive operation */
	UpdateWidgetDatabaseInfoHelperThread * thread = new UpdateWidgetDatabaseInfoHelperThread(this);
	connect(thread, SIGNAL(databaseInfoRead(const DatabaseInfo &)), this, SLOT(addDatabase(const DatabaseInfo &)));
	connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
	connect(thread, SIGNAL(finished()), this, SLOT(slotDatabaseInfoThreadFinished()));
	thread->start();
}


void UpdateWidget::setUpdateButtonEnabledState( void ) {
	if(qlamApp->settings()->usingSystemDatabases()) {
		ui->updateNowButton->setDisabled(true);
		ui->statusLabel->setText(tr(""));
	}
	else {
		ui->updateNowButton->setEnabled(true);
	}
}
