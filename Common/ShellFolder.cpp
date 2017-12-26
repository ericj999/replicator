#include "stdafx.h"
#include <string>
#include <shellapi.h>
#include <vector>

#include "StringT.h"
#include "ShellFolder.h"
#include "ComMemObj.h"
#include "util.h"

namespace ShellWrapper
{
// IShellFolder
	ShellFolder::ShellFolder(std::wstring const& path)
	{
		Open(path);
	}

	void ShellFolder::Open(std::wstring const& path)
	{
		if (m_interface)
		{
			m_interface->Release();
			m_interface = nullptr;
		}
		std::vector<std::wstring> parsingPath = Util::ParseParsingPath(path);
		if (parsingPath.size() < 2)
		{
			throw std::runtime_error("Cannot replicate entire device!");
		}

		if (path[0] == L':')
			OpenByEnumeration(parsingPath);
		else
			OpenDirect(path);

		if (!m_interface)
		{
			throw std::runtime_error("Failed to open folder!");
		}
	}

	void ShellFolder::OpenDirect(std::wstring const& path)
	{
		HRESULT hr = E_FAIL;
		ShellFolder desktopFolder;
		if (SUCCEEDED(SHGetDesktopFolder(&desktopFolder)))
		{
			ComMemObj<ITEMIDLIST> itemId;

			if (SUCCEEDED(hr = desktopFolder->ParseDisplayName(NULL, nullptr, const_cast<LPWSTR>(path.c_str()), nullptr, &itemId, nullptr)))
			{
				ShellWrapper::ShellFolder folder;
				hr = desktopFolder->BindToObject(itemId.Get(), nullptr, IID_IShellFolder, reinterpret_cast<void**>(&folder));
				if (SUCCEEDED(hr))
				{
					m_interface = folder.m_interface;
					m_interface->AddRef();
				}
			}
		}
	}

	void ShellFolder::OpenByEnumeration(const std::vector<std::wstring>& parsingPath)
	{
		HRESULT hr = E_FAIL;
		ShellFolder desktopFolder;

		if (SUCCEEDED(SHGetDesktopFolder(&desktopFolder)))
		{
			bool found = false;
			size_t index = 0;
			ShellWrapper::ShellFolder currentFolder{ desktopFolder };

			for (index = 0; index < parsingPath.size(); ++index)
			{
				found = false;
				EnumIDList enumIDList;
				if (SUCCEEDED(hr = currentFolder->EnumObjects(NULL, SHCONTF_FOLDERS, &enumIDList)))
				{
					HRESULT getResult = E_FAIL;
					do
					{
						ULONG get = 10, got = 0;
						ComMemArray<LPITEMIDLIST, 10> folderItemIdList;

						getResult = enumIDList->Next(get, folderItemIdList.data(), &got);
						for (int i = 0; i < static_cast<int>(got); ++i)
						{
							std::wstring name = currentFolder.GetDisplayNameOf(folderItemIdList[i], SHGDN_FORPARSING | SHGDN_INFOLDER);
							if (parsingPath[index].compare(name) == 0)
							{
								ShellFolder childFolder;
								if (SUCCEEDED(hr = currentFolder->BindToObject(folderItemIdList[i], nullptr, IID_IShellFolder, reinterpret_cast<void**>(&childFolder))))
								{
									found = true;
									currentFolder = childFolder;
								}
								break;
							}
						}
						if (found)
							break;

					} while ((getResult == S_OK) && SUCCEEDED(hr));
				}
				if (!found)
					break;
			}

			if (found && (index == parsingPath.size()))
			{
				m_interface = currentFolder.m_interface;
				m_interface->AddRef();
			}
		}
	}

	std::wstring ShellFolder::GetDisplayNameOf(PCUITEMID_CHILD pidl, SHGDNF flags)
	{
		std::wstring name;
		STRRET strRet;
		memset(&strRet, 0, sizeof(STRRET));
		strRet.uType = STRRET_CSTR;

		if (SUCCEEDED(m_interface->GetDisplayNameOf(pidl, flags, &strRet)))
			name = strRet.pOleStr;

		return name;
	}

	ShellFolder ShellFolder::CreateSubFolder(const std::wstring& subFolderName)
	{
		ShellFolder subFolder;
		ShellItem itemObj;

		SHGetItemFromObject(m_interface, IID_IShellItem, (void**)&itemObj);
		if (itemObj.isValid())
		{
			ShellItem subItem = itemObj.CreateChildItem(subFolderName, FILE_ATTRIBUTE_DIRECTORY);
			if(subItem.isValid())
			{
				subItem->BindToHandler(nullptr, BHID_SFObject, IID_IShellFolder, (void**)&subFolder);
			}
		}
		return subFolder;
	}

	ShellItem2 ShellFolder::OpenFileItem(const std::wstring& filename)
	{
		ShellItem2 shellItem2;
		ShellItem itemObj;

		SHGetItemFromObject(m_interface, IID_IShellItem, (void**)&itemObj);
		if (itemObj.isValid())
		{
			ShellItem subItem = itemObj.OpenChildItem(filename);
			if (subItem.isValid())
			{
				subItem->QueryInterface(IID_IShellItem2, (void**)&shellItem2);
			}
		}
		return shellItem2;
	}

	// doesn't work when using the IStream object.....
	ShellItem ShellFolder::CreateFileItem(const std::wstring& filename)
	{
		ShellItem itemObj;
		ShellItem nullItem;

		SHGetItemFromObject(m_interface, IID_IShellItem, (void**)&itemObj);
		if (itemObj.isValid())
		{
			ShellItem subItem = itemObj.CreateChildItem(filename, FILE_ATTRIBUTE_NORMAL);
			return subItem;
		}
		return nullItem;
	}

// ShellItem
	// open the child item if exists, else create a new child item
	ShellItem ShellItem::CreateChildItem(const std::wstring& name, DWORD attributes)
	{
		HRESULT hr = E_FAIL;
		ShellItem subItem, newItem;

		newItem = OpenChildItem(name);

		if (!newItem)
		{
			ComInterface<IFileOperation> fileOperation{ CLSID_FileOperation };

			hr = fileOperation->SetOperationFlags(FOF_NO_UI);

			if (SUCCEEDED(hr = fileOperation->NewItem(m_interface, attributes, name.c_str(), NULL, NULL)))
			{
				if (SUCCEEDED(hr = fileOperation->PerformOperations()))
				{
					hr = SHCreateItemFromRelativeName(m_interface, name.c_str(), NULL, IID_IShellItem, (void**)&newItem);
					if (SUCCEEDED(hr))
					{
						return newItem;
					}
				}
			}
		}
		else
			return newItem;
			
		return subItem;
	}

	ShellItem ShellItem::OpenChildItem(const std::wstring& name)
	{
		ShellItem subItem, newItem;

		HRESULT hr = SHCreateItemFromRelativeName(m_interface, name.c_str(), NULL, IID_IShellItem, (void**)&newItem);
		if (SUCCEEDED(hr))
		{
			return newItem;
		}
		return subItem;
	}
		
	bool ShellItem::IsFolder()
	{
		SFGAOF attrs = 0;
		HRESULT hr = E_FAIL;

		if (SUCCEEDED(hr = m_interface->GetAttributes(SFGAO_CAPABILITYMASK | SFGAO_DISPLAYATTRMASK | SFGAO_CONTENTSMASK | SFGAO_STORAGECAPMASK, &attrs)))
			return (attrs & SFGAO_FOLDER);

		return false;
	}

// IShellItem2
	std::wstring ShellItem2::GetName(SIGDN sigdn)
	{
		LPWSTR pwstr = nullptr;
		std::wstring wstr;
		HRESULT hr = E_FAIL;

		if (SUCCEEDED(hr = m_interface->GetDisplayName(sigdn, &pwstr)) && pwstr)
		{
			wstr = pwstr;
			CoTaskMemFree(pwstr);
		}
		return wstr;
	}

	bool ShellItem2::IsFolder()
	{
		SFGAOF attrs = 0;
		HRESULT hr = E_FAIL;

		if (SUCCEEDED(hr = m_interface->GetAttributes(SFGAO_CAPABILITYMASK | SFGAO_DISPLAYATTRMASK | SFGAO_CONTENTSMASK | SFGAO_STORAGECAPMASK, &attrs)))
			return (attrs & SFGAO_FOLDER);

		return false;
	}
}