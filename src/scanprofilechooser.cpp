#include "scanprofilechooser.h"
#include "ui_scanprofilechooser.h"

/**
  * \todo
  *  - make this a subclass of QButtonGroup if we stick with QCommandLinkButton
  *    objects
  */

#include <QtGlobal>

#include <QtCore/QDebug>
#include <QtWidgets/QCommandLinkButton>
#include <QtWidgets/QButtonGroup>
#include "application.h"

#define QLAM_SCANPROFILECHOOSER_SCAN_ICON_THEME_NAME "system-search"

using namespace Qlam;

ScanProfileChooser::ScanProfileChooser(QWidget *parent)
: QWidget(parent),
  ui(new Ui::ScanProfileChooser),
  m_buttons(0),
  m_buttonLayout(0),
  m_currentProfileIndex(-1) {
	ui->setupUi(this);
	m_buttons = new QButtonGroup(this);
	m_buttonLayout = qobject_cast<QVBoxLayout *>(ui->scanProfiles->layout());
	connect(m_buttons, SIGNAL(buttonClicked(QAbstractButton *)), this, SLOT(slotProfileButtonClicked(QAbstractButton *)));
}


ScanProfileChooser::~ScanProfileChooser() {
	delete ui;
}


bool ScanProfileChooser::addProfile( const ScanProfile & profile ) {
	if(!m_buttonLayout) {
		return false;
	}

	QIcon fallbackIcon;
	fallbackIcon.addFile(":iconbar/scan64", QSize(64, 64));
	fallbackIcon.addFile(":iconbar/scan32", QSize(32, 32));
	fallbackIcon.addFile(":iconbar/scan22", QSize(22, 22));

	QCommandLinkButton * button = new QCommandLinkButton(profile.name(), this);
	button->setCheckable(true);
	button->setIcon(QIcon::fromTheme(QLAM_SCANPROFILECHOOSER_SCAN_ICON_THEME_NAME, fallbackIcon));
	m_buttons->addButton(button);
	m_buttonLayout->insertWidget(m_buttonLayout->count() - 1, button);
	return true;
}


bool ScanProfileChooser::removeProfile( int i ) {
	if(0 > i || m_buttons->buttons().count() <= i) {
		return false;
	}

	QAbstractButton * w = m_buttons->buttons().at(i);
	w->setParent(nullptr);
	m_buttonLayout->removeWidget(w);
	m_buttons->removeButton(w);
	delete w;
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


void ScanProfileChooser::slotProfileButtonClicked( QAbstractButton * b ) {
	m_currentProfileIndex = m_buttons->buttons().indexOf(b);

	if(0 <= m_currentProfileIndex) {
		Q_EMIT(profileChosen(m_currentProfileIndex));
	}
}
