#pragma once

#include <Objbase.h>
#include <string>
#include <stdexcept>
#include <vector>

template<typename T>
class ComMemObj
{
public:
	ComMemObj() : m_memObj{nullptr} {}

	ComMemObj(T* ptr)
	{
		if (m_memObj) CoTaskMemFree(m_memObj); m_memObj = nullptr;
		m_memObj = ptr;
	}

	~ComMemObj()
	{
		if (m_memObj)
			CoTaskMemFree(m_memObj);
	}

	T* Get() const { return m_memObj; }

	T** operator &() { if (m_memObj) CoTaskMemFree(m_memObj); m_memObj = nullptr; return &m_memObj; }
protected:
	T* m_memObj;
};


template<typename T, size_t N = 10>
class ComMemArray
{
public:
	ComMemArray()
	{
		memset(m_mem, 0, sizeof(T) * N);
	}
	~ComMemArray()
	{
		for (size_t i = 0; i < N; ++i)
			if (m_mem[i]) CoTaskMemFree(m_mem[i]);
	}

	T const operator[](size_t index) { return m_mem[index]; }
	T* data() { return m_mem; }
protected:
	size_t m_size;
	T m_mem[N];
};

class ShellItemIDList
{
public:
	ShellItemIDList() {}
	~ShellItemIDList()
	{
		for (auto item : m_list)
			if (item) CoTaskMemFree(item);
	}

	using iterator = std::vector<LPITEMIDLIST>::iterator;
	using const_iterator = std::vector<LPITEMIDLIST>::const_iterator;

	iterator begin() { return m_list.begin();  }
	const_iterator begin() const { return m_list.begin(); }
	iterator end() { return m_list.end();  }
	const_iterator end() const { return m_list.end(); }

	bool empty() const { return m_list.empty(); }
	size_t size() { return m_list.size(); }
	void push_back(const LPITEMIDLIST& itemIdList) { m_list.push_back(itemIdList); }

protected:
	std::vector<LPITEMIDLIST> m_list;

};
