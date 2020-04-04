#include "treeitem.h"

using namespace Qlam;

TreeItem::TreeItem(QString name) noexcept
: m_parent(nullptr),
  m_children(),
  m_name(std::move(name)) {
}

TreeItem::~TreeItem() {
	clear();
}

void TreeItem::addChild(TreeItem * item) {
	item->setParent(this);
}

void TreeItem::addChild(const QString & name) {
	addChild(new TreeItem(name));
}

void TreeItem::addPath(const QStringList & path) {
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

QString TreeItem::child(int idx) const {
	return childItem(idx)->name();
}

TreeItem * TreeItem::childItem(int idx) const {
	Q_ASSERT(idx >= 0 && idx < m_children.count());
	return m_children.at(idx);
}

TreeItem * TreeItem::takeChildItem(int idx) {
	Q_ASSERT(idx >= 0 && idx < m_children.count());
	TreeItem * ret = m_children.at(idx);
	ret->setParent(nullptr);
	return ret;
}

void TreeItem::clear() {
	qDeleteAll(m_children);
	m_children.clear();
}

TreeItem * TreeItem::findChild(const QString & name) const {
	for(auto * child : m_children) {
		if(child->name() == name) {
			return child;
		}
	}

	return nullptr;
}

TreeItem * TreeItem::findPath(const QStringList & path) const {
	if(path.isEmpty()) {
		return nullptr;
	}

	QStringList myPath(path);
	TreeItem * child = findChild(myPath.takeAt(0));

	if(!child || myPath.isEmpty()) {
		return child;
	}

	return child->findPath(myPath);
}

QString TreeItem::parent() const {
	Q_ASSERT(m_parent != nullptr);
	return m_parent->name();
}

void TreeItem::setParent(TreeItem * parent) {
	if(parent == m_parent) {
		return;
	}

	if(m_parent != nullptr) {
		m_parent->m_children.removeAll(this);
	}

	m_parent = parent;

	if(m_parent != nullptr) {
		m_parent->m_children.append(this);
	}
}
