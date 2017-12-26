#pragma once

#include "StringT.h"
#include "ShellFolder.h"

#define MD5LEN	16

namespace Util
{
	class MD5Hash
	{
	public:
		MD5Hash(const PathT& file);
		MD5Hash(ShellWrapper::ShellItem2& shellItem);
		~MD5Hash();

		bool operator==(const MD5Hash& other)
		{
			return (memcmp(m_data, other.m_data, sizeof(other.m_data)) == 0) ? true : false;
		}

	private:
		BYTE m_data[MD5LEN];

		bool Calculate(const PathT& file);
		bool Calculate(ShellWrapper::ShellItem2& shellItem);
	};
}
