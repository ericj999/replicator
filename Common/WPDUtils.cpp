#include "stdafx.h"
#include "WPDPortableDeviceProperties.h"
#include "WPDUtils.h"
#include "Log.h"

namespace WPD
{
	HRESULT StreamCopy(ComInterface<IStream>& sourceStream, ComInterface<IPortableDeviceDataStream>& destStream, DWORD transferSizeBytes, DWORD& bytesWrittenOut)
	{
		bytesWrittenOut = 0;
		HRESULT hr = S_OK;

		// Allocate a temporary buffer (of Optimal transfer size) for the read results to
		// be written to.
		BYTE* objectData = new (std::nothrow) BYTE[transferSizeBytes];
		if (objectData != nullptr)
		{
			DWORD totalBytesRead = 0;
			DWORD totalBytesWritten = 0;

			DWORD bytesRead = 0;
			DWORD bytesWritten = 0;

			// Read until the number of bytes returned from the source stream is 0, or
			// an error occured during transfer.
			do
			{
				// Read object data from the source stream
				hr = sourceStream->Read(objectData, transferSizeBytes, &bytesRead);
				if (SUCCEEDED(hr))
				{
					totalBytesRead += bytesRead; // Calculating total bytes read from device for debugging purposes only

					if (SUCCEEDED(hr = destStream->Write(objectData, bytesRead, &bytesWritten)))
					{
						totalBytesWritten += bytesWritten;
					}
					else
					{
						Log::logger.error(StringT(_T("Failed to write to stream.")) + ToStringT(hr));
					}
				}
				else
				{
					Log::logger.error(StringT(_T("Failed to read from stream.")) + ToStringT(hr));
				}
			} while (SUCCEEDED(hr) && (bytesRead > 0));
			// Remember to delete the temporary transfer buffer
			delete[] objectData;
		}
		return hr;
	}
}