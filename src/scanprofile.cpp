#include "scanprofile.h"

using namespace Qlam;

ScanProfile::ScanProfile( const QString & name )
: m_name(),
  m_paths() {
	setName(name);
}

void ScanProfile::setPath( int i, const QString & path ) {
	if(i < 0 || i >= m_paths.count()) {
		return;
	}

	m_paths[i] = path;
}
