#include <QtGlobal>

#include <QtCore/QDebug>
#include <QtCore/QSettings>

#include "application.h"
#include "mainwindow.h"

using namespace Qlam;

int main( int argc, char * argv[] ) {
	QSettings::setDefaultFormat(QSettings::IniFormat);
	Qlam::Application app(argc, argv);
    Qlam::MainWindow w;
    w.show();

	for(int i = 1; i < argc; ++i) {
		QString arg = QString::fromUtf8(argv[i]);

		if("--profile" == arg) {
			++i;

			if(i >= argc) {
				qDebug() << "--profile requires a profile name";
			}
			else if(!w.startScanByProfileName(QString::fromUtf8(argv[i]))) {
				qDebug() << "profile could not be found or scan could not be started";
			}

			break;
		}
		else if("--paths") {
			QStringList paths;

			for(int j = i + 1; j < argc; ++j) {
				paths.append(QString::fromUtf8(argv[j]));
			}

			if(!w.startCustomScan(paths)) {
				qDebug() << "scan could not be started";
			}

			break;
		}
	}

	return app.exec();
}
