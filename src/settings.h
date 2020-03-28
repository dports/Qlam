#ifndef QLAM_SETTINGS_H
#define QLAM_SETTINGS_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>

namespace Qlam {
	class Settings:
	public QObject {

			Q_OBJECT

		public:
			enum UpdateServerType {
				OfficialMirror = 0,
				Custom
			};

			explicit Settings( QObject * parent = nullptr );
			explicit Settings( const QString & filePath, QObject * parent = nullptr );

			inline QString databasePath( void ) const {
				return m_dbPath;
			}

			inline bool usingSystemDatabases( void ) const {
				return m_dbPath.isEmpty();
			}

			inline UpdateServerType updateServerType( void ) const {
				return m_updateServerType;
			}

			QString updateMirror( void ) const {
				return m_updateMirror;
			}

			QUrl customUpdateServer( void ) const {
				return m_customUpdateServer;
			}

			QUrl updateServer( void ) const;

			bool areModified( void ) const {
				return m_modified;
			}

		public Q_SLOTS:
			inline void setDatabasePath( const QString & path ) {
				if(path != m_dbPath) {
					m_dbPath = path;
					m_modified = true;
					Q_EMIT(databasePathChanged(m_dbPath));
				}
			}

			inline void useSystemDatabases( void ) {
				if(!m_dbPath.isEmpty()) {
					m_dbPath = QString();
					m_modified = true;
					Q_EMIT(databasePathChanged(m_dbPath));
				}
			}

			inline void setUpdateServerType ( const UpdateServerType & type ) {
				if(type != m_updateServerType) {
					m_updateServerType = type;
					m_modified = true;
					Q_EMIT(updateServerTypeChanged(type));
				}
			}

			bool setUpdateMirror( const QString & mirror );

			inline void setCustomUpdateServer( const QString & server ) {
				setCustomUpdateServer(QUrl(server));
			}

			inline void setCustomUpdateServer( const QUrl & server ) {
				if(server != m_customUpdateServer) {
					m_customUpdateServer = server;
					m_modified = true;
					Q_EMIT(customUpdateServerChanged(server));
					Q_EMIT(customUpdateServerChanged(server.toString()));
				}
			}

			void load( void );

			void save( void ) const {
				saveCopyAs(m_filePath);
				m_modified = false;
			}

			void saveAs( const QString & path ) {
				saveCopyAs(path);
				m_filePath = path;
				m_modified = false;
			}

			void saveCopyAs( const QString & path ) const;

		Q_SIGNALS:
			void changed( void );
			void databasePathChanged( const QString & path );
			void updateServerTypeChanged( const UpdateServerType & type );
			void updateMirrorChanged( const QString & mirror );
			void customUpdateServerChanged( const QString & server );
			void customUpdateServerChanged( const QUrl & server );

		private:
			void construct( void );

			static QString updateServerTypeToString( const UpdateServerType & type );
			static UpdateServerType stringToUpdateServerType( const QString & type );

			QString m_filePath;
			QString m_dbPath;
			UpdateServerType m_updateServerType;
			QString m_updateMirror;
			QUrl m_customUpdateServer;

		protected:
			mutable bool m_modified;
	};
}

#endif // QLAM_SETTINGS_H
