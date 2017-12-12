#pragma once
#include "ComInterface.h"

namespace WPD
{
	// IPortableDeviceContent
	class PortableDeviceContent : public ComInterface<IPortableDeviceContent>
	{
	public:
		PortableDeviceContent() :
			ComInterface() {}

		~PortableDeviceContent() {}
	};
}
