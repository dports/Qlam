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
			explicit SettingsWidget(Settings *, QWidget * = nullptr);
			explicit SettingsWidget(QWidget * = nullptr);
			~SettingsWidget()override;

			Settings * settings() {
				return m_settings;
			}

			void setSettings(Settings *);

		public Q_SLOTS:
			void syncWithSettings();
			void chooseDatabasePath();

		private Q_SLOTS:
			void slotDatabasePathChanged();
			void slotServerTypeChanged();
			void slotMirrorChanged();
			void slotCustomServerChanged();

		private:
			void connectSettings();
			void disconnectSettings();
			void listDatabases();
			void setupMirrors();

			std::unique_ptr<Ui::SettingsWidget> m_ui;
			Settings * m_settings;
	};
}

#endif // QLAM_SETTINGSWIDGET_H
