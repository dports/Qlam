#ifndef QLAM_SCANPROFILE_H
#define QLAM_SCANPROFILE_H

#include <QtCore/QString>
#include <QtCore/QStringList>

namespace Qlam {
	class ScanProfile {
		public:
			ScanProfile( const QString & name = QString() );

			const QStringList & paths( void ) const {
				return m_paths;
			}

			void setPaths( const QStringList & paths ) {
				m_paths = paths;
			}

			const QString & name( void ) const {
				return m_name;
			}

			void setName( const QString & name ) {
				m_name = name;
			}

			void addPath( const QString & path ) {
				m_paths.append(path);
			}

			void removePath( const QString & path ) {
				m_paths.removeAll(path);
			}

			void removePath( int i ) {
				if(0 <= i && m_paths.count() > i) {
					m_paths.removeAt(i);
				}
			}

			void setPath( int i, const QString & path );

			void clearPaths( void ) {
				m_paths.clear();
			}

		private:
			QString m_name;
			QStringList m_paths;
	};
}

#endif // QLAM_SCANPROFILE_H
