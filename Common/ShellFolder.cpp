#include "stdafx.h"
#include <string>
#include <vector>

#include "StringT.h"
#include "ShellFolder.h"
#include "ComMemObj.h"

namespace ShellWrapper
{
// IShellFolder
	ShellFolder::ShellFolder(std::wstring const& path)
	{
		Open(path);
	}

	void ShellFolder::Open(std::wstring const& path)
	{
		HRESULT hr;

		if (m_interface)
		{
			m_interface->Release();
			m_interface = nullptr;
		}

		std::vector<std::wstring> parsingPath = ShellWrapper::ParseParsingPath(path);
		if (parsingPath.size() < 2)
			throw std::runtime_error("Cannot replicate entire device!");

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
		if (!m_interface)
		{
			throw std::runtime_error("Failed to open folder!");
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

// IShellItem2
	std::wstring ShellItem::GetName(SIGDN sigdn)
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

	bool ShellItem::IsFolder()
	{
		SFGAOF attrs = 0;
		HRESULT hr = E_FAIL;

		if (SUCCEEDED(hr = m_interface->GetAttributes(SFGAO_CAPABILITYMASK | SFGAO_DISPLAYATTRMASK | SFGAO_CONTENTSMASK | SFGAO_STORAGECAPMASK, &attrs)))
			return (attrs & SFGAO_FOLDER);

		return false;
	}

// Utility functions
	std::vector<std::wstring> ParseParsingPath(const std::wstring& path)
	{
		std::vector<std::wstring> list;

		std::wstring::size_type lastPos = 0;
		std::wstring::size_type next = path.find_first_not_of(L"\\?", lastPos);
		std::wstring::size_type pos = path.find_first_of(L'\\', (next == std::wstring::npos) ? lastPos : next);

		for (;;)
		{
			if (pos == std::wstring::npos)
			{
				list.push_back(path.substr(lastPos));
				break;
			}
			else
				list.push_back(path.substr(lastPos, pos - lastPos));

			lastPos = pos + 1;
			next = path.find_first_not_of(L"\\?", lastPos);

			if (next == std::wstring::npos)
				break;	// end with \
							
			pos = path.find_first_of(L'\\', next);
		}
		return list;
	}
}