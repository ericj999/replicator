#pragma once
#include "ComInterface.h"

namespace WPD
{
	HRESULT StreamCopy(ComInterface<IStream>& sourceStream, ComInterface<IPortableDeviceDataStream>& destStream, DWORD transferSizeBytes, DWORD& bytesWrittenOut);
}