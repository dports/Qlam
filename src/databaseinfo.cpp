/** \file databaseinfo.cpp
 *
 * \todo
 * - implement read() for windows - don't rely on sigtool program
 */
#include "databaseinfo.h"

#include <QtCore/QDebug>
#include <QtCore/QProcess>
#include <QtCore/QDir>
#include <QtCore/QRegExp>
#include <QtCore/QDateTime>

using namespace Qlam;

DatabaseInfo::DatabaseInfo( const QString & path )
: m_path(path),
  m_buildTime(),
  m_version(),
  m_signatures(-1),
  m_builderName(),
  m_md5() {
	read();
}


bool DatabaseInfo::isValid() {
	return !m_path.isEmpty() && !m_buildTime.isEmpty() && !m_version.isEmpty() && (-1 < m_signatures) && !m_builderName.isEmpty();
}


void DatabaseInfo::parseLine( const QByteArray & line ) {
qDebug() << "parsing sigtool line" << line;
	static QRegExp s_rxSplit("^([^:]+): *([^ ].+)$");

	QString myLine = QString::fromUtf8(line);

	if(s_rxSplit.exactMatch(myLine)) {
		QString field = s_rxSplit.cap(1).toLower();
		QString value = s_rxSplit.cap(2);

		if("build time" == field) {
			m_buildTime = value;
		}
		else if("version" == field) {
			m_version = value;
		}
		else if("signatures" == field) {
			bool ok;
			int sigs = value.toInt(&ok);

			if(!ok) {
				qDebug() << "invalid signature count" << value;
				m_signatures = -1;
			}
			else {
				m_signatures = sigs;
			}
		}
		else if("builder" == field) {
			m_builderName = value;
		}
		else if("md5" == field) {
			m_md5 = value;
		}
	}
}


void DatabaseInfo::parseBuffer( QByteArray & buffer ) {
	for(int i = 0; i < buffer.length(); ++i) {
		if(10 == buffer.at(i)) {
			parseLine(buffer.mid(0, i));
			buffer = buffer.right(buffer.length() - i - 1);
			i = 0;
		}
	}
}


void DatabaseInfo::read() {
	m_buildTime = QString();
	m_version = QString();
	m_signatures = 0;
	m_builderName = QString();
	m_md5 = QString();

	QProcess p;
	QStringList args;
	args.append("-i");
	args.append(QDir::toNativeSeparators(m_path));
	p.setReadChannel(QProcess::StandardOutput);
	p.start("sigtool", args);
	p.waitForStarted();
	QByteArray buffer;
	buffer.reserve(1024);

	while(p.state() == QProcess::Running) {
		p.waitForReadyRead();
		buffer.append(p.readAll());
		parseBuffer(buffer);
	}
}
