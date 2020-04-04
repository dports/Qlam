#include "settingswidget.h"
#include "ui/ui_settingswidget.h"

#include <QtGlobal>

#include <QtCore/QDir>
#include <QtCore/QStringList>
#include <QtWidgets/QFileDialog>
#include "application.h"
#include "settings.h"

using namespace Qlam;

SettingsWidget::SettingsWidget( Settings * settings, QWidget * parent )
: SettingsWidget(parent) {
    m_settings = settings;
	connectSettings();
	syncWithSettings();
}

SettingsWidget::SettingsWidget( QWidget * parent )
: QWidget(parent),
  m_ui(std::make_unique<Ui::SettingsWidget>()),
  m_settings(nullptr) {
	m_ui->setupUi(this);
	setupMirrors();
}

SettingsWidget::~SettingsWidget() = default;

void SettingsWidget::setSettings( Settings * settings ) {
	disconnectSettings();
	m_settings = settings;
	connectSettings();
	syncWithSettings();
}

void SettingsWidget::slotDatabasePathChanged() {
	listDatabases();

	if(m_settings) {
		disconnectSettings();
		m_settings->setDatabasePath(m_ui->databasePath->text());
		connectSettings();
	}
}

void SettingsWidget::slotServerTypeChanged() {
	if(!m_settings) {
		return;
	}

	if(m_ui->updateServerType->checkedButton() == m_ui->officialMirrorButton) {
		disconnectSettings();
		m_settings->setUpdateServerType(Settings::OfficialMirror);
		connectSettings();
	}
	else if(m_ui->updateServerType->checkedButton() == m_ui->customServerButton) {
		disconnectSettings();
		m_settings->setUpdateServerType(Settings::Custom);
		connectSettings();
	}
}

void SettingsWidget::slotMirrorChanged() {
	if(!m_settings) {
		return;
	}

	disconnectSettings();
	m_settings->setUpdateMirror(m_ui->mirrorCombo->currentText());
	connectSettings();
}

void SettingsWidget::slotCustomServerChanged() {
	if(!m_settings) {
		return;
	}

	disconnectSettings();
	m_settings->setCustomUpdateServer(m_ui->customServer->text());
	connectSettings();
}

void SettingsWidget::connectSettings() {
	if(m_settings) {
		connect(m_settings, &Settings::changed, this, &SettingsWidget::syncWithSettings);
	}
}

void SettingsWidget::disconnectSettings() {
	if(m_settings) {
		m_settings->disconnect(this);
	}
}

void SettingsWidget::syncWithSettings() {
	disconnectSettings();

	if(!m_settings) {
		m_ui->databasePath->clear();
	}
	else {
		m_ui->databasePath->setText(m_settings->databasePath());
		m_ui->officialMirrorButton->setChecked(false);
		m_ui->customServerButton->setChecked(false);

		switch(m_settings->updateServerType()) {
			case Settings::OfficialMirror:
				m_ui->officialMirrorButton->setChecked(true);
				break;

			case Settings::Custom:
				m_ui->customServerButton->setChecked(true);
				break;
		}

		bool block = m_ui->mirrorCombo->blockSignals(true);
		m_ui->mirrorCombo->setCurrentIndex(m_ui->mirrorCombo->findText(m_settings->updateMirror()));
		m_ui->mirrorCombo->blockSignals(block);
		m_ui->customServer->setText(m_settings->customUpdateServer().toString());
		listDatabases();

		if(m_settings->areModified()) {
			setWindowTitle(tr("Settings *"));
		}
		else {
			setWindowTitle(tr("Settings"));
		}
	}

	connectSettings();
}

void SettingsWidget::listDatabases() {
	m_ui->databases->clear();
	QString path(m_ui->databasePath->text());

	if(path.isEmpty()) {
		path = qlamApp->systemDatabasePath();
	}

	QDir d(path);

	for(const auto & file : d.entryList(QStringList() << "*.cvd" << "*.cld", QDir::Files | QDir::NoDotAndDotDot)) {
		m_ui->databases->addItem(file);
	}
}

void SettingsWidget::setupMirrors() {
    static constexpr std::array<const char *, 255> mirrors({
        "ac", "ad", "ae", "af", "ag", "ai", "al", "am", "an", "ao",
        "aq", "ar", "as", "at", "au", "aw", "ax", "az", "ba", "bb",
        "bd", "be", "bf", "bg", "bh", "bi", "bj", "bl", "bm", "bn",
        "bo", "bq", "br", "bs", "bt", "bv", "bw", "by", "bz", "ca",
        "cc", "cd", "cf", "cg", "ch", "ci", "ck", "cl", "cm", "cn",
        "co", "cr", "cu", "cv", "cw", "cx", "cy", "cz", "de", "dj",
        "dk", "dm", "do", "dz", "ec", "ee", "eg", "eh", "er", "es",
        "et", "eu", "fi", "fj", "fk", "fm", "fo", "fr", "ga", "gb",
        "gd", "ge", "gf", "gg", "gh", "gi", "gl", "gm", "gn", "gp",
        "gq", "gr", "gs", "gt", "gu", "gw", "gy", "hk", "hm", "hn",
        // 100
        "hr", "ht", "hu", "id", "ie", "il", "im", "in", "io", "iq",
        "ir", "is", "it", "je", "jm", "jo", "jp", "ke", "kg", "kh",
        "ki", "km", "kn", "kp", "kr", "kw", "ky", "kz", "la", "lb",
        "lc", "li", "lk", "lr", "ls", "lt", "lu", "lv", "ly", "ma",
        "mc", "md", "me", "mf", "mg", "mh", "mk", "ml", "mm", "mn",
        "mo", "mp", "mq", "mr", "ms", "mt", "mu", "mv", "mw", "mx",
        "my", "mz", "na", "nc", "ne", "nf", "ng", "ni", "nl", "no",
        "np", "nr", "nu", "nz", "om", "pa", "pe", "pf", "pg", "ph",
        "pk", "pl", "pm", "pn", "pr", "ps", "pt", "pw", "py", "qa",
        "re", "ro", "rs", "ru", "rw", "sa", "sb", "sc", "sd", "se",
        // 200
        "sg", "sh", "si", "sj", "sk", "sl", "sm", "sn", "so", "sr",
        "ss", "st", "su", "sv", "sx", "sy", "sz", "tc", "td", "tf",
        "tg", "th", "tj", "tk", "tl", "tm", "tn", "to", "tp", "tr",
        "tt", "tv", "tw", "tz", "ua", "ug", "uk", "um", "us", "uy",
        "uz", "va", "vc", "ve", "vg", "vi", "vn", "vu", "wf", "ws",
        "ye", "yt", "za", "zm", "zw",
    });

    std::for_each(mirrors.cbegin(), mirrors.cend(), [this] (const QString & mirror) {
        m_ui->mirrorCombo->addItem(mirror);
    });
}

void SettingsWidget::chooseDatabasePath() {
	QString path = QFileDialog::getExistingDirectory(this, tr("Database path"), m_ui->databasePath->text());

	if(!path.isEmpty()) {
		m_ui->databasePath->setText(path);
		slotDatabasePathChanged();
	}
}
