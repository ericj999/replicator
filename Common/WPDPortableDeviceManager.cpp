#include "stdafx.h"
#include <memory>

#include "WPDPortableDeviceManager.h"
#include <stdexcept>
#include "Log.h"

namespace WPD
{
// class PortableDeviceManager
	std::vector<std::wstring> PortableDeviceManager::GetDevices()
	{
		std::vector<std::wstring> devices;
		DWORD count = 0;

		HRESULT hr = E_FAIL;
		if (SUCCEEDED(hr = m_interface->GetDevices(nullptr, &count)))
		{
			std::unique_ptr<PWSTR[]> deviceIDs{ new PWSTR[count] };
			if (SUCCEEDED(hr = m_interface->GetDevices(deviceIDs.get(), &count)))
			{
				for (DWORD index = 0; index < count; ++index)
				{
					devices.push_back(deviceIDs[index]);
					CoTaskMemFree(deviceIDs[index]);
				}
			}
			else
			{
				Log::logger.error(StringT(L"Failed to get portable devices. code:") + ToStringT(hr));
			}
		}
		else
		{
			Log::logger.error(StringT(L"Failed to get portable device count. code:") + ToStringT(hr));
		}
		return devices;
	}

	std::wstring PortableDeviceManager::getDeviceFriendlyName(const std::wstring& deviceId)
	{
		std::wstring str;
		DWORD size = 0;

		HRESULT hr = E_FAIL;
		if (SUCCEEDED(hr = m_interface->GetDeviceFriendlyName(deviceId.c_str(), nullptr, &size))
			&& (size > 0))
		{
			std::unique_ptr<WCHAR[]> name{ new WCHAR[++size] };
			if (SUCCEEDED(hr = m_interface->GetDeviceFriendlyName(deviceId.c_str(), name.get(), &size)))
				str = name.get();
			else
			{
				Log::logger.error(StringT(L"Failed to get device name. code:") + ToStringT(hr));
			}
		}
		else
		{
			Log::logger.error(StringT(L"Failed to device name size. code:") + ToStringT(hr));
		}
		return str;
	}
	std::wstring PortableDeviceManager::getDeviceManufacturer(const std::wstring& deviceId)
	{
		std::wstring str;
		DWORD size = 0;

		HRESULT hr = E_FAIL;
		if (SUCCEEDED(hr = m_interface->GetDeviceManufacturer(deviceId.c_str(), nullptr, &size))
			&& (size > 0))
		{
			std::unique_ptr<WCHAR[]> name{ new WCHAR[++size] };
			if (SUCCEEDED(hr = m_interface->GetDeviceManufacturer(deviceId.c_str(), name.get(), &size)))
				str = name.get();
			else
			{
				Log::logger.error(StringT(L"Failed to get manufacturer. code:") + ToStringT(hr));
			}
		}
		else
		{
			Log::logger.error(StringT(L"Failed to get size of manufacturer name. code:") + ToStringT(hr));
		}
		return str;
	}
	std::wstring PortableDeviceManager::getDeviceDescription(const std::wstring& deviceId)
	{
		std::wstring str;
		DWORD size = 0;

		HRESULT hr = E_FAIL;
		if (SUCCEEDED(hr = m_interface->GetDeviceDescription(deviceId.c_str(), nullptr, &size))
			&& (size > 0))
		{
			std::unique_ptr<WCHAR[]> name{ new WCHAR[++size] };
			if (SUCCEEDED(hr = m_interface->GetDeviceDescription(deviceId.c_str(), name.get(), &size)))
				str = name.get();
			else
			{
				Log::logger.error(StringT(L"Failed to get description. code:") + ToStringT(hr));
			}
		}
		else
		{
			Log::logger.error(StringT(L"Failed to get description size. code:") + ToStringT(hr));
		}
		return str;
	}
}