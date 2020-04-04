#include "mainwindow.h"
#include "ui/ui_mainwindow.h"

#include <QtGlobal>

#include <QtWidgets/QMessageBox>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QToolButton>
#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtCore/QDir>
#include <QtGui/QIcon>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QStackedWidget>
#include "application.h"
#include "scanwidget.h"
#include "scanprofilechooser.h"
#include "updatewidget.h"
#include "settingsdialogue.h"
#include "scanner.h"
#include "scanprofile.h"


using namespace Qlam;

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        m_ui(std::make_unique<Ui::MainWindow>()),
        m_scanStack(nullptr),
        m_scanBackButton(nullptr),
        m_scanWidget(nullptr),
        m_scanProfileChooser(nullptr),
        m_updateWidget(nullptr),
        m_reportsWidget(nullptr),
        m_settingsDialogue(nullptr) {
	m_ui->setupUi(this);
	addBuiltInWidgets();
	m_ui->iconBar->setCurrentRow(0);
//	m_ui->iconBar->setIconMargin(10);

	readSettings();

	connect(m_ui->iconBar, &QListWidget::currentRowChanged, m_ui->stackWidget, &QStackedWidget::setCurrentIndex);
	connect(m_scanWidget, SIGNAL(saveProfileButtonClicked()), this, SLOT(slotSaveProfileButtonClicked()));
	connect(m_scanWidget, SIGNAL(scanPathsChanged()), this, SLOT(slotScanPathsChanged()));
	connect(m_scanWidget, SIGNAL(scanStarted()), this, SLOT(slotDisableBackButton()));
	connect(m_scanWidget, SIGNAL(scanFinished()), this, SLOT(slotEnableBackButton()));
	connect(Application::instance(), SIGNAL(scanProfileAdded(int)), this, SLOT(slotScanProfileAdded(int)));
}


void MainWindow::addBuiltInWidgets() {
	Q_ASSERT(m_ui->iconBar);

	{
		QIcon fallbackIcon;
		fallbackIcon.addFile(":iconbar/scan64", QSize(64, 64));
		fallbackIcon.addFile(":iconbar/scan32", QSize(32, 32));
		fallbackIcon.addFile(":iconbar/scan22", QSize(22, 22));
		m_ui->iconBar->addItem(new QListWidgetItem(QIcon::fromTheme("system-search", fallbackIcon), tr("Scan")));
	}

	{
		QIcon fallbackIcon;
		fallbackIcon.addFile(":iconbar/update64", QSize(64, 64));
		fallbackIcon.addFile(":iconbar/update32", QSize(32, 32));
		fallbackIcon.addFile(":iconbar/update22", QSize(22, 22));
        m_ui->iconBar->addItem(new QListWidgetItem(QIcon::fromTheme("download", fallbackIcon), tr("Update")));
	}

	{
		QIcon fallbackIcon;
		fallbackIcon.addFile(":iconbar/reports64", QSize(64, 64));
		fallbackIcon.addFile(":iconbar/reports32", QSize(32, 32));
		fallbackIcon.addFile(":iconbar/reports22", QSize(22, 22));
        m_ui->iconBar->addItem(new QListWidgetItem(QIcon::fromTheme("accessories-text-editor", fallbackIcon), tr("Reports")));
	}

	{
		QIcon fallbackIcon;
		fallbackIcon.addFile(":iconbar/settings64", QSize(64, 64));
		fallbackIcon.addFile(":iconbar/settings32", QSize(32, 32));
		fallbackIcon.addFile(":iconbar/settings22", QSize(22, 22));
        m_ui->iconBar->addItem(new QListWidgetItem(QIcon::fromTheme("preferences-system", fallbackIcon), tr("Settings")));
	}

	m_scanStack = new QStackedWidget;
	m_scanProfileChooser = new ScanProfileChooser;
	m_scanWidget = new ScanWidget;
	m_updateWidget = new UpdateWidget;
	m_reportsWidget = new QWidget;
	m_settingsDialogue = new SettingsDialogue(qlamApp->settings());
	m_settingsDialogue->hideCloseButton();

	for(auto * p : Application::instance()->scanProfiles()) {
		m_scanProfileChooser->addProfile(*p);
	}

	m_scanStack->addWidget(m_scanProfileChooser);
	m_scanStack->addWidget(m_scanWidget);
	m_scanStack->setCurrentIndex(0);

	connect(m_scanProfileChooser, SIGNAL(profileChosen(int)), this, SLOT(slotScanProfileChosen(int)));

	auto * scanLayout = new QVBoxLayout;
	m_scanBackButton = new QToolButton(this);

	QIcon fallbackIcon;
	fallbackIcon.addFile(":/mainwindow/icons/back22", QSize(22, 22));
	fallbackIcon.addFile(":/mainwindow/icons/back32", QSize(32, 32));
	fallbackIcon.addFile(":/mainwindow/icons/back64", QSize(64, 64));
	m_scanBackButton->setIcon(QIcon::fromTheme("go-previous", fallbackIcon));
	m_scanBackButton->setEnabled(0 < m_scanStack->currentIndex());
	auto * backButtonLayout = new QHBoxLayout;
	backButtonLayout->addWidget(m_scanBackButton);
	backButtonLayout->addStretch();

	connect(m_scanBackButton, &QToolButton::clicked, this, &MainWindow::slotScanBackButtonClicked);
	connect(m_scanStack, &QStackedWidget::currentChanged, this, &MainWindow::syncScanBackButtonWithStack);

	scanLayout->addLayout(backButtonLayout);
	scanLayout->addWidget(m_scanStack);

	auto * container = new QWidget(this);
	container->setLayout(scanLayout);
	m_ui->stackWidget->addWidget(container);
	m_ui->stackWidget->addWidget(m_updateWidget);
	m_ui->stackWidget->addWidget(m_reportsWidget);
	m_ui->stackWidget->addWidget(m_settingsDialogue);
}


void MainWindow::readSettings() {
	QSettings s;

	s.beginGroup("MainWindow");
	resize(s.value("size", size()).toSize());
	move(s.value("pos", pos()).toPoint());
	s.endGroup();
}


void MainWindow::writeSettings() const {
	QSettings s;

	s.beginGroup("MainWindow");
	s.setValue("size", size());
	s.setValue("pos", pos());
	s.endGroup();
}


//void MainWindow::slotSyncStackWithList() {
//}


bool MainWindow::startScanByProfileName(const QString & profile) {
	/* TODO refactor - store profiles here not in chooser */
	for(auto * p : Application::instance()->scanProfiles()) {
		if(p->name() == profile) {
			m_scanWidget->setScanProfile(*p);
			m_scanStack->setCurrentWidget(m_scanWidget);
			m_scanWidget->doScan();
			return true;
		}
	}

	return false;
}


bool MainWindow::startCustomScan(const QStringList & paths) {
	m_scanWidget->setScanProfile(Application::instance()->scanProfile(0));

	for(const auto & path : paths) {
		m_scanWidget->addScanPath(path);
	}

	m_scanStack->setCurrentWidget(m_scanWidget);
	m_scanWidget->doScan();
	return true;
}


void MainWindow::closeEvent(QCloseEvent * event) {
	if(qlamApp->settings()->areModified()) {
		switch(QMessageBox::question(this, tr("Quit"), tr("The settings have been modified since you last saved them.\n\nWould you like to save them before you exit?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel)) {
			case QMessageBox::Yes:
				qlamApp->settings()->save();
				break;

			case QMessageBox::No:
#if defined(QT_DEBUG)
				qDebug() << "changes to settings discarded";
#endif
				break;

			default:
				event->ignore();
				return;
		}
	}

	/* save the GUI settings */
	writeSettings();
	event->accept();
}


void MainWindow::slotScanProfileAdded(int idx) {
	m_scanProfileChooser->addProfile(Application::instance()->scanProfile(idx));
}


void MainWindow::slotScanProfileChosen(int idx) {
	/* TODO validate index */
	m_scanWidget->setScanProfile(Application::instance()->scanProfile(idx));
	m_scanStack->setCurrentWidget(m_scanWidget);
}


void MainWindow::slotSaveProfileButtonClicked() {
	int idx = m_scanProfileChooser->currentProfileIndex();

	if(0 == idx) {
		bool ok;
		QString name = QInputDialog::getText(this, tr("Save scan profile"), tr("Enter a name for your scan profile"), QLineEdit::Normal, QString(), &ok);

		if(ok && !name.isEmpty()) {
			auto * profile = new ScanProfile(name);
			profile->setPaths(m_scanWidget->scanPaths());
			Application::instance()->addScanProfile(profile);
		}
	}
	else {
		ScanProfile * profile = Application::instance()->scanProfiles().at(idx);

		if(profile) {
			profile->clearPaths();
			profile->setPaths(m_scanWidget->scanPaths());
		}
	}
}

void MainWindow::slotScanPathsChanged() {
	int idx = m_scanProfileChooser->currentProfileIndex();

	/* TODO MainWindow should not need to know that index 0 is the custom
	 * scan profile */
	if(0 == idx) {
		ScanProfile * profile = Application::instance()->scanProfiles().at(idx);
		profile->clearPaths();
		profile->setPaths(m_scanWidget->scanPaths());
	}
}

void MainWindow::slotScanBackButtonClicked() {
	int idx = m_scanStack->currentIndex();

	if(0 < idx) {
		--idx;
		m_scanStack->setCurrentIndex(idx);
	}

	syncScanBackButtonWithStack();
}


void MainWindow::syncScanBackButtonWithStack() {
	m_scanBackButton->setEnabled(0 < m_scanStack->currentIndex());
}


void MainWindow::slotDisableBackButton() {
	m_scanBackButton->setDisabled(true);
}


void MainWindow::slotEnableBackButton() {
	m_scanBackButton->setEnabled(true);
}

MainWindow::~MainWindow() = default;
