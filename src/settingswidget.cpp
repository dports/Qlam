#include "settingswidget.h"
#include "ui_settingswidget.h"

#include <QtGlobal>

#include <QtCore/QDir>
#include <QtCore/QStringList>
#include <QtWidgets/QFileDialog>
#include "application.h"
#include "settings.h"

using namespace Qlam;

SettingsWidget::SettingsWidget( Settings * settings, QWidget * parent )
: QWidget(parent),
  ui(new Ui::SettingsWidget),
  m_settings(settings) {
	ui->setupUi(this);
	setupMirrors();
	connectSettings();
	syncWithSettings();
}


SettingsWidget::SettingsWidget( QWidget * parent )
: QWidget(parent),
  ui(new Ui::SettingsWidget),
  m_settings(0) {
	ui->setupUi(this);
	setupMirrors();
}


SettingsWidget::~SettingsWidget( void ) {
	delete ui;
}


void SettingsWidget::setSettings( Settings * settings ) {
	disconnectSettings();
	m_settings = settings;
	connectSettings();
	syncWithSettings();
}


void SettingsWidget::slotDatabasePathChanged( void ) {
	listDatabases();

	if(m_settings) {
		disconnectSettings();
		m_settings->setDatabasePath(ui->databasePath->text());
		connectSettings();
	}
}


void SettingsWidget::slotServerTypeChanged( void ) {
	if(!m_settings) {
		return;
	}

	if(ui->updateServerType->checkedButton() == ui->officialMirrorButton) {
		disconnectSettings();
		m_settings->setUpdateServerType(Settings::OfficialMirror);
		connectSettings();
	}
	else if(ui->updateServerType->checkedButton() == ui->customServerButton) {
		disconnectSettings();
		m_settings->setUpdateServerType(Settings::Custom);
		connectSettings();
	}
}


void SettingsWidget::slotMirrorChanged( void ) {
	if(!m_settings) {
		return;
	}

	disconnectSettings();
	m_settings->setUpdateMirror(ui->mirrorCombo->currentText());
	connectSettings();
}


void SettingsWidget::slotCustomServerChanged( void ) {
	if(!m_settings) {
		return;
	}

	disconnectSettings();
	m_settings->setCustomUpdateServer(ui->customServer->text());
	connectSettings();
}


void SettingsWidget::connectSettings( void ) {
	if(m_settings) {
		connect(m_settings, SIGNAL(changed()), this, SLOT(syncWithSettings()));
	}
}


void SettingsWidget::disconnectSettings( void ) {
	if(m_settings) {
		m_settings->disconnect(this);
	}
}


void SettingsWidget::syncWithSettings( void ) {
	disconnectSettings();

	if(!m_settings) {
		ui->databasePath->clear();
	}
	else {
		ui->databasePath->setText(m_settings->databasePath());
		ui->officialMirrorButton->setChecked(false);
		ui->customServerButton->setChecked(false);

		switch(m_settings->updateServerType()) {
			case Settings::OfficialMirror:
				ui->officialMirrorButton->setChecked(true);
				break;

			case Settings::Custom:
				ui->customServerButton->setChecked(true);
				break;
		}

		bool block = ui->mirrorCombo->blockSignals(true);
		ui->mirrorCombo->setCurrentIndex(ui->mirrorCombo->findText(m_settings->updateMirror()));
		ui->mirrorCombo->blockSignals(block);
		ui->customServer->setText(m_settings->customUpdateServer().toString());
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


void SettingsWidget::listDatabases( void ) {
	ui->databases->clear();
	QString path(ui->databasePath->text());

	if(path.isEmpty()) {
		path = qlamApp->systemDatabasePath();
	}

	QDir d(path);

	for(const auto & file : d.entryList(QStringList() << "*.cvd" << "*.cld", QDir::Files | QDir::NoDotAndDotDot)) {
		ui->databases->addItem(file);
	}
}

void SettingsWidget::setupMirrors( void ) {
	ui->mirrorCombo->addItem("ac");
	ui->mirrorCombo->addItem("ad");
	ui->mirrorCombo->addItem("ae");
	ui->mirrorCombo->addItem("af");
	ui->mirrorCombo->addItem("ag");
	ui->mirrorCombo->addItem("ai");
	ui->mirrorCombo->addItem("al");
	ui->mirrorCombo->addItem("am");
	ui->mirrorCombo->addItem("an");
	ui->mirrorCombo->addItem("ao");
	ui->mirrorCombo->addItem("aq");
	ui->mirrorCombo->addItem("ar");
	ui->mirrorCombo->addItem("as");
	ui->mirrorCombo->addItem("at");
	ui->mirrorCombo->addItem("au");
	ui->mirrorCombo->addItem("aw");
	ui->mirrorCombo->addItem("ax");
	ui->mirrorCombo->addItem("az");
	ui->mirrorCombo->addItem("ba");
	ui->mirrorCombo->addItem("bb");
	ui->mirrorCombo->addItem("bd");
	ui->mirrorCombo->addItem("be");
	ui->mirrorCombo->addItem("bf");
	ui->mirrorCombo->addItem("bg");
	ui->mirrorCombo->addItem("bh");
	ui->mirrorCombo->addItem("bi");
	ui->mirrorCombo->addItem("bj");
	ui->mirrorCombo->addItem("bl");
	ui->mirrorCombo->addItem("bm");
	ui->mirrorCombo->addItem("bn");
	ui->mirrorCombo->addItem("bo");
	ui->mirrorCombo->addItem("bq");
	ui->mirrorCombo->addItem("br");
	ui->mirrorCombo->addItem("bs");
	ui->mirrorCombo->addItem("bt");
	ui->mirrorCombo->addItem("bv");
	ui->mirrorCombo->addItem("bw");
	ui->mirrorCombo->addItem("by");
	ui->mirrorCombo->addItem("bz");
	ui->mirrorCombo->addItem("ca");
	ui->mirrorCombo->addItem("cc");
	ui->mirrorCombo->addItem("cd");
	ui->mirrorCombo->addItem("cf");
	ui->mirrorCombo->addItem("cg");
	ui->mirrorCombo->addItem("ch");
	ui->mirrorCombo->addItem("ci");
	ui->mirrorCombo->addItem("ck");
	ui->mirrorCombo->addItem("cl");
	ui->mirrorCombo->addItem("cm");
	ui->mirrorCombo->addItem("cn");
	ui->mirrorCombo->addItem("co");
	ui->mirrorCombo->addItem("cr");
	ui->mirrorCombo->addItem("cu");
	ui->mirrorCombo->addItem("cv");
	ui->mirrorCombo->addItem("cw");
	ui->mirrorCombo->addItem("cx");
	ui->mirrorCombo->addItem("cy");
	ui->mirrorCombo->addItem("cz");
	ui->mirrorCombo->addItem("de");
	ui->mirrorCombo->addItem("dj");
	ui->mirrorCombo->addItem("dk");
	ui->mirrorCombo->addItem("dm");
	ui->mirrorCombo->addItem("do");
	ui->mirrorCombo->addItem("dz");
	ui->mirrorCombo->addItem("ec");
	ui->mirrorCombo->addItem("ee");
	ui->mirrorCombo->addItem("eg");
	ui->mirrorCombo->addItem("eh");
	ui->mirrorCombo->addItem("er");
	ui->mirrorCombo->addItem("es");
	ui->mirrorCombo->addItem("et");
	ui->mirrorCombo->addItem("eu");
	ui->mirrorCombo->addItem("fi");
	ui->mirrorCombo->addItem("fj");
	ui->mirrorCombo->addItem("fk");
	ui->mirrorCombo->addItem("fm");
	ui->mirrorCombo->addItem("fo");
	ui->mirrorCombo->addItem("fr");
	ui->mirrorCombo->addItem("ga");
	ui->mirrorCombo->addItem("gb");
	ui->mirrorCombo->addItem("gd");
	ui->mirrorCombo->addItem("ge");
	ui->mirrorCombo->addItem("gf");
	ui->mirrorCombo->addItem("gg");
	ui->mirrorCombo->addItem("gh");
	ui->mirrorCombo->addItem("gi");
	ui->mirrorCombo->addItem("gl");
	ui->mirrorCombo->addItem("gm");
	ui->mirrorCombo->addItem("gn");
	ui->mirrorCombo->addItem("gp");
	ui->mirrorCombo->addItem("gq");
	ui->mirrorCombo->addItem("gr");
	ui->mirrorCombo->addItem("gs");
	ui->mirrorCombo->addItem("gt");
	ui->mirrorCombo->addItem("gu");
	ui->mirrorCombo->addItem("gw");
	ui->mirrorCombo->addItem("gy");
	ui->mirrorCombo->addItem("hk");
	ui->mirrorCombo->addItem("hm");
	ui->mirrorCombo->addItem("hn");
	ui->mirrorCombo->addItem("hr");
	ui->mirrorCombo->addItem("ht");
	ui->mirrorCombo->addItem("hu");
	ui->mirrorCombo->addItem("id");
	ui->mirrorCombo->addItem("ie");
	ui->mirrorCombo->addItem("il");
	ui->mirrorCombo->addItem("im");
	ui->mirrorCombo->addItem("in");
	ui->mirrorCombo->addItem("io");
	ui->mirrorCombo->addItem("iq");
	ui->mirrorCombo->addItem("ir");
	ui->mirrorCombo->addItem("is");
	ui->mirrorCombo->addItem("it");
	ui->mirrorCombo->addItem("je");
	ui->mirrorCombo->addItem("jm");
	ui->mirrorCombo->addItem("jo");
	ui->mirrorCombo->addItem("jp");
	ui->mirrorCombo->addItem("ke");
	ui->mirrorCombo->addItem("kg");
	ui->mirrorCombo->addItem("kh");
	ui->mirrorCombo->addItem("ki");
	ui->mirrorCombo->addItem("km");
	ui->mirrorCombo->addItem("kn");
	ui->mirrorCombo->addItem("kp");
	ui->mirrorCombo->addItem("kr");
	ui->mirrorCombo->addItem("kw");
	ui->mirrorCombo->addItem("ky");
	ui->mirrorCombo->addItem("kz");
	ui->mirrorCombo->addItem("la");
	ui->mirrorCombo->addItem("lb");
	ui->mirrorCombo->addItem("lc");
	ui->mirrorCombo->addItem("li");
	ui->mirrorCombo->addItem("lk");
	ui->mirrorCombo->addItem("lr");
	ui->mirrorCombo->addItem("ls");
	ui->mirrorCombo->addItem("lt");
	ui->mirrorCombo->addItem("lu");
	ui->mirrorCombo->addItem("lv");
	ui->mirrorCombo->addItem("ly");
	ui->mirrorCombo->addItem("ma");
	ui->mirrorCombo->addItem("mc");
	ui->mirrorCombo->addItem("md");
	ui->mirrorCombo->addItem("me");
	ui->mirrorCombo->addItem("mf");
	ui->mirrorCombo->addItem("mg");
	ui->mirrorCombo->addItem("mh");
	ui->mirrorCombo->addItem("mk");
	ui->mirrorCombo->addItem("ml");
	ui->mirrorCombo->addItem("mm");
	ui->mirrorCombo->addItem("mn");
	ui->mirrorCombo->addItem("mo");
	ui->mirrorCombo->addItem("mp");
	ui->mirrorCombo->addItem("mq");
	ui->mirrorCombo->addItem("mr");
	ui->mirrorCombo->addItem("ms");
	ui->mirrorCombo->addItem("mt");
	ui->mirrorCombo->addItem("mu");
	ui->mirrorCombo->addItem("mv");
	ui->mirrorCombo->addItem("mw");
	ui->mirrorCombo->addItem("mx");
	ui->mirrorCombo->addItem("my");
	ui->mirrorCombo->addItem("mz");
	ui->mirrorCombo->addItem("na");
	ui->mirrorCombo->addItem("nc");
	ui->mirrorCombo->addItem("ne");
	ui->mirrorCombo->addItem("nf");
	ui->mirrorCombo->addItem("ng");
	ui->mirrorCombo->addItem("ni");
	ui->mirrorCombo->addItem("nl");
	ui->mirrorCombo->addItem("no");
	ui->mirrorCombo->addItem("np");
	ui->mirrorCombo->addItem("nr");
	ui->mirrorCombo->addItem("nu");
	ui->mirrorCombo->addItem("nz");
	ui->mirrorCombo->addItem("om");
	ui->mirrorCombo->addItem("pa");
	ui->mirrorCombo->addItem("pe");
	ui->mirrorCombo->addItem("pf");
	ui->mirrorCombo->addItem("pg");
	ui->mirrorCombo->addItem("ph");
	ui->mirrorCombo->addItem("pk");
	ui->mirrorCombo->addItem("pl");
	ui->mirrorCombo->addItem("pm");
	ui->mirrorCombo->addItem("pn");
	ui->mirrorCombo->addItem("pr");
	ui->mirrorCombo->addItem("ps");
	ui->mirrorCombo->addItem("pt");
	ui->mirrorCombo->addItem("pw");
	ui->mirrorCombo->addItem("py");
	ui->mirrorCombo->addItem("qa");
	ui->mirrorCombo->addItem("re");
	ui->mirrorCombo->addItem("ro");
	ui->mirrorCombo->addItem("rs");
	ui->mirrorCombo->addItem("ru");
	ui->mirrorCombo->addItem("rw");
	ui->mirrorCombo->addItem("sa");
	ui->mirrorCombo->addItem("sb");
	ui->mirrorCombo->addItem("sc");
	ui->mirrorCombo->addItem("sd");
	ui->mirrorCombo->addItem("se");
	ui->mirrorCombo->addItem("sg");
	ui->mirrorCombo->addItem("sh");
	ui->mirrorCombo->addItem("si");
	ui->mirrorCombo->addItem("sj");
	ui->mirrorCombo->addItem("sk");
	ui->mirrorCombo->addItem("sl");
	ui->mirrorCombo->addItem("sm");
	ui->mirrorCombo->addItem("sn");
	ui->mirrorCombo->addItem("so");
	ui->mirrorCombo->addItem("sr");
	ui->mirrorCombo->addItem("ss");
	ui->mirrorCombo->addItem("st");
	ui->mirrorCombo->addItem("su");
	ui->mirrorCombo->addItem("sv");
	ui->mirrorCombo->addItem("sx");
	ui->mirrorCombo->addItem("sy");
	ui->mirrorCombo->addItem("sz");
	ui->mirrorCombo->addItem("tc");
	ui->mirrorCombo->addItem("td");
	ui->mirrorCombo->addItem("tf");
	ui->mirrorCombo->addItem("tg");
	ui->mirrorCombo->addItem("th");
	ui->mirrorCombo->addItem("tj");
	ui->mirrorCombo->addItem("tk");
	ui->mirrorCombo->addItem("tl");
	ui->mirrorCombo->addItem("tm");
	ui->mirrorCombo->addItem("tn");
	ui->mirrorCombo->addItem("to");
	ui->mirrorCombo->addItem("tp");
	ui->mirrorCombo->addItem("tr");
	ui->mirrorCombo->addItem("tt");
	ui->mirrorCombo->addItem("tv");
	ui->mirrorCombo->addItem("tw");
	ui->mirrorCombo->addItem("tz");
	ui->mirrorCombo->addItem("ua");
	ui->mirrorCombo->addItem("ug");
	ui->mirrorCombo->addItem("uk");
	ui->mirrorCombo->addItem("um");
	ui->mirrorCombo->addItem("us");
	ui->mirrorCombo->addItem("uy");
	ui->mirrorCombo->addItem("uz");
	ui->mirrorCombo->addItem("va");
	ui->mirrorCombo->addItem("vc");
	ui->mirrorCombo->addItem("ve");
	ui->mirrorCombo->addItem("vg");
	ui->mirrorCombo->addItem("vi");
	ui->mirrorCombo->addItem("vn");
	ui->mirrorCombo->addItem("vu");
	ui->mirrorCombo->addItem("wf");
	ui->mirrorCombo->addItem("ws");
	ui->mirrorCombo->addItem("ye");
	ui->mirrorCombo->addItem("yt");
	ui->mirrorCombo->addItem("za");
	ui->mirrorCombo->addItem("zm");
	ui->mirrorCombo->addItem("zw");
}


void SettingsWidget::chooseDatabasePath( void ) {
	QString path = QFileDialog::getExistingDirectory(this, tr("Database path"), ui->databasePath->text());

	if(!path.isEmpty()) {
		ui->databasePath->setText(path);
		slotDatabasePathChanged();
	}
}
