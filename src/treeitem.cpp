#include "treeitem.h"

using namespace Qlam;

#if defined(QT_DEBUG)
#include <QDebug>
int TreeItem::s_count = 0;
#endif


TreeItem::TreeItem( const QString & name )
: m_parent(0),
  m_children(),
  m_name(name) {
#if defined(QT_DEBUG)
//qDebug() << "creating TreeItem" << name << "(" << ((void *) this) << ")";
s_count++;
#endif
}


TreeItem::~TreeItem() {
#if defined(QT_DEBUG)
//qDebug() << "deleting TreeItem(" << ((void *) this) << ")";
s_count--;
//qDebug() << s_count << "TreeItems remain";
#endif
	clear();
}


void TreeItem::addChild( TreeItem * item ) {
	item->setParent(this);
}


void TreeItem::addChild( const QString & name ) {
	addChild(new TreeItem(name));
}


void TreeItem::addPath( const QStringList & path ) {
	if(path.isEmpty()) {
		return;
	}

	QStringList myPath(path);
	QString name = myPath.takeAt(0);
	TreeItem * child = findChild(name);

	if(!child) {
		child = new TreeItem(name);
		addChild(child);
	}

	child->addPath(myPath);
}


QString TreeItem::child( int i ) const {
	return childItem(i)->name();
}


TreeItem * TreeItem::childItem( int i ) const {
	Q_ASSERT(i >= 0 && i < m_children.count());
	return m_children.at(i);
}


TreeItem * TreeItem::takeChildItem( int i ) {
	Q_ASSERT(i >= 0 && i < m_children.count());
	TreeItem * ret = m_children.at(i);
	ret->setParent(0);
	return ret;
}


void TreeItem::clear() {
	qDeleteAll(m_children);
	m_children.clear();
}


TreeItem * TreeItem::findChild( const QString & name ) const {
	for(auto * child : m_children) {
		if(child->name() == name) {
			return child;
		}
	}

	return 0;
}


TreeItem * TreeItem::findPath( const QStringList & path ) const {
	if(path.isEmpty()) {
		return 0;
	}

	QStringList myPath(path);
	TreeItem * child = findChild(myPath.takeAt(0));

	if(!child || myPath.isEmpty()) {
		return child;
	}

	return child->findPath(myPath);
}


QString TreeItem::parent() const {
	Q_ASSERT(!!m_parent);
	return m_parent->name();
}


void TreeItem::setParent( TreeItem * parent ) {
	if(parent == m_parent) {
		return;
	}

	if(!!m_parent) {
		m_parent->m_children.removeAll(this);
	}

	m_parent = parent;

	if(!!m_parent) {
		m_parent->m_children.append(this);
	}
}
