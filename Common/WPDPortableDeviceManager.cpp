#include "stdafx.h"
#include <memory>

#include "WPDPortableDeviceManager.h"
#include <stdexcept>

namespace WPD
{
// class PortableDeviceManager
	std::vector<std::wstring> PortableDeviceManager::GetDevices()
	{
		std::vector<std::wstring> devices;
		DWORD count = 0;

		if (SUCCEEDED(m_interface->GetDevices(nullptr, &count)))
		{
			std::unique_ptr<PWSTR[]> deviceIDs{ new PWSTR[count] };
			if (SUCCEEDED(m_interface->GetDevices(deviceIDs.get(), &count)))
			{
				for (DWORD index = 0; index < count; ++index)
				{
					devices.push_back(deviceIDs[index]);
					CoTaskMemFree(deviceIDs[index]);
				}
			}
		}
		return devices;
	}

	std::wstring PortableDeviceManager::getDeviceFriendlyName(const std::wstring& deviceId)
	{
		std::wstring str;
		DWORD size = 0;

		if (SUCCEEDED(m_interface->GetDeviceFriendlyName(deviceId.c_str(), nullptr, &size))
			&& (size > 0))
		{
			std::unique_ptr<WCHAR[]> name{ new WCHAR[++size] };
			if (SUCCEEDED(m_interface->GetDeviceFriendlyName(deviceId.c_str(), name.get(), &size)))
				str = name.get();
		}
		return str;
	}
	std::wstring PortableDeviceManager::getDeviceManufacturer(const std::wstring& deviceId)
	{
		std::wstring str;
		DWORD size = 0;

		if (SUCCEEDED(m_interface->GetDeviceManufacturer(deviceId.c_str(), nullptr, &size))
			&& (size > 0))
		{
			std::unique_ptr<WCHAR[]> name{ new WCHAR[++size] };
			if (SUCCEEDED(m_interface->GetDeviceManufacturer(deviceId.c_str(), name.get(), &size)))
				str = name.get();
		}
		return str;
	}
	std::wstring PortableDeviceManager::getDeviceDescription(const std::wstring& deviceId)
	{
		std::wstring str;
		DWORD size = 0;

		if (SUCCEEDED(m_interface->GetDeviceDescription(deviceId.c_str(), nullptr, &size))
			&& (size > 0))
		{
			std::unique_ptr<WCHAR[]> name{ new WCHAR[++size] };
			if (SUCCEEDED(m_interface->GetDeviceDescription(deviceId.c_str(), name.get(), &size)))
				str = name.get();
		}
		return str;
	}
}