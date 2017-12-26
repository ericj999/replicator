#include "stdafx.h"
#include "WPDPortableDeviceProperties.h"

namespace WPD
{
	PortableDeviceKeyCollection::PortableDeviceKeyCollection(const PROPERTYKEY keys[], size_t count) :
		ComInterface(CLSID_PortableDeviceKeyCollection)
	{
		for (size_t index = 0; index < count; ++index)
		{
			HRESULT hr = m_interface->Add(keys[index]);
		}
	}


// 
	const PROPERTYKEY itemPropKeys[]
	{
		WPD_OBJECT_CONTENT_TYPE,
		WPD_OBJECT_ORIGINAL_FILE_NAME,
		WPD_OBJECT_DATE_MODIFIED
	};

	constexpr size_t itemPropKeysSize = sizeof(itemPropKeys) / sizeof(PROPERTYKEY);

	HRESULT PortableDeviceProperties::GetItemWithProperties(const std::wstring& objId, PortableDeviceItem& item)
	{
		HRESULT hr = E_FAIL;
		PortableDeviceValues objValues;
		PortableDeviceKeyCollection propertiesToRead{ itemPropKeys, itemPropKeysSize };

		hr = m_interface->GetValues(objId.c_str(), propertiesToRead.Get(), &objValues);
		if(SUCCEEDED(hr))
		{
			item.m_objId = objId;
			item.m_contentType = objValues.GetGuidValue(WPD_OBJECT_CONTENT_TYPE);
			item.m_name = objValues.GetStringValue(WPD_OBJECT_ORIGINAL_FILE_NAME);
			item.m_modifiedTime = objValues.GetFileTimeValue(WPD_OBJECT_DATE_MODIFIED);
		}
		return hr;
	}

// PortableDeviceValues
	std::wstring PortableDeviceValues::GetStringValue(const PROPERTYKEY& key)
	{
		std::wstring str;
		if (m_interface)
		{
			PWSTR value = nullptr;
			HRESULT hr = m_interface->GetStringValue(key, &value);
			if (SUCCEEDED(hr))
				str = value;

			if(value) CoTaskMemFree(value);
		}
		return str;
	}

	GUID PortableDeviceValues::GetGuidValue(const PROPERTYKEY& key)
	{
		GUID guid = GUID_NULL;
		if (m_interface && SUCCEEDED(m_interface->GetGuidValue(key, &guid)))
			return guid;

		return GUID_NULL;
	}

	FILETIME PortableDeviceValues::GetFileTimeValue(const PROPERTYKEY& key)
	{
		FILETIME ft = { 0 };
		if (m_interface)
		{
			ComPropVariant value;
			HRESULT hr = m_interface->GetValue(key, value.Get());
			if (SUCCEEDED(hr))
			{
				if ((value.type() == VT_DATE) || (value.type() == VT_FILETIME))
				{
					ft = value.GetFileTimeValue();
				}
			}
		}
		return ft;
	}

}
