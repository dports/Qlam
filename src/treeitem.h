#ifndef QLAM_TREEITEM_H
#define QLAM_TREEITEM_H

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QStringList>

namespace Qlam {
	class TreeItem {
		public:
			explicit TreeItem( const QString & name = QString() );
			virtual ~TreeItem( void );

			void addChild( TreeItem * item );
			void addChild( const QString & name );

			void addPath( const QStringList & path );

			inline void addPath( const QString & path ) {
				addPath(path.split('/'));
			}

			inline int childCount( void ) const {
				return m_children.count();
			}

			inline bool isTip( void ) const {
				return 0 == childCount();
			}

			inline bool isRoot( void ) const {
				return !m_parent;
			}

			inline bool hasChildren( void ) {
				return 0 < childCount();
			}

			QString child( int i ) const;
			TreeItem * childItem( int i ) const;
			TreeItem * takeChildItem( int i );

			void clear( void );

			inline QString name( void ) const {
				return m_name;
			}

			inline void setName( const QString & name ) {
				m_name = name;
			}

			inline bool containsChild( const QString & name ) {
				return !!findChild(name);
			}

			TreeItem * findChild( const QString & name ) const;

			inline bool containsPath( const QStringList & path ) const {
				return !!findPath(path);
			}

			inline bool containsPath( const QString path ) const {
				return !!findPath(path.split('/'));
			}

			TreeItem * findPath( const QStringList & path ) const;

			inline TreeItem * findPath( const QString path ) const {
				return findPath(path.split('/'));
			}

			QString parent( void ) const;

			TreeItem * parentItem( void ) const {
				return m_parent;
			}

		private:
			void setParent( TreeItem * parent );
			TreeItem * m_parent;
			QList<TreeItem *> m_children;
			QString m_name;

#if defined(QT_DEBUG)
			static int s_count;
#endif
	};
}

#endif // QLAM_TREEITEM_H
