#include "settings.h"

#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtCore/QUrl>
#include <QtGlobal>
#include "application.h"

using namespace Qlam;

Settings::Settings( QObject * parent )
: Settings(QStringLiteral(), parent) {
}


Settings::Settings(QString filePath, QObject * parent)
: QObject(parent),
  m_filePath(std::move(filePath)),
  m_dbPath(),
  m_updateServerType(OfficialMirror),
  m_updateMirror(),
  m_customUpdateServer(),
  m_modified(false) {
    load();
    connect(this, &Settings::databasePathChanged, this, &Settings::changed);
    connect(this, &Settings::updateServerTypeChanged, this, &Settings::changed);
    connect(this, &Settings::updateMirrorChanged, this, &Settings::changed);
    connect(this, qOverload<const QString &>(&Settings::customUpdateServerChanged), this, &Settings::changed);
}

bool Settings::setUpdateMirror( const QString & mirror ) {
	if(mirror != m_updateMirror) {
		// TODO validate mirror
		m_updateMirror = mirror;
		m_modified = true;
		Q_EMIT updateMirrorChanged(mirror);
	}

	return true;
}

QUrl Settings::updateServer() const {
	if(Custom == updateServerType()) {
		return customUpdateServer();
	}

	return QUrl(QString("http://db.%1.clamav.net").arg(updateMirror()));
}

void Settings::fillSettings(QSettings & settings) const {
	settings.setValue("database.path", databasePath());
	settings.setValue("updateserver.type", updateServerTypeToString(updateServerType()));
	settings.setValue("updateserver.mirror", updateMirror());
	settings.setValue("updateserver.customserver.url", customUpdateServer().toString());
}

void Settings::readSettings(const QSettings & settings) {
	setDatabasePath(settings.value("database.path", QString()).toString());
	setUpdateServerType(stringToUpdateServerType(settings.value("updateserver.type", "OfficialMirror").toString()));
	setUpdateMirror(settings.value("updateserver.mirror", "").toString());
	setCustomUpdateServer(settings.value("updateserver.customserver.url", "").toString());
}

void Settings::load() {
	if(m_filePath.isEmpty()) {
		qDebug() << "loading settings from default path";
		readSettings(QSettings(QSettings::IniFormat, QSettings::UserScope, qlamApp->organizationName(), qlamApp->applicationName() + ".appconfig"));
	}
	else {
		qDebug() << "loading settings from" << m_filePath;
		readSettings(QSettings(m_filePath, QSettings::IniFormat));
	}

	m_modified = false;
	Q_EMIT changed();
}

void Settings::saveCopyAs( const QString & path ) const {
	if(m_filePath.isEmpty()) {
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, qlamApp->organizationName(), qlamApp->applicationName() + ".appconfig");
		fillSettings(settings);		
		qDebug() << "Saved settings to file" << settings.fileName();
	}
	else {
		QSettings settings(path, QSettings::IniFormat);
		fillSettings(settings);
		qDebug() << "Saved settings to file" << settings.fileName();
	}
}

QString Settings::updateServerTypeToString( const Settings::UpdateServerType & type ) {
	switch(type) {
		case OfficialMirror:
			return QString::fromUtf8("OfficialMirror");

		case Custom:
			return QString::fromUtf8("Custom");
	}

	return QString();
}

Settings::UpdateServerType Settings::stringToUpdateServerType( const QString & type ) {
	if("OfficialMirror" == type) {
		return OfficialMirror;
	}
	else if("Custom" == type) {
		return Custom;
	}

	return UpdateServerType(-1);
}
