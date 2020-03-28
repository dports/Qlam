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

#ifndef QLAM_ICONBARWIDGET_H
#define QLAM_ICONBARWIDGET_H

#include <QtGlobal>

#if QT_VERSION >= 0x050000
#include <QtWidgets/QWidget>
#else
#include <QtGui/QWidget>
#endif

#include <QtCore/QDebug>
#include <QtCore/QString>
#include <QtCore/QSize>
#include <QtCore/QList>
#include <QtGui/QIcon>
#include <QtCore/QVariant>
#include <QtCore/QRect>
#include <QtGui/QColor>

class QMouseEvent;
class QPaintEvent;

namespace Qlam {
	class IconBarWidget
	: public QWidget {

		Q_OBJECT

		public:
			IconBarWidget( QWidget * parent = nullptr );
			~IconBarWidget( void );

			int addIcon( const QString & text );
			int addIcon( const QIcon & icon, const QString & text = QString() );

			inline int count( void ) const {
				qDebug() << "item list is at" << ((void *) &m_items);
				return m_items.count();
			}

			int currentIndex( void ) const;
			QSize iconSize( void ) const;
			Qt::TextElideMode elideMode( void ) const;

			inline int insertIcon( int i, const QString & text ) {
				insertIcon(i, QIcon(), text);
			}

			int insertIcon( int i, const QIcon & icon, const QString & text = QString() );

			void moveIcon( int from, int to );
			void removeIcon( int i );

			void setIconSize( const QSize & size );
			void setElideMode( Qt::TextElideMode mode );

			void setIcon( int i, const QIcon & icon );
			void setIconText( int i, const QString & text );
			void setIconData(int i, const QVariant & data );
			void setIconTextColor(int i, const QColor& colour );
			void setIconTextColour(int i, const QColor& colour );
			void setIconToolTip( int i, const QString & tip );

			int iconAt( const QPoint & pos ) const;
			QVariant iconData( int i ) const;
			QIcon icon( int i ) const;
			QRect iconRect( int i ) const;
			QString iconText( int i ) const;
			QColor iconTextColour( int i ) const;
			QColor iconTextColor( int i ) const;
			QString iconToolTip( int i ) const;

			virtual QSize sizeHint( void ) const;

		public slots:
			void setCurrentIndex( int i );

		signals:
			void currentChanged( int i );
			void iconMoved( int from, int to );

		protected:
			virtual void mousePressEvent( QMouseEvent * );
			virtual void paintEvent( QPaintEvent * );

		private:
			void updateIconDisplayRects( void );
			void updateMinimumSize( void );

			struct IconBarItem {
				QIcon icon;
				QString text;
				QVariant data;
				QColor textColour;
				QString toolTip;
				QRect displayRect;
				bool selected;
			};

			QList<IconBarItem *> m_items;
			int m_currentIndex;
			QSize m_iconSize;
			Qt::TextElideMode m_elideMode;
	};
};

#endif // QLAM_ICONBARWIDGET_H
