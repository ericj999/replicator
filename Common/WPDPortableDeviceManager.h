#pragma once
#include <vector>

#include "ComInterface.h"

namespace WPD
{
	// IPortableDeviceManager
	class PortableDeviceManager : public ComInterface<IPortableDeviceManager>
	{
	public:
		PortableDeviceManager() :
			ComInterface( CLSID_PortableDeviceManager ) {}

		~PortableDeviceManager() {}

		std::vector<std::wstring> GetDevices();

		std::wstring getDeviceFriendlyName(const std::wstring& deviceId);
		std::wstring getDeviceManufacturer(const std::wstring& deviceId);
		std::wstring getDeviceDescription(const std::wstring& deviceId);

	};
}
