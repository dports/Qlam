#ifndef QLAM_APPLICATION_H
#define QLAM_APPLICATION_H

#include <QtGlobal>

#if QT_VERSION >= 0x050000
#include <QtWidgets/QApplication>
#else
#include <QtGui/QApplication>
#endif

#include <QtCore/QList>

struct cl_engine;

#include "settings.h"
#include "scanprofile.h"
#include "databaseinfo.h"

#define qlamApp (Qlam::Application::instance())

namespace Qlam {

	class Application
	: public QApplication {

			Q_OBJECT

		public:
			explicit Application(int &, char **);
			~Application() override;

			inline static Application * instance() {
				return s_instance;
			}

			static QString clamAvVersion();
			bool clamAvInitialised() const;
			static QString systemDatabasePath();
			QList<DatabaseInfo> databases();

			/* discard the cache of database information so that the next call
			 * to databases() rescans the databases files */
//			void discardDatabaseInfoCache();

			inline void addScanProfile(const ScanProfile & profile) {
				addScanProfile(new ScanProfile(profile));
			}

			void addScanProfile(ScanProfile * profile);

			QList<ScanProfile *> scanProfiles() const {
				return m_scanProfiles;
			}

			ScanProfile scanProfile(int) const;
			int exec();

			struct cl_engine * acquireEngine();
			void releaseEngine();

			Settings * settings() {
				return m_settings;
			}

		Q_SIGNALS:
			void scanProfileAdded(const QString &);
			void scanProfileAdded(int);

		public Q_SLOTS:

		protected:
			void timerEvent(QTimerEvent *) override;

		private Q_SLOTS:
			void readScanProfiles();
			void writeScanProfiles();

		private:
			void disposeEngine();

			static Application * s_instance;
			QList<ScanProfile *> m_scanProfiles;
			int m_clamavInit;

			struct cl_engine * m_scanEngine;
			int m_engineLockCount;
			int m_engineDisposeTimer;

			Settings * m_settings;
	};
}

#endif // QLAM_APPLICATION_H
