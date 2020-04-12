#include "mainwindow.h"
#include "ui/ui_mainwindow.h"

#include <QtGlobal>

#include <QtWidgets/QMessageBox>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QToolButton>
#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtCore/QDir>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QStackedWidget>
#include <QtCore/QMimeData>
#include "application.h"
#include "scanwidget.h"
#include "scanprofilechooser.h"
#include "settingsdialogue.h"
#include "scanprofile.h"


using namespace Qlam;

MainWindow::MainWindow(QWidget * parent)
:   QMainWindow(parent),
    m_ui(std::make_unique<Ui::MainWindow>())
{
	m_ui->setupUi(this);
    setAcceptDrops(true);
    readWindowSettings();

    for(auto * p : Application::instance()->scanProfiles()) {
        m_ui->scanProfileChooser->addProfile(*p);
    }

    m_ui->iconBar->setCurrentRow(0);
    m_ui->settings->setSettings(qlamApp->settings());

	connect(m_ui->iconBar, &QListWidget::currentRowChanged, m_ui->stackWidget, &QStackedWidget::setCurrentIndex);
    connect(m_ui->scanProfileChooser, &ScanProfileChooser::profileChosen, this, &MainWindow::slotScanProfileChosen);
    connect(m_ui->scanBack, &QToolButton::clicked, this, &MainWindow::slotScanBackButtonClicked);
    connect(m_ui->scanStack, &QStackedWidget::currentChanged, this, &MainWindow::syncScanBackButtonWithStack);
	connect(m_ui->scanWidget, &ScanWidget::saveProfileButtonClicked, this, &MainWindow::slotSaveProfileButtonClicked);
	connect(m_ui->scanWidget, &ScanWidget::scanPathsChanged, this, &MainWindow::slotScanPathsChanged);
	connect(m_ui->scanWidget, &ScanWidget::scanStarted, this, &MainWindow::slotDisableBackButton);
	connect(m_ui->scanWidget, &ScanWidget::scanFinished, this, &MainWindow::slotEnableBackButton);
	connect(Application::instance(), qOverload<int>(&Application::scanProfileAdded), this,  &MainWindow::slotScanProfileAdded);
}

MainWindow::~MainWindow() = default;

void MainWindow::readWindowSettings() {
	QSettings settings;

	settings.beginGroup("MainWindow");
	resize(settings.value("size", size()).toSize());
	move(settings.value("pos", pos()).toPoint());
	settings.endGroup();
}

void MainWindow::writeWindowSettings() const {
	QSettings settings;

	settings.beginGroup("MainWindow");
	settings.setValue("size", size());
	settings.setValue("pos", pos());
	settings.endGroup();
}

bool MainWindow::startScanByProfileName(const QString & profileName) {
	for(auto * profile : Application::instance()->scanProfiles()) {
		if(profile->name() == profileName) {
			m_ui->scanWidget->setScanProfile(*profile);
			m_ui->scanStack->setCurrentWidget(m_ui->scanWidget);
			m_ui->scanWidget->doScan();
			return true;
		}
	}

	return false;
}

bool MainWindow::startCustomScan(const QStringList & paths) {
	m_ui->scanWidget->setScanProfile(Application::instance()->scanProfile(0));

	for(const auto & path : paths) {
		m_ui->scanWidget->addScanPath(path);
	}

	m_ui->scanStack->setCurrentWidget(m_ui->scanWidget);
	m_ui->scanWidget->doScan();
	return true;
}

void MainWindow::closeEvent(QCloseEvent * event) {
	if(qlamApp->settings()->areModified()) {
		switch(QMessageBox::question(this, tr("Quit"), tr("The settings have been modified since you last saved them.\n\nWould you like to save them before you exit?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel)) {
			case QMessageBox::Yes:
				qlamApp->settings()->save();
				break;

			case QMessageBox::No:
				break;

			default:
				event->ignore();
				return;
		}
	}

	/* save the GUI settings */
    writeWindowSettings();
	event->accept();
}

void MainWindow::slotScanProfileAdded(int idx) {
	m_ui->scanProfileChooser->addProfile(Application::instance()->scanProfile(idx));
}

void MainWindow::slotScanProfileChosen(int idx) {
	m_ui->scanWidget->setScanProfile(Application::instance()->scanProfile(idx));
	m_ui->scanStack->setCurrentWidget(m_ui->scanWidget);
}

void MainWindow::slotSaveProfileButtonClicked() {
	int idx = m_ui->scanProfileChooser->currentProfileIndex();

	if(0 == idx) {
		bool ok;
		QString name = QInputDialog::getText(this, tr("Save scan profile"), tr("Enter a name for your scan profile"), QLineEdit::Normal, QString(), &ok);

		if(ok && !name.isEmpty()) {
			auto * profile = new ScanProfile(name);
			profile->setPaths(m_ui->scanWidget->scanPaths());
			Application::instance()->addScanProfile(profile);
		}
	}
	else {
		ScanProfile * profile = Application::instance()->scanProfiles().at(idx);

		if(profile) {
			profile->clearPaths();
			profile->setPaths(m_ui->scanWidget->scanPaths());
		}
	}
}

void MainWindow::slotScanPathsChanged() {
	int idx = m_ui->scanProfileChooser->currentProfileIndex();

	// TODO MainWindow should not need to know that index 0 is the bespoke scan profile
	if(0 == idx) {
		ScanProfile * profile = Application::instance()->scanProfiles().at(idx);
		profile->clearPaths();
		profile->setPaths(m_ui->scanWidget->scanPaths());
	}
}

void MainWindow::slotScanBackButtonClicked() {
	int idx = m_ui->scanStack->currentIndex();

	if(0 < idx) {
		--idx;
		m_ui->scanStack->setCurrentIndex(idx);
	}

	syncScanBackButtonWithStack();
}

void MainWindow::syncScanBackButtonWithStack() {
	m_ui->scanBack->setEnabled(0 < m_ui->scanStack->currentIndex());
}

void MainWindow::slotDisableBackButton() {
	m_ui->scanBack->setDisabled(true);
}

void MainWindow::slotEnableBackButton() {
	m_ui->scanBack->setEnabled(true);
}

void MainWindow::dragEnterEvent(QDragEnterEvent * event) {
    if(QList<QUrl> urls = event->mimeData()->urls(); std::any_of(urls.cbegin(), urls.cend(), [](const QUrl & url) {
        return !url.toLocalFile().isEmpty();
    })) {
        event->acceptProposedAction();
        // if we don't find any local paths, the drag'n'drop is rejected - only local paths can be used for scan
    }
}

void MainWindow::dropEvent(QDropEvent * event) {
    QList<QUrl> urls = event->mimeData()->urls();

    urls.erase(std::remove_if(urls.begin(), urls.end(), [](const QUrl & url) {
        return url.toLocalFile().isEmpty();
    }), urls.end());

    if (!urls.empty()) {
        m_ui->scanWidget->setScanProfile((qlamApp->scanProfile(0)));
        m_ui->scanStack->setCurrentWidget(m_ui->scanWidget);

        std::for_each(urls.cbegin(), urls.cend(), [scanWidget = m_ui->scanWidget](const QUrl & url) {
            scanWidget->addScanPath(url.toLocalFile());
        });
    }
}
