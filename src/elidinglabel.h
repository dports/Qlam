#ifndef QLAM_ELIDINGLABEL_H
#define QLAM_ELIDINGLABEL_H

#include <QtGlobal>
#include <QtWidgets/QLabel>
#include <QtCore/QString>

class QResizeEvent;

namespace Qlam {
	class ElidingLabel
	: public QLabel {

			Q_OBJECT

		public:
			explicit ElidingLabel( QWidget * parent = nullptr, Qt::WindowFlags flags = {} );
			explicit ElidingLabel( const QString & text, QWidget * parent = nullptr, Qt::WindowFlags flags = {} );

			void setElideMode( const Qt::TextElideMode & mode );

			[[nodiscard]] inline Qt::TextElideMode elideMode() const {
				return m_elideMode;
			}

		protected:
            void disablePaintEvents();
			void enablePaintEvents();
			void resizeEvent(QResizeEvent *) override;
			void paintEvent(QPaintEvent *) override;
			bool eventFilter(QObject *, QEvent *) override;

		private:
            bool m_suppressPaintEvents;
			Qt::TextElideMode m_elideMode;
	};
}

#endif // QLAM_ELIDINGLABEL_H
