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
			explicit ElidingLabel( QWidget * parent = nullptr, Qt::WindowFlags flags = 0 );
			explicit ElidingLabel( const QString & text, QWidget * parent = nullptr, Qt::WindowFlags flags = 0 );

			inline QString text( void ) const {
				return m_text;
			}

			void setElideMode( const Qt::TextElideMode & mode );

			inline Qt::TextElideMode elideMode( void ) const {
				return m_elideMode;
			}

		signals:

		public slots:
			void setText( const QString & text );

		protected:
			virtual void resizeEvent( QResizeEvent * ev );

		private:
			void doElide( void );

			QString m_text;
			Qt::TextElideMode m_elideMode;
	};
}

#endif // QLAM_ELIDINGLABEL_H
