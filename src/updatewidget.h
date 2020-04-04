#ifndef QLAM_UPDATEWIDGET_H
#define QLAM_UPDATEWIDGET_H

#include <QtGlobal>

#include <QtCore/QString>
#include <QtWidgets/QWidget>

#include "databaseinfo.h"

namespace Ui {
	class UpdateWidget;
}

namespace Qlam {
	class Updater;

	class UpdateWidget
	: public QWidget {

		Q_OBJECT

		public:
			explicit UpdateWidget(QWidget * = nullptr);
			~UpdateWidget() override;

			QString statusText() const;
			void setStatusText(const QString & text);

		public Q_SLOTS:
			void doUpdate();

		Q_SIGNALS:
			void updateClicked();

		private Q_SLOTS:
			void addDatabase(const DatabaseInfo & db);

			void slotDatabaseInfoThreadFinished();

			void slotUpdatesFound();
			void slotUpdateFailed(const QString &);
			void slotUpdatingMainDatabase(int);
			void slotUpdatingDailyDatabase(int);
			void slotUpdatingBytecodeDatabase(int);
			void slotUpdateSucceeded();
			void slotUpdaterFinished();
			void slotAlreadyUpToDate();

			void listDatabases();
			void setUpdateButtonEnabledState();

		private:
			std::unique_ptr<Ui::UpdateWidget> m_ui;
			Updater * m_updater;
	};
}

#endif // QLAM_UPDATEWIDGET_H
