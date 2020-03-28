#include "elidinglabel.h"

#include <QtGui/QResizeEvent>

#include <QtCore/QDebug>

using namespace Qlam;

ElidingLabel::ElidingLabel(QWidget *parent, Qt::WindowFlags flags)
: QLabel(parent, flags),
  m_elideMode(Qt::ElideMiddle) {
	setTextFormat(Qt::PlainText);
	setWordWrap(false);
}


ElidingLabel::ElidingLabel(const QString & text, QWidget * parent, Qt::WindowFlags flags)
: QLabel(parent, flags),
  m_elideMode(Qt::ElideMiddle) {
	setTextFormat(Qt::PlainText);
	setWordWrap(false);
	setText(text);
}


void ElidingLabel::setText( const QString & text ) {
	if(text != m_text) {
		m_text = text;
		doElide();
		update();
	}
}


void ElidingLabel::setElideMode(const Qt::TextElideMode & mode) {
	if(mode != m_elideMode) {
		m_elideMode = mode;
		doElide();
		update();
	}
}


void ElidingLabel::resizeEvent( QResizeEvent * ev ) {
	if(ev->oldSize().width() != width()) {
		doElide();
	}

	QLabel::resizeEvent(ev);
}


void ElidingLabel::doElide( void ) {
	/* TODO do we need to accommodate frame thickness? */
	QLabel::setText(fontMetrics().elidedText(m_text, m_elideMode, width() - margin() - 1));
}
