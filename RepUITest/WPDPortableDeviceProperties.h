#pragma once

#include "ComInterface.h"

namespace WPD
{
	// IPortableDeviceProperties
	class PortableDeviceProperties : public ComInterface<IPortableDeviceProperties>
	{
	public:
		PortableDeviceProperties() :
			ComInterface() {}

		~PortableDeviceProperties() {}
	};

	class PortableDeviceKeyCollection : public ComInterface<IPortableDeviceKeyCollection>
	{
	public:
		PortableDeviceKeyCollection() :
			ComInterface(CLSID_PortableDeviceKeyCollection) {}

		~PortableDeviceKeyCollection() {}
	};

	// IPortableDeviceValues
	class PortableDeviceValues : public ComInterface<IPortableDeviceValues>
	{
	public:
		PortableDeviceValues() :
			ComInterface(CLSID_PortableDeviceValues) {}

		~PortableDeviceValues() {}
	};

	class PropVariant
	{
	public:
		PropVariant()
		{
			PropVariantInit(&m_prop);
		}

		~PropVariant()
		{
			PropVariantClear(&m_prop);
		}

		PROPVARIANT* Get() { return &m_prop; }

	protected:
		PROPVARIANT m_prop;
	};

}
