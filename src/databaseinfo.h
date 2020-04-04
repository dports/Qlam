#ifndef QLAM_DATABASEINFO_H
#define QLAM_DATABASEINFO_H

#include <QtCore/QString>
#include <QtCore/QFileInfo>

namespace Qlam {
	class DatabaseInfo {
		public:
			explicit DatabaseInfo(const QString & = {});

			bool isValid();

			QString path() const {
				return m_path;
			}

			QString fileName() const {
				return QFileInfo(m_path).fileName();
			}

			QString buildTime() const {
				return m_buildTime;
			}

			QString version() const {
				return m_version;
			}

			int signatureCount() const {
				return m_signatures;
			}

			QString builderName() const {
				return m_builderName;
			}

			QString md5() const {
				return m_md5;
			}

		private:
			void parseLine(const QByteArray &);
			void parseBuffer(QByteArray & );
			void read();

			QString m_path;
			QString m_buildTime;
			QString m_version;
			int m_signatures;
			QString m_builderName;
			QString m_md5;
	};
}

#endif // QLAM_DATABASEINFO_H
