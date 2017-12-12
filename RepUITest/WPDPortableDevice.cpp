#include "stdafx.h"
#include <stdexcept>

#include "WPDPortableDevice.h"
#include "WPDPortableDeviceProperties.h"

namespace WPD
{
#define CLIENT_NAME         L"File Replicator"
#define CLIENT_MAJOR_VER    1
#define CLIENT_MINOR_VER    1
#define CLIENT_REVISION     0

	PortableDevice::PortableDevice(const std::wstring& deviceId) :
		ComInterface{ CLSID_PortableDeviceFTM }
	{
		PortableDeviceValues clientInfo;

		clientInfo->SetUnsignedIntegerValue(WPD_CLIENT_MAJOR_VERSION, CLIENT_MAJOR_VER);
		clientInfo->SetUnsignedIntegerValue(WPD_CLIENT_MINOR_VERSION, CLIENT_MINOR_VER);
		clientInfo->SetUnsignedIntegerValue(WPD_CLIENT_REVISION, CLIENT_REVISION);
		clientInfo->SetUnsignedIntegerValue(WPD_CLIENT_SECURITY_QUALITY_OF_SERVICE, SECURITY_IMPERSONATION);
		// we always open the device in read only mode
		clientInfo->SetUnsignedIntegerValue(WPD_CLIENT_DESIRED_ACCESS, GENERIC_READ);
		
		if (FAILED(m_interface->Open(deviceId.c_str(), clientInfo.Get())))
		{
			throw std::runtime_error("Failed to open the portable device.");
		}
	}
}