#include "stdafx.h"
#include <string>

#include "ShellFolder.h"
#include "ComMemObj.h"

HRESULT EnumShellFolder(const std::wstring& path)
{
	HRESULT hr = E_FAIL;

	ShellWrapper::ShellFolder desktopFolder;

	if (SUCCEEDED(hr = SHGetDesktopFolder(&desktopFolder)))
	{
		ComMemObj<ITEMIDLIST> itemId;

		if (SUCCEEDED(hr = desktopFolder->ParseDisplayName(NULL, nullptr, const_cast<LPWSTR>(path.c_str()), nullptr, &itemId, nullptr)))
		{
			ShellWrapper::ShellFolder folder;
			hr = desktopFolder->BindToObject(itemId.Get(), nullptr, IID_IShellFolder, reinterpret_cast<void**>(&folder));
			if (SUCCEEDED(hr))
			{
				ShellWrapper::EnumIDList enumIDList;
				if (SUCCEEDED(hr = folder->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &enumIDList)))
				{
					HRESULT getResult = E_FAIL;
					do
					{
						ULONG get = 10, got = 0;
						getResult = enumIDList->Next(get, &itemId, &got);
						for (int i = 0; i < static_cast<int>(got); ++i)
						{
							STRRET strRet;
							memset(&strRet, 0, sizeof(STRRET));
							strRet.uType = STRRET_CSTR;

							folder->GetDisplayNameOf(itemId.GetAt(i), SHGDN_NORMAL, &strRet);

							SFGAOF attributes = SFGAO_CANCOPY | SFGAO_FOLDER | SFGAO_STREAM;
							if (SUCCEEDED(folder->GetAttributesOf(1, (PCUITEMID_CHILD_ARRAY)itemId.GetAt(i), &attributes)))
							{

							}
						}
					} while (getResult == S_OK);
				}
			}
		}
	}
	return hr;
}