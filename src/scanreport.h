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
			enum class Outcome {
				Unknown = 0,	/* outcome of scan is not known */
				Failed,			/* scan failed to complete */
				Clean,			/* scan completed and did not find any issues */
				Infected			/* scan completed and found one or more issues */
			};

			ScanReport();

			inline Outcome outcome() const {
				return m_outcome;
			}

			inline void setOutcome( const Outcome & outcome ) {
				m_outcome = outcome;
			}

			inline QDateTime startDateTime() const {
				return m_startTime;
			}

			inline void setStartDateTime( const QDateTime & d ) {
				m_startTime = d;
			}

			inline QDate startDate() const {
				return m_startTime.date();
			}

			inline void setStartDate( const QDate & d ) {
				m_startTime.setDate(d);
			}

			inline QTime startTime() const {
				return m_startTime.time();
			}

			inline void setStartTime( const QTime & t ) {
				m_startTime.setTime(t);
			}

			inline QDateTime endDateTime() const {
				return m_endTime;
			}

			inline void setEndDateTime( const QDateTime & d ) {
				m_endTime = d;
			}

			inline QDate endDate() const {
				return m_endTime.date();
			}

			inline void setEndDate( const QDate & d ) {
				m_endTime.setDate(d);
			}

			inline QTime endTime() const {
				return m_endTime.time();
			}

			inline void setEndTime( const QTime & t ) {
				m_endTime.setTime(t);
			}

			inline QString title() const {
				return m_title;
			}

			inline void setTitle( const QString & title ) {
				m_title = title;
			}

            inline const QStringList & scannedPaths() {
                return m_scannedPaths;
            }

            inline QStringList scannedPaths() const {
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

			inline void clearScannedPaths() {
				m_scannedPaths.clear();
			}

			inline int scannedPathCount() const {
				return m_scannedPaths.count();
			}

            inline const QList<FileWithIssues> & infectedFiles() {
				return m_infections;
			}

			inline QList<FileWithIssues> infectedFiles() const {
				return m_infections;
			}

			inline void addInfectedFile( const QString & path ) {
				m_infections.append(FileWithIssues(path));
			}

			inline void addInfectedFile( const FileWithIssues & infection ) {
				m_infections.append(infection);
			}

			void removeInfectedFile( const QString & path );
			void removeInfectedFile( const FileWithIssues & infection );

			inline void removeInfectedFile( int i ) {
                Q_ASSERT(0 <= i && i < m_infections.count());
				m_infections.removeAt(i);
			}

			inline void clearInfectedFiles() {
				m_infections.clear();
			}

            inline int infectedFileCount() const {
				return m_infections.count();
			}

		private:
			Outcome m_outcome;
			QDateTime m_startTime, m_endTime;
			QString m_title;
			QStringList m_scannedPaths;	/* not everything, just those passed to the scanner */
			QList<FileWithIssues> m_infections;
	};

} // namespace Qlam

#endif // QLAM_SCANREPORT_H
