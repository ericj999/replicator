#pragma once
#include "ComInterface.h"
#include "StringT.h"
#include "WPDPortableDeviceProperties.h"

namespace WPD
{
	// IPortableDeviceContent
	class PortableDeviceContent : public ComInterface<IPortableDeviceContent>
	{
	public:
		PortableDeviceContent() :
			ComInterface() {}

		~PortableDeviceContent() {}

		HRESULT TransferFile(const PathT& srcPath, const std::wstring& folderObjId);
		HRESULT UpdateFile(const PathT& srcPath, const std::wstring& folderObjId, const std::wstring& destObjId);

		HRESULT CreateFolder(const std::wstring& parentObjId, const std::wstring& folderName, std::wstring& newFolderId);

	protected:
		HRESULT CreateTransferPropertiesFromFile(const PathT& source, ComInterface<IStream>& srcStream, const std::wstring& parentObjId, PortableDeviceValues& properties, bool update = false);
	};
}
