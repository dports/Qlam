/*
	 <one line to give the library's name and an idea of what it does.>
	 Copyright (C) <year>  <name of author>

	 This library is free software; you can redistribute it and/or
	 modify it under the terms of the GNU Lesser General Public
	 License as published by the Free Software Foundation; either
	 version 2.1 of the License, or (at your option) any later version.

	 This library is distributed in the hope that it will be useful,
	 but WITHOUT ANY WARRANTY; without even the implied warranty of
	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	 Lesser General Public License for more details.

	 You should have received a copy of the GNU Lesser General Public
	 License along with this library; if not, write to the Free Software
	 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include <QtGlobal>

#include <QtCore/QDebug>
#include <QtGui/QMouseEvent>
#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>

#if QT_VERSION >= 0x050000
#include <QtWidgets/QStyle>
#include <QtWidgets/QStyleOptionViewItemV4>
#include <QtWidgets/QStyleOptionFrameV3>
#else
#include <QtGui/QStyle>
#include <QtGui/QStyleOptionViewItemV4>
#include <QtGui/QStyleOptionFrameV3>
#endif

#include "iconbarwidget.h"


#define QLAM_ICONBAR_DEFAULT_ICON_WIDTH 32
#define QLAM_ICONBAR_DEFAULT_ICON_HEIGHT 32
#define QLAM_ICONBAR_DEFAULT_VMARGIN 2
#define QLAM_ICONBAR_DEFAULT_HMARGIN 2
#define QLAM_ICONBAR_DEFAULT_SPACING 8

using namespace Qlam;


IconBarWidget::IconBarWidget( QWidget * parent )
:	QWidget(parent),
	m_items(),
	m_currentIndex(-1),
	m_iconSize(QLAM_ICONBAR_DEFAULT_ICON_WIDTH, QLAM_ICONBAR_DEFAULT_ICON_HEIGHT),
	m_elideMode(Qt::ElideRight) {
	qDebug() << "created IconBarWidget" << ((void *) this);
	qDebug() << "current index" << m_currentIndex;
	qDebug() << "icon size" << m_iconSize;
	qDebug() << "elide mode" << m_elideMode;
	qDebug() << "items:" << m_items.count();
	qDebug() << "item list is at" << ((void *) &m_items);
	updateMinimumSize();
}


IconBarWidget::~IconBarWidget( void ) {
	qDebug() << "deleting remaining items";

	foreach(IconBarItem * it, m_items) {
		qDebug() << "deleting item at" << ((void *) it);
		delete it;
	}

	qDebug() << "clearning items list";
	m_items.clear();
}


int IconBarWidget::addIcon( const QString & text ) {
qDebug() << "calling insertIcon on widget";
	return insertIcon(count(), text);
}


int IconBarWidget::addIcon( const QIcon & icon, const QString & text ) {
	return insertIcon(count(), icon, text);
}


int IconBarWidget::currentIndex( void ) const {
	return m_currentIndex;
}


QSize IconBarWidget::iconSize( void ) const {
	return m_iconSize;
}


Qt::TextElideMode IconBarWidget::elideMode( void ) const {
	return m_elideMode;
}


int IconBarWidget::insertIcon( int i, const QIcon & icon, const QString & text ) {
	if(0 > i) {
		i = 0;
	}

	if(count() < i) {
		i = count();
	}

	IconBarItem * it = new IconBarItem();
qDebug() << "setting new item icon";
	it->icon = icon;
qDebug() << "setting new item text";
	it->text = text;
qDebug() << "setting new item selected state";
	it->selected = false;
qDebug() << "setting new item text colour";
	it->textColour = palette().text().color();
qDebug() << "inserting item into list";
	m_items.insert(i, it);
qDebug() << "updating display rects for icons";
	updateIconDisplayRects();
	qDebug() << "recalculating min size";
	updateMinimumSize();
	qDebug() << "updating display";
	update();
	return i;
}


void IconBarWidget::moveIcon( int from, int to ) {
	if(from < 0 || from >= count()) return;
	if(to < 0 || to >= count()) return;
	if(to == from) return;
	IconBarItem * it = m_items.at(from);
	m_items.removeAt(from);
	if(to > from) --to;
	m_items.insert(to, it);
	updateIconDisplayRects();
	updateMinimumSize();
	update();
}


void IconBarWidget::removeIcon( int i ) {
	delete m_items.at(i);
	m_items.removeAt(i);
	updateIconDisplayRects();
	updateMinimumSize();
	update();
}


void IconBarWidget::setIconSize( const QSize & size ) {
	m_iconSize = size;
	updateIconDisplayRects();
	updateMinimumSize();
	update();
}


void IconBarWidget::setElideMode( Qt::TextElideMode mode ) {
	m_elideMode = mode;
	update();
}


void IconBarWidget::setIcon( int i, const QIcon & icon ) {
	if(i < 0 || i >= count()) return;
	m_items.at(i)->icon = icon;
	update();
}


void IconBarWidget::setIconText( int i, const QString & text ) {
	if(i < 0 || i >= count()) return;
	m_items.at(i)->text = text;
	update();
}


void IconBarWidget::setIconData(int i, const QVariant & data ) {
	if(i < 0 || i >= count()) return;
	m_items.at(i)->data = data;
}


void IconBarWidget::setIconTextColor(int i, const QColor & colour ) {
	setIconTextColour(i, colour);
}


void IconBarWidget::setIconTextColour(int i, const QColor & colour ) {
	if(i < 0 || i >= count()) return;
	m_items.at(i)->textColour = colour;
	update();
}


void IconBarWidget::setIconToolTip( int i, const QString & tip ) {
	if(i < 0 || i >= count()) return;
	m_items.at(i)->toolTip = tip;
}


int IconBarWidget::iconAt( const QPoint & pos ) const {
	for(int i = 0; i < m_items.size(); ++i) {
		if(m_items.at(i)->displayRect.contains(pos)) return i;
	}

	return -1;
}


QVariant IconBarWidget::iconData( int i ) const {
	if(i < 0 || i >= count()) return QVariant();
	return m_items.at(i)->data;
}


QIcon IconBarWidget::icon( int i ) const {
	if(i < 0 || i >= count()) return QIcon();
	return m_items.at(i)->icon;
}


QRect IconBarWidget::iconRect( int i ) const {
	if(i < 0 || i >= count()) return QRect();
	return m_items.at(i)->displayRect;
}


QString IconBarWidget::iconText( int i ) const {
	if(i < 0 || i >= count()) return QString();
	return m_items.at(i)->text;
}


QColor IconBarWidget::iconTextColour( int i ) const {
	if(i < 0 || i >= count()) return QColor();
	return m_items.at(i)->textColour;
}


QColor IconBarWidget::iconTextColor( int i ) const {
	return iconTextColour(i);
}


QString IconBarWidget::iconToolTip( int i ) const {
	if(i < 0 || i >= count()) return QString();
	return m_items.at(i)->toolTip;
}


void IconBarWidget::setCurrentIndex( int i ) {
	m_currentIndex = i;
	update();
}


QSize IconBarWidget::sizeHint( void ) const {
	return QSize(75, 250);
}


//QSize IconBarWidget::minimumSize( void ) const {
//	if(m_items.isEmpty()) return QSize(width(), m_iconSize.height() + QLAM_ICONBAR_DEFAULT_SPACING + QLAM_ICONBAR_DEFAULT_SPACING);
//	return QSize(width(), m_items.last()->displayRect.bottom());
//}


void IconBarWidget::mousePressEvent( QMouseEvent * ev ) {
	int i = iconAt(ev->pos());

	if(i > -1) {
		ev->accept();

		if(i != m_currentIndex) {
			m_currentIndex = i;
			update();
			emit(currentChanged(m_currentIndex));
		}
	}
	else
		QWidget::mousePressEvent(ev);
}


void IconBarWidget::paintEvent( QPaintEvent * ev ) {
	QPainter p(this);
	int c = count();
	QFontMetrics fm(fontMetrics());

	/* draw the overall border */
	QStyleOptionFrameV3 widgetFrameStyleOpt;
	widgetFrameStyleOpt.initFrom(this);
	widgetFrameStyleOpt.rect = rect();
	widgetFrameStyleOpt.frameShape = QFrame::Panel;
	style()->drawPrimitive(QStyle::PE_FrameLineEdit, &widgetFrameStyleOpt, &p, this);

	/* draw the icons */
	updateIconDisplayRects();
	QStyleOptionViewItemV4 selectedIconStyleOpt;
	selectedIconStyleOpt.initFrom(this);
	selectedIconStyleOpt.backgroundBrush = palette().highlight();

	for(int i = 0; i < c; ++i) {
		IconBarItem * it = m_items.at(i);
		if(it->displayRect.top() > height()) break;

		if(i == m_currentIndex) {
			selectedIconStyleOpt.rect = it->displayRect;
			style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &selectedIconStyleOpt, &p, this);
		}

		p.drawPixmap(it->displayRect.left() + (it->displayRect.width() / 2) - (m_iconSize.width() / 2), it->displayRect.top() + QLAM_ICONBAR_DEFAULT_SPACING, it->icon.pixmap(m_iconSize));

		if(!it->text.isEmpty()) {
			QString text = it->text;
			if(fm.width(text) > (it->displayRect.width() - QLAM_ICONBAR_DEFAULT_SPACING)) text = fm.elidedText(text, m_elideMode, m_iconSize.width());

			if(i == m_currentIndex)
				p.setPen(palette().highlightedText().color());
			else
				p.setPen(it->textColour);

			p.drawText(QLAM_ICONBAR_DEFAULT_HMARGIN + (it->displayRect.width() / 2) - (fm.width(text) / 2), it->displayRect.top() + QLAM_ICONBAR_DEFAULT_SPACING + QLAM_ICONBAR_DEFAULT_SPACING + QLAM_ICONBAR_DEFAULT_SPACING + m_iconSize.height(), text);
		}
	}
}


void IconBarWidget::updateMinimumSize( void ) {
	int w = m_iconSize.width() + QLAM_ICONBAR_DEFAULT_SPACING + QLAM_ICONBAR_DEFAULT_SPACING + QLAM_ICONBAR_DEFAULT_HMARGIN + QLAM_ICONBAR_DEFAULT_HMARGIN;
	if(m_items.isEmpty()) setMinimumSize(w, m_iconSize.height() + QLAM_ICONBAR_DEFAULT_SPACING + QLAM_ICONBAR_DEFAULT_SPACING);
	else setMinimumSize(w, m_items.last()->displayRect.bottom());

}


void IconBarWidget::updateIconDisplayRects( void ) {
	QFontMetrics fm(fontMetrics());
	int y = QLAM_ICONBAR_DEFAULT_VMARGIN;
	int right = width() - QLAM_ICONBAR_DEFAULT_HMARGIN - 1;

	foreach(IconBarItem * it, m_items) {
		it->displayRect.setTopLeft(QPoint(QLAM_ICONBAR_DEFAULT_HMARGIN, y));
		it->displayRect.setBottomRight(QPoint(right, y + QLAM_ICONBAR_DEFAULT_SPACING + QLAM_ICONBAR_DEFAULT_SPACING + m_iconSize.height() + (it->text.isEmpty() ? 0 : fm.height() + QLAM_ICONBAR_DEFAULT_SPACING)));
		y += it->displayRect.height();
	}
}
