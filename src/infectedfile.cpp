#include "infectedfile.h"

using namespace Qlam;

FileWithIssues::FileWithIssues( const QString & path )
: m_path() {
	setPath(path);
}
