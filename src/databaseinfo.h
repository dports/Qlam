#ifndef QLAM_DATABASEINFO_H
#define QLAM_DATABASEINFO_H

#include <QtCore/QString>
#include <QtCore/QFileInfo>

namespace Qlam {
	class DatabaseInfo {
		public:
			DatabaseInfo( const QString & path = QString() );

			bool isValid( void);

			QString path( void ) const {
				return m_path;
			}

			QString fileName( void ) const {
				return QFileInfo(m_path).fileName();
			}

			QString buildTime( void ) const {
				return m_buildTime;
			}

			QString version( void ) const {
				return m_version;
			}

			int signatureCount( void ) const {
				return m_signatures;
			}

			QString builderName( void ) const {
				return m_builderName;
			}

			QString md5( void ) const {
				return m_md5;
			}

		private:
			void parseLine( const QByteArray & line );
			void parseBuffer( QByteArray & buffer );
			void read( void );

			QString m_path;
			QString m_buildTime;
			QString m_version;
			int m_signatures;
			QString m_builderName;
			QString m_md5;
	};
}

#endif // QLAM_DATABASEINFO_H
