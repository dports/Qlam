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
			explicit UpdateWidget(QWidget *parent = nullptr);
			~UpdateWidget();

			QString statusText( void ) const;
			void setStatusText( const QString & text );

		public Q_SLOTS:
			void doUpdate( void );

		Q_SIGNALS:
			void updateClicked( void );

		private Q_SLOTS:
			void addDatabase( const DatabaseInfo & db );

			void slotDatabaseInfoThreadFinished( void );

			void slotUpdatesFound( void );
			void slotUpdateFailed( const QString & msg );
			void slotUpdatingMainDatabase( int newVersion );
			void slotUpdatingDailyDatabase( int newVersion );
			void slotUpdatingBytecodeDatabase( int newVersion );
			void slotUpdateSucceeded( void );
			void slotUpdaterFinished( void );
			void slotAlreadyUpToDate( void );

			void listDatabases( void );
			void setUpdateButtonEnabledState( void );

		private:
			Ui::UpdateWidget * ui;
			Updater * m_updater;
	};
}

#endif // QLAM_UPDATEWIDGET_H
