#ifndef QLAM_INFECTEDFILE_H
#define QLAM_INFECTEDFILE_H

#include <QtCore/QString>
#include <QtCore/QStringList>

namespace Qlam {
	class InfectedFile {
		public:
			InfectedFile( const QString & path = QString() );

			inline QString path( void ) const {
				return m_path;
			}

			inline const QStringList & viruses( void ) const {
				return m_viruses;
			}

			inline void addVirus( const QString & name ) {
				m_viruses.append(name);
			}

		private:
			void setPath( const QString & path ) {
				m_path = path;
			}

			QString m_path;
			QStringList m_viruses;
	};
}

#endif // QLAM_INFECTEDFILE_H
