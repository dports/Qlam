#include "settingsdialogue.h"
#include "ui_settingsdialogue.h"

#include <QtGlobal>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFileDialog>
#include "settings.h"
#include "settingswidget.h"

using namespace Qlam;

SettingsDialogue::SettingsDialogue( Settings * settings, QWidget * parent )
: QDialog(parent),
  ui(new Ui::SettingsDialogue) {
	ui->setupUi(this);
	ui->settingsWidget->setSettings(settings);

	QIcon fallbackIcon;
	fallbackIcon.addFile(":/settingsdialogue/icons/save22", QSize(22, 22));
	fallbackIcon.addFile(":/settingsdialogue/icons/save32", QSize(32, 32));
//	fallbackIcon.addFile(":/settingsdialogue/icons/save22", QSize(22, 22));
	m_saveButton = new QPushButton(QIcon::fromTheme("document-save", fallbackIcon), tr("Save"), this);
	ui->buttonBox->addButton(m_saveButton, QDialogButtonBox::ApplyRole);
	connect(m_saveButton, SIGNAL(clicked()), this, SLOT(saveSettings()));
}


SettingsDialogue::SettingsDialogue( QWidget * parent )
: QDialog(parent),
  ui(new Ui::SettingsDialogue) {
	ui->setupUi(this);
}


SettingsDialogue::~SettingsDialogue( void ) {
	delete ui;
}


void SettingsDialogue::setCloseButtonVisible( bool v ) {
	if(v) {
		ui->buttonBox->setStandardButtons(ui->buttonBox->standardButtons() | QDialogButtonBox::Close);
	}
	else {
		ui->buttonBox->setStandardButtons(ui->buttonBox->standardButtons() & (~QDialogButtonBox::Close));
	}
}


void SettingsDialogue::saveSettings( void ) const {
	if(!!ui->settingsWidget->settings()) {
		ui->settingsWidget->settings()->save();
	}
}
