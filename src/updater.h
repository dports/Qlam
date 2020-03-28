#ifndef QLAM_UPDATER_H
#define QLAM_UPDATER_H

#include <QtCore/QThread>
#include <QtCore/QString>

class QObject;

namespace Qlam {
	class Updater
	: public QThread {

			Q_OBJECT

		public:
			explicit Updater( QObject * parent = nullptr );

		Q_SIGNALS:
			void upToDate( void );
			void updatesFound( void );
			void checkFailed( QString err );
			void updatingMainDatabase( int version );
			void updatingDailyDatabase( int version );
			void updatingBytecodeDatabase( int version );
			void updatingOtherDatabase( QString name, int version );
			void updateProgress( int pc );
			void updateFailed( QString msg );
			void updateSucceeded( void );
			void updateComplete( void );
			void aborted( void );
			void abortRequested( void );

		public Q_SLOTS:
			void abort( void ) {
				m_abort = true;
				Q_EMIT(abortRequested());
			}

		protected:
			virtual void run( void );

		private Q_SLOTS:
			void emitUpdateProgress( qint64, qint64 );

		private:
			bool m_abort;
	};
}

#endif // QLAM_UPDATER_H
