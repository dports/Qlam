#ifndef QLAM_TREEITEM_H
#define QLAM_TREEITEM_H

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QStringList>

namespace Qlam {
	class TreeItem {
		public:
			explicit TreeItem(QString = {}) noexcept;
			virtual ~TreeItem();

			void addChild(TreeItem *);
			void addChild(const QString &);

			void addPath(const QStringList &);

			inline void addPath(const QString & path) {
				addPath(path.split('/'));
			}

			inline int childCount() const {
				return m_children.count();
			}

			inline bool isTip() const {
				return 0 == childCount();
			}

			inline bool isRoot() const {
				return !m_parent;
			}

			inline bool hasChildren() {
				return 0 < childCount();
			}

			QString child(int) const;
			TreeItem * childItem(int) const;
			TreeItem * takeChildItem(int);

			void clear();

			inline QString name() const {
				return m_name;
			}

			inline void setName(const QString & name) {
				m_name = name;
			}

			inline bool containsChild(const QString & name) {
				return findChild(name) != nullptr;
			}

			TreeItem * findChild(const QString &) const;

			inline bool containsPath(const QStringList & path) const {
				return findPath(path) != nullptr;
			}

			inline bool containsPath(const QString & path) const {
				return findPath(path.split('/')) != nullptr;
			}

			TreeItem * findPath(const QStringList &) const;

			inline TreeItem * findPath(const QString & path) const {
				return findPath(path.split('/'));
			}

			QString parent() const;

			TreeItem * parentItem() const {
				return m_parent;
			}

		private:
			void setParent(TreeItem * parent);
			TreeItem * m_parent;
			QList<TreeItem *> m_children;
			QString m_name;
	};
}

#endif // QLAM_TREEITEM_H
