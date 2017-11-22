#pragma once
#include <vector>
#include "StringT.h"

namespace Database
{
	enum PropertyType
	{
		PT_INT64,
		PT_INT,
		PT_TEXT,
		PT_NULL
	};

	class Property
	{
//		friend class Recordset;
	public:
		// construct property with value
		Property();
		Property(const StringT& name, __int64 i64, int index = -1);
		Property(const StringT& name, int i, int index = -1);
		Property(const StringT& name, const StringT& str, int index = -1);
		Property(const StringT& name, LPCTSTR str, int index = -1);
		// construct a property holder
		Property(const StringT& name, PropertyType pt);
		// copy c'tor
		Property(const Property& p);
		// d'tor
		~Property() {}

		const StringT& ToString();

		int m_index;
		StringT m_name;
		PropertyType m_type;
		union
		{
			__int64 m_i64;
			int m_i;
		};
		StringT m_str;

		// methods used by Recordset
		void setNULL(bool null = true) { m_isNULL = null;  }
		bool IsNULL() const { return m_isNULL;  }

	protected:
		bool m_isNULL;
	};

//	using PropertyList = std::vector<Property>;
	class PropertyList : public std::vector < Property >
	{
	public:
		PropertyList() {}
		~PropertyList() {}

		const Property& Find(const StringT& name)
		{
			for (auto p = begin(); p != end(); ++p)
			{
				if (p->m_name == name)
					return *p;
			}
			return m_nullProp;
		}

	private:
		Property m_nullProp;
	};
}
