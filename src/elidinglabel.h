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

			inline QString text() const {
				return m_text;
			}

			void setElideMode( const Qt::TextElideMode & mode );

			inline Qt::TextElideMode elideMode() const {
				return m_elideMode;
			}

		Q_SIGNALS:

		public Q_SLOTS:
			void setText(const QString &);

		protected:
			void resizeEvent(QResizeEvent *) override;

		private:
			void doElide();

			QString m_text;
			Qt::TextElideMode m_elideMode;
	};
}

#endif // QLAM_ELIDINGLABEL_H
