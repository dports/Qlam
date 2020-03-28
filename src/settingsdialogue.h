#ifndef SETTINGSDIALOGUE_H
#define SETTINGSDIALOGUE_H

#include <QDialog>

namespace Ui {
	class SettingsDialogue;
}

class QPushButton;

namespace Qlam {
	class Settings;

	class SettingsDialogue
	: public QDialog {

			Q_OBJECT

		public:
			explicit SettingsDialogue( QWidget * parent = nullptr );
			explicit SettingsDialogue( Settings * settings, QWidget *parent = nullptr );
			~SettingsDialogue( void );

			void showCloseButton( void ) {
				setCloseButtonVisible(true);
			}

			void hideCloseButton( void ) {
				setCloseButtonVisible(false);
			}

			void setCloseButtonVisible( bool v );

		public Q_SLOTS:
			void saveSettings( void ) const;

		private:
			Ui::SettingsDialogue * ui;
			QPushButton * m_saveButton;
	};
}

#endif // SETTINGSDIALOGUE_H
