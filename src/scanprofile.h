#ifndef QLAM_SCANPROFILE_H
#define QLAM_SCANPROFILE_H

#include <QtCore/QString>
#include <QtCore/QStringList>

namespace Qlam {
	class ScanProfile {
		public:
			explicit ScanProfile(const QString & = {});

			const QStringList & paths() const {
				return m_paths;
			}

			void setPaths(const QStringList & paths) {
				m_paths = paths;
			}

			const QString & name() const {
				return m_name;
			}

			void setName(const QString & name) {
				m_name = name;
			}

			void addPath(const QString & path) {
				m_paths.append(path);
			}

			void removePath(const QString & path) {
				m_paths.removeAll(path);
			}

			void removePath(int idx) {
				if(0 <= idx && m_paths.count() > idx) {
					m_paths.removeAt(idx);
				}
			}

			void setPath(int, const QString &);

			void clearPaths() {
				m_paths.clear();
			}

		private:
			QString m_name;
			QStringList m_paths;
	};
}

#endif // QLAM_SCANPROFILE_H
