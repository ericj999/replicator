#include "stdafx.h"
#include "RepSource.h"
#include "LocaleResources.h"

RepSource::RepSource(const PathT& path) :
	m_parent{ path }
{
}


RepSource::~RepSource()
{
}

void RepSource::add(const PathT& path)
{
	if (path.wstring().compare(0, m_parent.wstring().length(), m_parent.wstring()) == 0)
	{
		m_children.push_back(PathT(path.wstring().substr(m_parent.wstring().length() + 1)));
	}
	else
	{
		throw std::runtime_error( EXCEPSTR_MISMATCHED_FOLDER_PATHS );
	}

}
