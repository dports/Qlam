#ifndef QLAM_VIRUSDATABASEDOWNLOADER_H
#define QLAM_VIRUSDATABASEDOWNLOADER_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>

class QIODevice;
class QNetworkAccessManager;
class QNetworkReply;

namespace Qlam {
	class VirusDatabaseDownloader
	: public QObject {

			Q_OBJECT

		public:
			enum Error {
				NoError = 0,
				ReadError,
				WriteError
			};

			VirusDatabaseDownloader(const QUrl &, const QString &, QObject * = nullptr);
			VirusDatabaseDownloader(const QUrl &, QIODevice *, QObject * = nullptr);

			bool isValid() const;

			Error error() const {
				return m_errorCode;
			}

		public Q_SLOTS:
			bool download();

		signals:
			void donwloadStarted();
			void downloadProgress(qint64, qint64);
			void downloadProgress(int);
			void downloadSucceeded();
			void downloadFailed();
			void finished();

		private Q_SLOTS:
			void slotDownloadProgress(qint64, qint64);
			void slotDownloadFinished();
			void slotReadFromReply();

		private:
			QUrl m_source;
			QIODevice * m_dest;
			Error m_errorCode;
			QNetworkAccessManager * m_netManager;
			QNetworkReply * m_reply;
	};
}

#endif // QLAM_VIRUSDATABASEDOWNLOADER_H
