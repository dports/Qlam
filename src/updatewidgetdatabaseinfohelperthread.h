#ifndef QLAM_UPDATEWIDGETDATABASEINFOHELPERTHREAD_H
#define QLAM_UPDATEWIDGETDATABASEINFOHELPERTHREAD_H

#include <QThread>

#include "databaseinfo.h"

namespace Qlam {
	class UpdateWidgetDatabaseInfoHelperThread
	: public QThread {

			Q_OBJECT

		public:
			explicit UpdateWidgetDatabaseInfoHelperThread( QObject * parent = nullptr );
#if defined(QT_DEBUG)
			virtual ~UpdateWidgetDatabaseInfoHelperThread( void );
#endif

		protected:
			virtual void run( void );

		signals:
			void databaseInfoRead( const DatabaseInfo & db );

		public slots:

		private:
	};
}

#endif // QLAM_UPDATEWIDGETDATABASEINFOHELPERTHREAD_H
