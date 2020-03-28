#ifndef QLAM_SCANPROFILECHOOSER_H
#define QLAM_SCANPROFILECHOOSER_H

#include <QtGlobal>

#include <QtWidgets/QWidget>
#include <QtWidgets/QButtonGroup>
#include <QtCore/QList>
#include "scanprofile.h"

class QVBoxLayout;

namespace Ui {
	class ScanProfileChooser;
}

namespace Qlam {
	class ScanProfileChooser
	: public QWidget {

			Q_OBJECT

		public:
			explicit ScanProfileChooser( QWidget * parent = nullptr );
			~ScanProfileChooser( void );

			bool addProfile( const ScanProfile & profile );
			bool removeProfile( int i );

			ScanProfile currentProfile( void );

			int currentProfileIndex( void ) const {
				return m_currentProfileIndex;
			}

		public Q_SLOTS:
			void clearProfiles( void );

		Q_SIGNALS:
			void profileChosen( int );

		private Q_SLOTS:
			void slotProfileButtonClicked( QAbstractButton * b );

		private:
			Ui::ScanProfileChooser *ui;
			QButtonGroup * m_buttons;
			QVBoxLayout * m_buttonLayout;
			int m_currentProfileIndex;
	};
}

#endif // QLAM_SCANPROFILECHOOSER_H
