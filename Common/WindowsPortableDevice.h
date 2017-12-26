#pragma once

#include "ComInterface.h"
#include <PortableDeviceApi.h>      // Include this header for Windows Portable Device API interfaces
#include <PortableDevice.h>         // Include this header for Windows Portable Device definitions

class WPDDevice : public ComInterface<IPortableDevice>
{
public:
	WPDDevice() {}
	~WPDDevice() {}
};
