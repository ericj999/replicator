#pragma once

#include <StringT.h>
#include <vector>

// The selected soruce paths
class RepSource
{
public:
	RepSource(const PathT& path);
	~RepSource();

	void add(const PathT& path);
	const PathT& getParent() const { return m_parent; }
	const std::vector<PathT>& getChildren() const { return m_children;  }

private:
	PathT m_parent;					// the parent folder of selected folder
	std::vector<PathT> m_children;	// child branches starting from the selected folder
};

