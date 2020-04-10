#include <QtGui/QResizeEvent>
#include <QtCore/QDebug>
#include "elidinglabel.h"

using namespace Qlam;

ElidingLabel::ElidingLabel(QWidget *parent, Qt::WindowFlags flags)
: QLabel(parent, flags),
  m_suppressPaintEvents(false),
  m_elideMode(Qt::ElideMiddle) {
	setTextFormat(Qt::PlainText);
	setWordWrap(false);
	installEventFilter(this);
	setSizePolicy({QSizePolicy::Minimum, QSizePolicy::Preferred});
}

ElidingLabel::ElidingLabel(const QString & text, QWidget * parent, Qt::WindowFlags flags)
: ElidingLabel(parent, flags) {
	setText(text);
}

void ElidingLabel::setElideMode(const Qt::TextElideMode & mode) {
	if(mode != m_elideMode) {
		m_elideMode = mode;
		update();
	}
}

void ElidingLabel::resizeEvent(QResizeEvent * event) {
    QLabel::resizeEvent(event);
    QString currentText = text();

    if (!currentText.isEmpty()) {
        QString minimalText = text().at(0) + QStringLiteral("…");
        setMinimumWidth(fontMetrics().horizontalAdvance(minimalText));
    }
}

void ElidingLabel::paintEvent(QPaintEvent * event) {
    if (Qt::ElideNone == elideMode()) {
        QLabel::paintEvent(event);
        return;
    }

    QString originalText = text();
    QString elidedText = fontMetrics().elidedText(originalText, elideMode(), event->rect().width() - margin() - 1);

    // filter out any paint events while we're monkeying with the text
    disablePaintEvents();
    QLabel::setText(elidedText);
    QLabel::paintEvent(event);
    QLabel::setText(originalText);
    enablePaintEvents();
}

bool ElidingLabel::eventFilter(QObject * object, QEvent * event) {
    if (object != this || event->type() != QEvent::Paint) {
        return QLabel::eventFilter(object, event);
    }

    return m_suppressPaintEvents;
}

void ElidingLabel::enablePaintEvents() {
    this->m_suppressPaintEvents = false;
}

void ElidingLabel::disablePaintEvents() {
    this->m_suppressPaintEvents = true;
}
