#pragma once
#include <map>
#include "ComInterface.h"

namespace WPD
{
// PortableDeviceItem
	class PortableDeviceItem
	{
	public:
		PortableDeviceItem() :
			m_contentType{ GUID_NULL }, m_modifiedTime{ 0 }
		{}
		~PortableDeviceItem() {}

		const std::wstring& GetObjId() const { return m_objId; }
		const std::wstring& GetName() const { return m_name; }
		const GUID& GetContentType() const { return m_contentType; }
		const FILETIME* GetModifiedTime() const { return &m_modifiedTime; }

	protected:
		friend class PortableDeviceProperties;

		std::wstring m_objId;
		std::wstring m_name;
		GUID m_contentType;
		FILETIME m_modifiedTime;
	};

	using PortableDeviceItemMap = std::map<std::wstring, PortableDeviceItem>;

// IPortableDeviceProperties
	class PortableDeviceProperties : public ComInterface<IPortableDeviceProperties>
	{
	public:
		PortableDeviceProperties() :
			ComInterface() {}

		~PortableDeviceProperties() {}

		HRESULT GetItemWithProperties(const std::wstring& objId, PortableDeviceItem& item);
	};

	class PortableDeviceKeyCollection : public ComInterface<IPortableDeviceKeyCollection>
	{
	public:
		PortableDeviceKeyCollection() :
			ComInterface(CLSID_PortableDeviceKeyCollection) {}

		PortableDeviceKeyCollection::PortableDeviceKeyCollection(const PROPERTYKEY keys[], size_t count);

		~PortableDeviceKeyCollection() {}
	};

// IPortableDeviceValues
	class PortableDeviceValues : public ComInterface<IPortableDeviceValues>
	{
	public:
		PortableDeviceValues() :
			ComInterface(CLSID_PortableDeviceValues) {}

		~PortableDeviceValues() {}

		std::wstring GetStringValue(const PROPERTYKEY& key);
		GUID GetGuidValue(const PROPERTYKEY& key);
		FILETIME GetFileTimeValue(const PROPERTYKEY& key);
	};
}
