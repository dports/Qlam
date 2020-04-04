#include <QtGlobal>
#include <QtWidgets/QTreeWidgetItem>
#include <QtWidgets/QMessageBox>
#include <QtCore/QDebug>
#include "updatewidget.h"
#include "ui/ui_updatewidget.h"
#include "qlam.h"
#include "application.h"
#include "updatewidgetdatabaseinfohelperthread.h"
#include "databaseinfo.h"
#include "updater.h"

/* TODO
 * when the database path in the settings is changed this widget needs to
 * update its database information. perhaps a signal from qlamApp?
 */

using namespace Qlam;

UpdateWidget::UpdateWidget(QWidget *parent)
: QWidget(parent),
  m_ui(std::make_unique<Ui::UpdateWidget>()),
	m_updater(nullptr) {
	m_ui->setupUi(this);
	QFont f(m_ui->title->font());
	f.setPointSizeF(f.pointSizeF() * QLAM_TITLE_LABEL_FONT_POINT_SIZE_RATIO);
#if QLAM_TITLE_LABEL_FONT_BOLD
	f.setBold(true);
#endif
	m_ui->title->setFont(f);
	m_ui->libraryVersion->setText(tr("ClamAV library version: %1").arg(Application::clamAvVersion()));
	m_ui->databases->setHeaderLabels(QStringList() << tr("Database") << QString());
	m_ui->updateProgress->setVisible(false);

	listDatabases();

	connect(qlamApp->settings(), &Settings::databasePathChanged, this, &UpdateWidget::listDatabases);
}

UpdateWidget::~UpdateWidget() = default;

void UpdateWidget::doUpdate() {
	if(!m_updater) {
		m_updater = new Updater(this);
		connect(m_updater, &Updater::checkFailed, this, &UpdateWidget::slotUpdateFailed);
		connect(m_updater, &Updater::updatingMainDatabase, this, &UpdateWidget::slotUpdatingMainDatabase);
		connect(m_updater, &Updater::updatingDailyDatabase, this, &UpdateWidget::slotUpdatingDailyDatabase);
		connect(m_updater, &Updater::updatingBytecodeDatabase, this, &UpdateWidget::slotUpdatingBytecodeDatabase);
		connect(m_updater, &Updater::updateFailed, this, &UpdateWidget::slotUpdateFailed);
		connect(m_updater, &Updater::updateSucceeded, this, &UpdateWidget::slotUpdateSucceeded);
		connect(m_updater, &Updater::finished, this, &UpdateWidget::slotUpdaterFinished);
		connect(m_updater, &Updater::upToDate, this, &UpdateWidget::slotAlreadyUpToDate);
		connect(m_updater, &Updater::updatesFound, this, &UpdateWidget::slotUpdatesFound);
		connect(m_updater, &Updater::updateProgress, m_ui->updateProgress, &QProgressBar::setValue);
	}

	if(m_updater->isRunning()) {
		qDebug() << "update already in progress";
		QMessageBox::critical(this, tr("Update"), tr("An update is already in progress."));
	}

	m_ui->updateNowButton->setEnabled(false);
	m_ui->updateProgress->setValue(0);
	m_ui->updateProgress->setMinimum(0);
	m_ui->updateProgress->setMaximum(0);
	m_ui->updateProgress->setVisible(true);
	m_ui->statusLabel->setText(tr("Checking for updates..."));
	m_updater->start();
}

void UpdateWidget::addDatabase( const DatabaseInfo & db ) {
	auto * dbItem = new QTreeWidgetItem(QStringList() << QFileInfo(db.path()).fileName() << tr("v%1").arg(db.version()));
	dbItem->addChild(new QTreeWidgetItem(QStringList() << tr("Path") << db.path()));
	dbItem->addChild(new QTreeWidgetItem(QStringList() << tr("Build time") << db.buildTime()));
	dbItem->addChild(new QTreeWidgetItem(QStringList() << tr("Version") << db.version()));
	dbItem->addChild(new QTreeWidgetItem(QStringList() << tr("Signatures") << QString::number(db.signatureCount())));
	dbItem->addChild(new QTreeWidgetItem(QStringList() << tr("Built by") << db.builderName()));
	dbItem->addChild(new QTreeWidgetItem(QStringList() << tr("MD5 sum") << db.md5()));
	m_ui->databases->addTopLevelItem(dbItem);
	dbItem->setExpanded(true);
	m_ui->databases->resizeColumnToContents(0);
	dbItem->setExpanded(false);
}

void UpdateWidget::slotDatabaseInfoThreadFinished() {
	m_ui->updateProgress->setVisible(false);
	m_ui->updateProgress->setMinimum(0);
	m_ui->updateProgress->setMaximum(100);
	m_ui->updateProgress->setValue(0);

	setUpdateButtonEnabledState();

	if(0 == m_ui->databases->topLevelItemCount()) {
		m_ui->statusLabel->setText(tr("No databases found. Check your database path in the settings."));
	}
	else {
		m_ui->statusLabel->setText("");
	}
}

void UpdateWidget::slotUpdatesFound() {
	m_ui->updateProgress->setMinimum(0);
	m_ui->updateProgress->setMaximum(100);
	m_ui->updateProgress->setValue(0);
	m_ui->statusLabel->setText(tr("Found updates."));
}

void UpdateWidget::slotUpdateFailed( const QString & msg ) {
	m_ui->statusLabel->setText(tr("Update failed: %1").arg(msg));
}

void UpdateWidget::slotUpdatingMainDatabase( int newVersion ) {
	m_ui->statusLabel->setText(tr("Updating main database to version %1.").arg(newVersion));
}

void UpdateWidget::slotUpdatingDailyDatabase( int newVersion ) {
	m_ui->statusLabel->setText(tr("Updating daily database to version %1.").arg(newVersion));
}

void UpdateWidget::slotUpdatingBytecodeDatabase( int newVersion ) {
	m_ui->statusLabel->setText(tr("Updating bytecode database to version %1.").arg(newVersion));
}

void UpdateWidget::slotUpdateSucceeded() {
	m_ui->statusLabel->setText(tr("Update completed successfully."));
	listDatabases();
}

void UpdateWidget::slotUpdaterFinished() {
	m_ui->updateProgress->setVisible(false);
	m_ui->updateProgress->setMinimum(0);
	m_ui->updateProgress->setMaximum(100);
	m_ui->updateProgress->setValue(0);
	setUpdateButtonEnabledState();
}

void UpdateWidget::slotAlreadyUpToDate() {
	m_ui->statusLabel->setText(tr("Already up to date."));
}

void UpdateWidget::listDatabases() {
qDebug() << "listing databases";
	m_ui->databases->clear();
	m_ui->updateProgress->setVisible(true);
	m_ui->updateProgress->setMinimum(0);
	m_ui->updateProgress->setMaximum(0);
	m_ui->statusLabel->setText(tr("Reading database details..."));
	m_ui->updateNowButton->setDisabled(true);

	/* do this in a separate thread as it's a relatively expensive operation */
	auto * thread = new UpdateWidgetDatabaseInfoHelperThread(this);
	connect(thread, &UpdateWidgetDatabaseInfoHelperThread::databaseInfoRead, this, &UpdateWidget::addDatabase);
	connect(thread, &UpdateWidgetDatabaseInfoHelperThread::finished, thread, &UpdateWidgetDatabaseInfoHelperThread::deleteLater);
	connect(thread, &UpdateWidgetDatabaseInfoHelperThread::finished, this, &UpdateWidget::slotDatabaseInfoThreadFinished);
	thread->start();
}

void UpdateWidget::setUpdateButtonEnabledState() {
	if(qlamApp->settings()->usingSystemDatabases()) {
		m_ui->updateNowButton->setDisabled(true);
		m_ui->statusLabel->setText(tr(""));
	}
	else {
		m_ui->updateNowButton->setEnabled(true);
	}
}

QString UpdateWidget::statusText() const {
    return m_ui->statusLabel->text();
}

void UpdateWidget::setStatusText(const QString &text) {
    m_ui->statusLabel->setText(text);
}
