#include "stdafx.h"
#include "ComInterface.h"
#include "WPDPortableDeviceContent.h"
#include "WPDPortableDeviceProperties.h"
#include "WPDDefinitions.h"
#include "WPDUtils.h"
#include "Log.h"

namespace WPD
{
	HRESULT PortableDeviceContent::TransferFile(const PathT& srcPath, const std::wstring& folderObjId)
	{
		ComInterface<IStream> srcStream;

		HRESULT hr = SHCreateStreamOnFileEx(srcPath.wstring().c_str(), STGM_READ, FILE_ATTRIBUTE_NORMAL, FALSE, nullptr, &srcStream);
		if (SUCCEEDED(hr))
		{
			PortableDeviceValues properties;
			DWORD optimalTransferSizeBytes = (8 * 1024);
			ComInterface<IStream> tempStream;
			ComInterface<IPortableDeviceDataStream> finalObjectDataStream;

			hr = CreateTransferPropertiesFromFile(srcPath, srcStream, folderObjId, properties);  // Returned properties describing the data
			if (SUCCEEDED(hr))
			{
				hr = m_interface->CreateObjectWithPropertiesAndData(properties.Get(), &tempStream, &optimalTransferSizeBytes, nullptr);
				if (SUCCEEDED(hr))
				{
					hr = tempStream->QueryInterface(IID_IPortableDeviceDataStream, (void**)&finalObjectDataStream);
					if (SUCCEEDED(hr))
					{
						DWORD totalBytesWritten = 0;

						hr = StreamCopy(srcStream, finalObjectDataStream, optimalTransferSizeBytes, totalBytesWritten);
						if (SUCCEEDED(hr))
						{
							hr = finalObjectDataStream->Commit(STGC_DEFAULT);
						}
					}
				}
			}
		}
		return hr;
	}

	HRESULT PortableDeviceContent::UpdateFile(const PathT& srcPath, const std::wstring& folderObjId, const std::wstring& destObjId)
	{
		ComInterface<IStream> srcStream;
		ComInterface<IPortableDeviceContent2> content2;

		HRESULT hr = m_interface->QueryInterface(IID_IPortableDeviceContent2, (void**)&content2);
		if (FAILED(hr))
			return hr;

		hr = SHCreateStreamOnFileEx(srcPath.wstring().c_str(), STGM_READ, FILE_ATTRIBUTE_NORMAL, FALSE, nullptr, &srcStream);
		if (SUCCEEDED(hr))
		{
			PortableDeviceValues properties;
			DWORD optimalTransferSizeBytes = (8 * 1024);
			ComInterface<IStream> tempStream;
			ComInterface<IPortableDeviceDataStream> finalObjectDataStream;

			hr = CreateTransferPropertiesFromFile(srcPath, srcStream, folderObjId, properties, true);  // Returned properties describing the data
			if (SUCCEEDED(hr))
			{
				hr = content2->UpdateObjectWithPropertiesAndData(destObjId.c_str(), properties.Get(), &tempStream, &optimalTransferSizeBytes);
				if (SUCCEEDED(hr))
				{
					hr = tempStream->QueryInterface(IID_IPortableDeviceDataStream, (void**)&finalObjectDataStream);
					if (SUCCEEDED(hr))
					{
						DWORD totalBytesWritten = 0;

						hr = StreamCopy(srcStream, finalObjectDataStream, optimalTransferSizeBytes, totalBytesWritten);
						if (SUCCEEDED(hr))
						{
							hr = finalObjectDataStream->Commit(STGC_DEFAULT);
						}
					}
				}
			}
		}
		return hr;
	}

	HRESULT PortableDeviceContent::CreateTransferPropertiesFromFile(const PathT& source, ComInterface<IStream>& srcStream,
		const std::wstring& parentObjId, PortableDeviceValues& properties, bool update /*= false*/)
	{
		HRESULT hr = E_FAIL;

		if (!update)
		{
			if (FAILED(hr = properties->SetStringValue(WPD_OBJECT_PARENT_ID, parentObjId.c_str())))
				return hr;
		}
		STATSTG statstg = { 0 };
		if (SUCCEEDED(srcStream->Stat(&statstg, STATFLAG_NONAME)))
		{
			hr = properties->SetUnsignedLargeIntegerValue(WPD_OBJECT_SIZE, statstg.cbSize.QuadPart);
		}

		if (FAILED(hr = properties->SetStringValue(WPD_OBJECT_ORIGINAL_FILE_NAME, source.filename().c_str())))
			return hr;

		if (FAILED(hr = properties->SetStringValue(WPD_OBJECT_NAME, source.stem().c_str())))
			return hr;

		ComPropVariant ftCreated{ &statstg.ctime }, ftModified{ &statstg.mtime };

		if (FAILED(hr = properties->SetValue(WPD_OBJECT_DATE_CREATED, ftCreated.Get())))
			return hr;

		if (FAILED(hr = properties->SetValue(WPD_OBJECT_DATE_MODIFIED, ftModified.Get())))
			return hr;
		
		// get the content type and format
		GUID contentType = GUID_NULL, format = GUID_NULL;

		GetFileContentType(source.extension(), contentType, format);

		if (FAILED(hr = properties->SetGuidValue(WPD_OBJECT_CONTENT_TYPE, contentType)))
			return hr;

		hr = properties->SetGuidValue(WPD_OBJECT_FORMAT, format);

		return hr;
	}

	HRESULT PortableDeviceContent::CreateFolder(const std::wstring& parentObjId, const std::wstring& folderName, std::wstring& newFolderId)
	{
		HRESULT hr = E_FAIL;
		PortableDeviceValues properties;
		/*
			These are the properties used by the WPD Shell Extension to create a folder :
			WPD_OBJECT_FORMAT = WPD_OBJECT_FORMAT_PROPERTIES_ONLY
			where : DEFINE_GUID(WPD_OBJECT_FORMAT_PROPERTIES_ONLY, 0x30010000, 0xAE6C, 0x4804, 0x98, 0xBA, 0xC5, 0x7B, 0x46, 0x96, 0x5F, 0xE7);
			WPD_OBJECT_CONTENT_TYPE = WPD_CONTENT_TYPE_FOLDER
			where: DEFINE_GUID(WPD_CONTENT_TYPE_FOLDER, 0x27E2E392, 0xA111, 0x48E0, 0xAB, 0x0C, 0xE1, 0x77, 0x05, 0xA0, 0x5F, 0x85);
			WPD_OBJECT_ORIGINAL_FILE_NAME = <name of the folder>
			WPD_OBJECT_NAME = <name of the folder>
			WPD_OBJECT_PARENT_ID = <parent object id>

			These are also set by the WPD Shell Extension but should be optional :
			WPD_OBJECT_IS_HIDDEN = true if hidden
			WPD_OBJECT_CAN_DELETE = false if read only
			WPD_OBJECT_DATE_CREATED = creation date
			WPD_OBJECT_DATE_MODIFIED = modified date
		*/
		if (FAILED(hr = properties->SetGuidValue(WPD_OBJECT_FORMAT, WPD_OBJECT_FORMAT_PROPERTIES_ONLY)))
		{
			Log::logger.error(StringT(L"Failed to set object format for new folder ") + folderName + StringT(L"(") + newFolderId + StringT(L"). Code:") + ToStringT(hr));
			return hr;
		}
		if (FAILED(hr = properties->SetStringValue(WPD_OBJECT_PARENT_ID, parentObjId.c_str())))
		{
			Log::logger.error(StringT(L"Failed to set object parent ID ") + parentObjId
				+ StringT(L" for new folder ") + folderName + StringT(L"(") + newFolderId + StringT(L"). Code:") + ToStringT(hr));
			return hr;
		}
		if (FAILED(hr = properties->SetStringValue(WPD_OBJECT_ORIGINAL_FILE_NAME, folderName.c_str())))
		{
			Log::logger.error(StringT(L"Failed to set original object name for new folder ") + folderName + StringT(L"(") + newFolderId + StringT(L"). Code:") + ToStringT(hr));
			return hr;
		}
		if (FAILED(hr = properties->SetStringValue(WPD_OBJECT_NAME, folderName.c_str())))
		{
			Log::logger.error(StringT(L"Failed to set object name for new folder ") + folderName + StringT(L"(") + newFolderId + StringT(L"). Code:") + ToStringT(hr));
			return hr;
		}
		if (FAILED(hr = properties->SetGuidValue(WPD_OBJECT_CONTENT_TYPE, WPD_CONTENT_TYPE_FOLDER)))
		{
			Log::logger.error(StringT(L"Failed to set content type for new folder ") + folderName + StringT(L"(") + newFolderId + StringT(L"). Code:") + ToStringT(hr));
			return hr;
		}
		PWSTR newObject = nullptr;
		if (SUCCEEDED(hr = m_interface->CreateObjectWithPropertiesOnly(properties.Get(), &newObject)))
			newFolderId = newObject;
		else
		{
			Log::logger.error(StringT(L"CreateObjectWithPropertiesOnly() failed. Code:") + ToStringT(hr));
		}

		if(newObject) CoTaskMemFree(newObject);
		return hr;
	}
}

