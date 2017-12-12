#pragma once
#include <vector>

#include <PortableDeviceApi.h>
#include <PortableDevice.h>

#include "ComInterface.h"

namespace WPD
{
	// PortableDevice
	class PortableDevice : public ComInterface<IPortableDevice>
	{
	public:
		PortableDevice(const std::wstring& deviceId);
		~PortableDevice() {}
	};
}

