#include "virusdatabasedownloader.h"

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

using namespace Qlam;

VirusDatabaseDownloader::VirusDatabaseDownloader( const QUrl & source, QString dest, QObject * parent)
: QObject(parent),
  m_source(source),
  m_dest(0),
  m_errorCode(NoError),
  m_netManager(0),
  m_reply(0) {
qDebug() << "source provided:" << source;
qDebug() << "source is set to:" << m_source;
	m_dest = new QFile(dest, this);
	m_dest->open(QIODevice::WriteOnly);
	m_netManager = new QNetworkAccessManager(this);
}


VirusDatabaseDownloader::VirusDatabaseDownloader( const QUrl & source, QIODevice * dest, QObject * parent )
: QObject(parent),
  m_source(source),
  m_dest(dest),
  m_errorCode(NoError),
  m_netManager(0),
  m_reply(0) {
qDebug() << "source provided:" << source;
qDebug() << "source is set to:" << m_source;
	m_netManager = new QNetworkAccessManager(this);
}


bool VirusDatabaseDownloader::isValid() const {
	return !!m_dest && m_dest->isOpen() && (m_dest->openMode() & QIODevice::WriteOnly) && m_source.isValid();
}


bool VirusDatabaseDownloader::download( void ) {
	Q_ASSERT(!!m_netManager);

	if(!!m_reply) {
		qCritical() << "download already in progress";
		return false;
	}

#if defined(QT_DEBUG)
qDebug() << "downloading from" << m_source;
#endif

	m_reply = m_netManager->get(QNetworkRequest(m_source));
	connect(m_reply, SIGNAL(downloadProgress(qint64,qint64)), this, SIGNAL(downloadProgress(qint64,qint64)));
	connect(m_reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(slotDownloadProgress(qint64,qint64)));
	connect(m_reply, SIGNAL(finished()), this, SLOT(slotDownloadFinished()));
	connect(m_reply, SIGNAL(readyRead()), this, SLOT(slotReadFromReply()));
	return true;
}


void VirusDatabaseDownloader::slotDownloadProgress(qint64 bytes, qint64 total ) {
	Q_EMIT(downloadProgress(int((100.0 * bytes) / total)));
}


void VirusDatabaseDownloader::slotReadFromReply( void ) {
	Q_ASSERT(!!m_reply);
	Q_ASSERT(!!m_dest);
	qint64 available = m_reply->bytesAvailable();
	char buffer[1024];
	int consecutiveErrorCount = 0;

	while(5 > consecutiveErrorCount && 0 < available) {
		qint64 bytesIn = m_reply->read(buffer, 1024);

		if(-1 == bytesIn) {
			++consecutiveErrorCount;
			continue;
		}

		consecutiveErrorCount = 0;
		available -= bytesIn;

		while(5 > consecutiveErrorCount && 0 < bytesIn) {
			qint64 bytesOut = m_dest->write(buffer, bytesIn);

			if(-1 == bytesOut) {
				++consecutiveErrorCount;
				continue;
			}

			bytesIn -= bytesOut;
		}

		if(5 <= consecutiveErrorCount) {
			qDebug() << "failed to write" << bytesIn << "bytes to destination";
			disconnect(m_reply);
			m_reply->abort();
			m_reply->deleteLater();
			m_reply = 0;
			m_errorCode = WriteError;
			Q_EMIT(downloadFailed());
			Q_EMIT(finished());
			return;
		}

		consecutiveErrorCount = 0;
	}

	if(5 <= consecutiveErrorCount) {
		qDebug() << "failed to read" << available << "bytes from source";
		disconnect(m_reply);
		m_reply->abort();
		m_reply->deleteLater();
		m_reply = 0;
		m_errorCode = ReadError;
		Q_EMIT(downloadFailed());
		Q_EMIT(finished());
		return;
	}
}


void VirusDatabaseDownloader::slotDownloadFinished( void ) {
	Q_ASSERT(!!m_reply);
	/* probably don't need to do this... */
	slotReadFromReply();

	/* reading from reply can delete and nullify m_reply on read or write error,
	 * so need to check it here */
	if(!!m_reply) {
		if(QNetworkReply::NoError != m_reply->error()) {
			m_errorCode = ReadError;
			Q_EMIT(downloadFailed());
		}
		else {
			m_errorCode = NoError;
			Q_EMIT(downloadSucceeded());
		}

		Q_EMIT(finished());
		m_reply->deleteLater();
		m_reply = 0;
	}
}
