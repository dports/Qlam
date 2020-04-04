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
			~UpdateWidgetDatabaseInfoHelperThread() override;
#endif

		protected:
			void run() override;

		Q_SIGNALS:
			void databaseInfoRead( const DatabaseInfo & db );
	};
}

#endif // QLAM_UPDATEWIDGETDATABASEINFOHELPERTHREAD_H
