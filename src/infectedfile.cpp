#include "infectedfile.h"

using namespace Qlam;

InfectedFile::InfectedFile( const QString & path )
: m_path() {
	setPath(path);
}
