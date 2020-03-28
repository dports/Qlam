#ifndef QLAM_SCANREPORT_H
#define QLAM_SCANREPORT_H

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QList>
#include <QtCore/QDateTime>
#include <QtCore/QDate>
#include <QtCore/QTime>

#include "infectedfile.h"

namespace Qlam {

	class ScanReport 	{
		public:
			enum Outcome {
				Unknown = 0,	/* outcome of scan is not known */
				Failed,			/* scan failed to complete */
				Clean,			/* scan completed and did not find any issues */
				Infected			/* scan completed and found one or more issues */
			};

			ScanReport( void );

			inline Outcome outcome( void ) const {
				return m_outcome;
			}

			inline void setOutcome( const Outcome & outcome ) {
				m_outcome = outcome;
			}

			inline QDateTime startDateTime( void ) const {
				return m_startTime;
			}

			inline void setStartDateTime( const QDateTime & d ) {
				m_startTime = d;
			}

			inline QDate startDate( void ) const {
				return m_startTime.date();
			}

			inline void setStartDate( const QDate & d ) {
				m_startTime.setDate(d);
			}

			inline QTime startTime( void ) const {
				return m_startTime.time();
			}

			inline void setStartTime( const QTime & t ) {
				m_startTime.setTime(t);
			}

			inline QDateTime endDateTime( void ) const {
				return m_endTime;
			}

			inline void setEndDateTime( const QDateTime & d ) {
				m_endTime = d;
			}

			inline QDate endDate( void ) const {
				return m_endTime.date();
			}

			inline void setEndDate( const QDate & d ) {
				m_endTime.setDate(d);
			}

			inline QTime endTime( void ) const {
				return m_endTime.time();
			}

			inline void setEndTime( const QTime & t ) {
				m_endTime.setTime(t);
			}

			inline QString title( void ) const {
				return m_title;
			}

			inline void setTitle( const QString & title ) {
				m_title = title;
			}

            inline const QStringList & scannedPaths( void ) {
                return m_scannedPaths;
            }

            inline QStringList scannedPaths( void ) const {
				return m_scannedPaths;
			}

			inline void addScannedPath( const QString & path ) {
				m_scannedPaths.append(path);
			}

			inline void removeScannedPath( const QString & path ) {
				m_scannedPaths.removeAll(path);
			}

			inline void removeScannedPath( int i ) {
                Q_ASSERT(0 <= i && i < m_scannedPaths.count());
				m_scannedPaths.removeAt(i);
			}

			inline void clearScannedPaths( void ) {
				m_scannedPaths.clear();
			}

			inline int scannedPathCount( void ) const {
				return m_scannedPaths.count();
			}

            inline const QList<InfectedFile> & infectedFiles( void ) {
				return m_infections;
			}

			inline QList<InfectedFile> infectedFiles( void ) const {
				return m_infections;
			}

			inline void addInfectedFile( const QString & path ) {
				m_infections.append(InfectedFile(path));
			}

			inline void addInfectedFile( const InfectedFile & infection ) {
				m_infections.append(infection);
			}

			void removeInfectedFile( const QString & path );
			void removeInfectedFile( const InfectedFile & infection );

			inline void removeInfectedFile( int i ) {
                Q_ASSERT(0 <= i && i < m_infections.count());
				m_infections.removeAt(i);
			}

			inline void clearInfectedFiles( void ) {
				m_infections.clear();
			}

            inline int infectedFileCount( void ) const {
				return m_infections.count();
			}

		private:
			Outcome m_outcome;
			QDateTime m_startTime, m_endTime;
			QString m_title;
			QStringList m_scannedPaths;	/* not everything, just those passed to the scanner */
			QList<InfectedFile> m_infections;
	};

} // namespace Qlam

#endif // QLAM_SCANREPORT_H
