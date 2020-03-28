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

			VirusDatabaseDownloader( const QUrl & source, QString dest, QObject *parent = nullptr );
			VirusDatabaseDownloader( const QUrl & source, QIODevice * dest, QObject *parent = nullptr );

			bool isValid( void ) const;

			Error error( void ) const {
				return m_errorCode;
			}

		public Q_SLOTS:
			bool download( void );

		signals:
			void donwloadStarted( void );
			void downloadProgress( qint64 bytes, qint64 total );
			void downloadProgress( int pc );
			void downloadSucceeded( void );
			void downloadFailed( void );
			void finished();

		private Q_SLOTS:
			void slotDownloadProgress( qint64, qint64 );
			void slotDownloadFinished( void );
			void slotReadFromReply( void );

		private:
			QUrl m_source;
			QIODevice * m_dest;
			Error m_errorCode;
			QNetworkAccessManager * m_netManager;
			QNetworkReply * m_reply;
	};
}

#endif // QLAM_VIRUSDATABASEDOWNLOADER_H
