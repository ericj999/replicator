#include "stdafx.h"
#include "RepSource.h"


RepSource::RepSource(const PathT& path) :
	m_parent{ path }
{
	m_parent.remove_filename();
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
		throw std::runtime_error("mismatch folder paths!");
	}

}
