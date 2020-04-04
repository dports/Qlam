#include "settingsdialogue.h"
#include "ui/ui_settingsdialogue.h"

#include <QtGlobal>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFileDialog>
#include "settings.h"

using namespace Qlam;

SettingsDialogue::SettingsDialogue(Settings * settings, QWidget * parent)
: SettingsDialogue(parent) {
	m_ui->settingsWidget->setSettings(settings);

	QIcon fallbackIcon;
	fallbackIcon.addFile(":/settingsdialogue/icons/save22", QSize(22, 22));
	fallbackIcon.addFile(":/settingsdialogue/icons/save32", QSize(32, 32));
	m_saveButton = new QPushButton(QIcon::fromTheme("document-save", fallbackIcon), tr("Save"), this);
	m_ui->buttonBox->addButton(m_saveButton, QDialogButtonBox::ApplyRole);
	connect(m_saveButton, &QPushButton::clicked, this, &SettingsDialogue::saveSettings);
}

SettingsDialogue::SettingsDialogue( QWidget * parent )
: QDialog(parent),
  m_saveButton(nullptr),
  m_ui(std::make_unique<Ui::SettingsDialogue>()) {
	m_ui->setupUi(this);
}

SettingsDialogue::~SettingsDialogue() = default;

void SettingsDialogue::setCloseButtonVisible(bool visible) {
	if(visible) {
		m_ui->buttonBox->setStandardButtons(m_ui->buttonBox->standardButtons() | QDialogButtonBox::Close);
	}
	else {
		m_ui->buttonBox->setStandardButtons(m_ui->buttonBox->standardButtons() & (~QDialogButtonBox::Close));
	}
}

void SettingsDialogue::saveSettings() const {
	if(m_ui->settingsWidget->settings()) {
		m_ui->settingsWidget->settings()->save();
	}
}
