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
			explicit ScanProfileChooser(QWidget * = nullptr);
			~ScanProfileChooser() override;

			bool addProfile(const ScanProfile &);
			bool removeProfile(int);

			ScanProfile currentProfile();

			int currentProfileIndex() const {
				return m_currentProfileIndex;
			}

		public Q_SLOTS:
			void clearProfiles();

		Q_SIGNALS:
			void profileChosen(int);

		private Q_SLOTS:
			void slotProfileButtonClicked(QAbstractButton *);

		private:
			std::unique_ptr<Ui::ScanProfileChooser> m_ui;
			QButtonGroup * m_buttons;
			QVBoxLayout * m_buttonLayout;
			int m_currentProfileIndex;
	};
}

#endif // QLAM_SCANPROFILECHOOSER_H
