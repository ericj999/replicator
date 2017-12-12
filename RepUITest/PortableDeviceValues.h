#pragma once
#include <vector>

#include "ComInterface.h"

namespace WPD
{
	// IPortableDeviceValues
	class PortableDeviceValues : public ComInterface<IPortableDeviceValues>
	{
	public:
		PortableDeviceValues() :
			ComInterface(CLSID_PortableDeviceValues) {}

		~PortableDeviceValues() {}
	};
}
