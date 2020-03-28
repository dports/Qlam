#ifndef QLAM_SETTINGSWIDGET_H
#define QLAM_SETTINGSWIDGET_H

#include <QtGlobal>
#include <QtWidgets/QWidget>

namespace Ui {
	class SettingsWidget;
}

namespace Qlam {
	class Settings;

	class SettingsWidget
	: public QWidget {

			Q_OBJECT

		public:
			explicit SettingsWidget( Settings * settings, QWidget * parent = nullptr );
			explicit SettingsWidget( QWidget * parent = nullptr );
			~SettingsWidget( void );

			Settings * settings( void ) {
				return m_settings;
			}

			void setSettings( Settings * settings );

		public Q_SLOTS:
			void syncWithSettings( void );
			void chooseDatabasePath( void );

		private Q_SLOTS:
			void slotDatabasePathChanged( void );
			void slotServerTypeChanged( void );
			void slotMirrorChanged( void );
			void slotCustomServerChanged( void );

		private:
			void connectSettings( void );
			void disconnectSettings( void );
			void listDatabases( void );
			void setupMirrors( void );

			Ui::SettingsWidget * ui;
			Settings * m_settings;
	};
}

#endif // QLAM_SETTINGSWIDGET_H
