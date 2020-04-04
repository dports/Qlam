#include "updater.h"

#include <QtGlobal>

#include <QtNetwork/QDnsLookup>
#include <QtCore/QList>
#include <QtCore/QDebug>
#include <QtCore/QByteArray>
#include <QtCore/QTimer>
#include <QtCore/QEventLoop>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include "application.h"
#include "settings.h"
#include "databaseinfo.h"
#include "virusdatabasedownloader.h"

using namespace Qlam;

Updater::Updater(QObject *parent)
: QThread(parent),
  m_abort(false) {
}


void Updater::run() {
	Settings * s = qlamApp->settings();
	QString dbPath = s->databasePath();

	if(dbPath.isEmpty()) {
		Q_EMIT updateFailed(tr("You are using the system ClamAV virus databases which are updated automatically."));
		return;
	}

	QList<DatabaseInfo> dbs(Application::instance()->databases());
	int currentBytecodeVersion = -1;
    int currentDailyVersion = -1;
    int currentMainVersion = -1;
    QString latestClamAvVersion;

	for(const auto & db: dbs) {

		if("main.cvd" == db.fileName()) {
			currentMainVersion = db.version().toInt();
		}
		else if("daily.cld" == db.fileName() || "daily.cvd" == db.fileName()) {
			currentDailyVersion = db.version().toInt();
		}
		else if("bytecode.cvd" == db.fileName()) {
			currentBytecodeVersion = db.version().toInt();
		}
	}

	qDebug() << "current";
	qDebug() << "main version: " << currentMainVersion;
	qDebug() << "daily version: " << currentDailyVersion;
	qDebug() << "bytecode version: " << currentBytecodeVersion;

	/* clamav TXT DNS record with database versions looks like this:
	 * <libclamav version>:<main.cvd version>:<daily.cld version>:<x>:<y>:<z>:<safebrowsing version>:<bytecode.cvd version>
	 */
	QByteArray dnsTxtRecord;
	QEventLoop loop;
	connect(this, &Updater::abortRequested, &loop, &QEventLoop::quit);

	{
		/* block for max 5s while the lookup is in progress. */
		QDnsLookup lookup(QDnsLookup::TXT, "current.cvd.clamav.net");
		QTimer t;
		t.setSingleShot(true);
		t.setInterval(5000);
		connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
		connect(&lookup, &QDnsLookup::finished, &loop, &QEventLoop::quit);
		lookup.lookup();
		t.start();
		loop.exec();

		if(m_abort) {
			Q_EMIT aborted();
			return;
		}

		if(!lookup.isFinished()) {
			qDebug() << "DNS request timed out";
			Q_EMIT checkFailed(tr("Failed to fetch latest version numbers for databases."));
			return;
		}

		QList<QDnsTextRecord> records(lookup.textRecords());

		if(1 > records.count()) {
			qDebug() << "DNS request returned no records";
			Q_EMIT checkFailed(tr("Failed to fetch latest version numbers for databases."));
			return;
		}
		else if(1 < records.count()) {
			qDebug() << "DNS request returned too many records";
			Q_EMIT checkFailed(tr("Failed to fetch latest version numbers for databases."));
			return;
		}

		QList<QByteArray> values(records.at(0).values());

		if(1 > values.count()) {
			qDebug() << "DNS request returned no values";
			Q_EMIT checkFailed(tr("Failed to fetch latest version numbers for databases."));
			return;
		}
		else if(1 < values.count()) {
			qDebug() << "DNS request returned too many values";
			Q_EMIT checkFailed(tr("Failed to fetch latest version numbers for databases."));
			return;
		}

		dnsTxtRecord = values.at(0);
	}

	qDebug() << "DNS TXT record:" << dnsTxtRecord;

	QList<QByteArray> numbers = dnsTxtRecord.split(':');
	bool ok;

	/* TODO parse clamav version into major.minor.release */
	latestClamAvVersion = numbers.at(0);

	if(latestClamAvVersion.isEmpty()) {
		Q_EMIT checkFailed("Invalid version number for ClamAV software.");
		return;
	}

	int latestMainVersion = numbers.at(1).toInt(&ok);

	if(!ok) {
		Q_EMIT checkFailed("Invalid version number for the main database.");
		return;
	}

	int latestDailyVersion = numbers.at(2).toInt(&ok);

	if(!ok) {
		Q_EMIT checkFailed("Invalid version number for the daily database.");
		return;
	}

	int latestBytecodeVersion = numbers.at(7).toInt(&ok);

	if(!ok) {
		Q_EMIT checkFailed("Invalid version number for the bytecode database.");
		return;
	}

	qDebug() << "LATEST";
	qDebug() << "main version: " << latestMainVersion;
	qDebug() << "daily version: " << latestDailyVersion;
	qDebug() << "bytecode version: " << latestBytecodeVersion;

	bool doMainUpdate = latestMainVersion > currentMainVersion;
	bool doDailyUpdate = latestDailyVersion > currentDailyVersion;
	bool doBytecodeUpdate = latestBytecodeVersion > currentBytecodeVersion;

	if(doMainUpdate || doDailyUpdate || doBytecodeUpdate) {
		Q_EMIT updatesFound();
	}
	else {
		Q_EMIT upToDate();
		return;
	}

	if(doMainUpdate) {
		Q_EMIT updatingMainDatabase(latestMainVersion);

		QFile f(qlamApp->settings()->databasePath() + "/main.cvd");

		if(!f.open(QIODevice::WriteOnly)) {
			Q_EMIT updateFailed(tr("Failed to open main database file for writing."));
			return;
		}

		/* download the updated database */
		QUrl u(s->updateServer());
qDebug() << "update server:" << u;
		u.setPath("/main.cvd");
qDebug() << "update URL:" << u;
		VirusDatabaseDownloader downloader(u, &f);
		connect(&downloader, qOverload<int>(&VirusDatabaseDownloader::downloadProgress), this, &Updater::updateProgress);
		connect(&downloader, &VirusDatabaseDownloader::finished, &loop, &QEventLoop::quit);

		if(!downloader.download()) {
			Q_EMIT updateFailed(tr("Failed to download main database."));
			Q_EMIT updateComplete();
			return;
		}

		loop.exec();

		if(m_abort) {
			Q_EMIT aborted();
			return;
		}

		if(VirusDatabaseDownloader::NoError != downloader.error()) {
			Q_EMIT updateFailed(tr("Failed to download main database."));
			Q_EMIT updateComplete();
			return;
		}
	}

	if(doDailyUpdate) {
		Q_EMIT updatingDailyDatabase(latestDailyVersion);

		QFile f(qlamApp->settings()->databasePath() + "/daily.cvd");

		if(!f.open(QIODevice::WriteOnly)) {
			Q_EMIT updateFailed(tr("Failed to open daily database file for writing."));
			return;
		}

		/* download the updated database */
		QUrl u(s->updateServer());
		u.setPath("/daily.cvd");
		VirusDatabaseDownloader downloader(u, &f);
		connect(&downloader, qOverload<int>(&VirusDatabaseDownloader::downloadProgress), this, &Updater::updateProgress);
		connect(&downloader, &VirusDatabaseDownloader::finished, &loop, &QEventLoop::quit);

		if(!downloader.download()) {
			Q_EMIT updateFailed(tr("Failed to download daily database."));
			Q_EMIT updateComplete();
			return;
		}

		loop.exec();

		if(m_abort) {
			Q_EMIT aborted();
			return;
		}

		if(VirusDatabaseDownloader::NoError != downloader.error()) {
			Q_EMIT updateFailed(tr("Failed to download daily database."));
			Q_EMIT updateComplete();
			return;
		}
	}

	if(doBytecodeUpdate) {
		Q_EMIT updatingBytecodeDatabase(latestBytecodeVersion);

		QFile file(qlamApp->settings()->databasePath() + "/bytecode.cvd");

		if(!file.open(QIODevice::WriteOnly)) {
			Q_EMIT updateFailed(tr("Failed to open bytecode database file for writing."));
			return;
		}

		/* download the updated database */
		QUrl url(s->updateServer());
		url.setPath("/bytecode.cvd");
		VirusDatabaseDownloader downloader(url, &file);
		connect(&downloader, qOverload<int>(&VirusDatabaseDownloader::downloadProgress), this, &Updater::updateProgress);
		connect(&downloader, &VirusDatabaseDownloader::finished, &loop, &QEventLoop::quit);

		if(!downloader.download()) {
			Q_EMIT updateFailed(tr("Failed to download bytecode database."));
			Q_EMIT updateComplete();
			return;
		}

		loop.exec();

		if(m_abort) {
			Q_EMIT aborted();
			return;
		}

		if(VirusDatabaseDownloader::NoError != downloader.error()) {
			Q_EMIT updateFailed(tr("Failed to download bytecode database."));
			Q_EMIT updateComplete();
			return;
		}
	}

	Q_EMIT updateSucceeded();
	Q_EMIT updateComplete();
}


void Updater::emitUpdateProgress( qint64 received, qint64 total) {
	Q_EMIT updateProgress(int(100.0 * received / total));
}
