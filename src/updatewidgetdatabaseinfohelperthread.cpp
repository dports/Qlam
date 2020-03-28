#include "updatewidgetdatabaseinfohelperthread.h"

#include <QtCore/QDebug>

#include "application.h"
#include "databaseinfo.h"

using namespace Qlam;

UpdateWidgetDatabaseInfoHelperThread::UpdateWidgetDatabaseInfoHelperThread( QObject *parent )
	: QThread(parent) {
}

#if defined(QT_DEBUG)
UpdateWidgetDatabaseInfoHelperThread::~UpdateWidgetDatabaseInfoHelperThread() {
	qDebug() << "deleting thread" << ((void *) this);
}
#endif

void UpdateWidgetDatabaseInfoHelperThread::run() {
	QString path = qlamApp->settings()->databasePath();

	if(path.isEmpty()) {
		path = qlamApp->systemDatabasePath();
	}

	if(!path.isEmpty()) {
		for(const auto & db : qlamApp->databases()) {
			Q_EMIT(databaseInfoRead(db));
		}
	}
}
