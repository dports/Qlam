#include "scanprofilechooser.h"
#include "ui/ui_scanprofilechooser.h"

// TODO make this a subclass of QButtonGroup if we stick with QCommandLinkButton objects

#include <QtGlobal>

#include <QtCore/QDebug>
#include <QtWidgets/QCommandLinkButton>
#include <QtWidgets/QButtonGroup>
#include "application.h"

#define QLAM_SCANPROFILECHOOSER_SCAN_ICON_THEME_NAME QStringLiteral("system-search")

using namespace Qlam;

ScanProfileChooser::ScanProfileChooser(QWidget * parent)
: QWidget(parent),
  m_ui(std::make_unique<Ui::ScanProfileChooser>()),
  m_buttons(nullptr),
  m_buttonLayout(nullptr),
  m_currentProfileIndex(-1) {
	m_ui->setupUi(this);
	m_buttons = new QButtonGroup(this);
	m_buttonLayout = qobject_cast<QVBoxLayout *>(m_ui->scanProfiles->layout());
	connect(m_buttons, qOverload<QAbstractButton *>(&QButtonGroup::buttonClicked), this, &ScanProfileChooser::slotProfileButtonClicked);
}

ScanProfileChooser::~ScanProfileChooser() = default;

bool ScanProfileChooser::addProfile(const ScanProfile & profile) {
	if(!m_buttonLayout) {
		return false;
	}

	QIcon fallbackIcon;
	fallbackIcon.addFile(":iconbar/scan64", QSize(64, 64));
	fallbackIcon.addFile(":iconbar/scan32", QSize(32, 32));
	fallbackIcon.addFile(":iconbar/scan22", QSize(22, 22));

	auto * button = new QCommandLinkButton(profile.name(), this);
	button->setCheckable(true);
	button->setIcon(QIcon::fromTheme(QLAM_SCANPROFILECHOOSER_SCAN_ICON_THEME_NAME, fallbackIcon));
	m_buttons->addButton(button);
	m_buttonLayout->insertWidget(m_buttonLayout->count() - 1, button);
	return true;
}

bool ScanProfileChooser::removeProfile(int idx) {
	if(0 > idx || m_buttons->buttons().count() <= idx) {
		return false;
	}

	QAbstractButton * button = m_buttons->buttons().at(idx);
	button->setParent(nullptr);
	m_buttonLayout->removeWidget(button);
	m_buttons->removeButton(button);
	delete button;
	return true;
}

ScanProfile ScanProfileChooser::currentProfile() {
	int i = currentProfileIndex();
	Q_ASSERT(i >= 0 && i < m_buttons->buttons().count());
	return *(Application::instance()->scanProfiles()).at(i);
}

void ScanProfileChooser::clearProfiles() {
	for(auto * b : m_buttons->buttons()) {
		b->setParent(nullptr);
		m_buttons->removeButton(b);
		m_buttonLayout->removeWidget(b);
		delete b;
	}
}

void ScanProfileChooser::slotProfileButtonClicked(QAbstractButton * button) {
	m_currentProfileIndex = m_buttons->buttons().indexOf(button);

	if(0 <= m_currentProfileIndex) {
		Q_EMIT profileChosen(m_currentProfileIndex);
	}
}
