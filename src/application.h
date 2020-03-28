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
			explicit Application( int & argc, char ** argv );
			virtual ~Application( void );

			inline static Application * instance( void ) {
				return s_instance;
			}

			static QString clamAvVersion( void );
			bool clamAvInitialised( void ) const;
			QString systemDatabasePath( void );
			QList<DatabaseInfo> databases( void );

			/* discard the cache of database information so that the next call
			 * to databases() rescans the databases files */
//			void discardDatabaseInfoCache( void );

			inline void addScanProfile( const ScanProfile & profile ) {
				addScanProfile(new ScanProfile(profile));
			}

			void addScanProfile( ScanProfile * profile );

			QList<ScanProfile *> scanProfiles( void ) const {
				return m_scanProfiles;
			}

			ScanProfile scanProfile( int i ) const;
			int exec( void );

			struct cl_engine * acquireEngine( void );
			void releaseEngine( void );

			Settings * settings( void ) {
				return m_settings;
			}

		Q_SIGNALS:
			void scanProfileAdded( const QString & name );
			void scanProfileAdded( int idx );

		public Q_SLOTS:

		protected:
			virtual void timerEvent( QTimerEvent * ev );

		private Q_SLOTS:
			void readScanProfiles( void );
			void writeScanProfiles( void );

		private:
			void disposeEngine( void );

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
