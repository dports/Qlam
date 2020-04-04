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
			explicit SettingsDialogue(QWidget * = nullptr);
			explicit SettingsDialogue(Settings *, QWidget * = nullptr);
			~SettingsDialogue() override;

			void showCloseButton() {
				setCloseButtonVisible(true);
			}

			void hideCloseButton() {
				setCloseButtonVisible(false);
			}

			void setCloseButtonVisible(bool);

		public Q_SLOTS:
			void saveSettings() const;

		private:
			std::unique_ptr<Ui::SettingsDialogue> m_ui;
			QPushButton * m_saveButton;
	};
}

#endif // SETTINGSDIALOGUE_H
