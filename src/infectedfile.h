#ifndef QLAM_FILEWITHISSUES_H
#define QLAM_FILEWITHISSUES_H

#include <QtCore/QString>
#include <QtCore/QStringList>

namespace Qlam {
	class FileWithIssues {
		public:
			explicit FileWithIssues(const QString & = {});

			inline QString path() const {
				return m_path;
			}

			inline const QStringList & issues() const {
				return m_issues;
			}

			inline void addIssue(const QString & name) {
				m_issues.append(name);
			}

		private:
			void setPath(const QString & path) {
				m_path = path;
			}

			QString m_path;
			QStringList m_issues;
	};
}

#endif // QLAM_FILEWITHISSUES_H
