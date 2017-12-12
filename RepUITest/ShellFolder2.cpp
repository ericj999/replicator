#include "stdafx.h"
#include <string>
#include <vector>

#include "ShellFolder.h"
#include "ComMemObj.h"

using StringT = std::wstring;

namespace String
{
	void Tokenize(const StringT& str, std::vector<StringT>& tokens, const StringT& delimiters /*= _T(" ")*/)
	{
		// Skip delimiters at beginning.
		StringT::size_type lastPos = str.find_first_not_of(delimiters, 0);
		// Find first "non-delimiter".
		StringT::size_type pos = str.find_first_of(delimiters, lastPos);

		while ((pos != StringT::npos) || (lastPos != StringT::npos))
		{
			// Found a token, add it to the vector.
			tokens.push_back(str.substr(lastPos, pos - lastPos));
			// Skip delimiters.  Note the "not_of"
			lastPos = str.find_first_not_of(delimiters, pos);
			// Find next "non-delimiter"
			pos = str.find_first_of(delimiters, lastPos);
		}
	}
}

namespace ShellWrapper
{
	ShellFolder::ShellFolder(std::wstring const& path)
	{
		Open(path);
	}

	void ShellFolder::Open(std::wstring const& path)
	{
		ShellFolder desktopFolder;
		HRESULT hr;
		std::wstring device, folder;

		if (m_interface)
		{
			m_interface->Release();
			m_interface = nullptr;
		}

		std::string::size_type pos = path.find(L"\\\\?\\");
		if (pos != std::string::npos)
		{
			pos = path.find(L"\\", pos + 4);
			device = path.substr(0, pos);
			folder = path.substr(pos + 1);
		}

		ShellFolder workingFolder;

		if (SUCCEEDED(SHGetDesktopFolder(&desktopFolder)))
		{
			ComMemObj<ITEMIDLIST> itemId;
			if (SUCCEEDED(hr = desktopFolder->ParseDisplayName(NULL, nullptr, const_cast<LPWSTR>(device.c_str()), nullptr, &itemId, nullptr)))
			{
				ShellFolder deviceFolder;
				if (SUCCEEDED(hr = desktopFolder->BindToObject(itemId.Get(), nullptr, IID_IShellFolder, reinterpret_cast<void**>(&deviceFolder))))
				{
					std::vector<std::wstring> subFolders;
					String::Tokenize(folder, subFolders, L"\\");

					ShellFolder currentFolder{ deviceFolder };

					bool found = false;
					size_t index = 0;
					for (index = 0; index < subFolders.size(); ++index)
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
									STRRET strRet;
									memset(&strRet, 0, sizeof(STRRET));
									strRet.uType = STRRET_CSTR;

									if (SUCCEEDED(currentFolder->GetDisplayNameOf(folderItemIdList[i], SHGDN_FORPARSING | SHGDN_INFOLDER, &strRet)))
									{
										if (subFolders[index].compare(strRet.pOleStr) == 0)
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
								}
							} while ((getResult == S_OK) && SUCCEEDED(hr));
						}
						if (!found)
							break;
					}

					if (found && (index == subFolders.size()))
					{
						m_interface = currentFolder.m_interface;
						m_interface->AddRef();
					}
				}
			}
		}
		if (!m_interface)
		{
			throw std::runtime_error("Failed to open folder!");
		}
	}

	HRESULT ShellFolder::EnumShellFolder(const std::wstring& path)
	{
		HRESULT hr = E_FAIL;
		if (!m_interface)
			return hr;

		ComMemObj<ITEMIDLIST> itemId;
		BindCtx bindCtx;

		BIND_OPTS bindOpts;
		bindOpts.cbStruct = sizeof(BIND_OPTS);
		bindOpts.grfMode = STGM_READ;

		bindCtx->SetBindOptions(&bindOpts);

		SFGAOF attrReq = SFGAO_CAPABILITYMASK | SFGAO_DISPLAYATTRMASK | SFGAO_CONTENTSMASK | SFGAO_STORAGECAPMASK, attrs = 0;

		hr = SHParseDisplayName(path.c_str(), NULL, &itemId, attrReq, &attrs);

		if (SUCCEEDED(hr = m_interface->ParseDisplayName(NULL, nullptr, const_cast<LPWSTR>(path.c_str()), nullptr, &itemId, nullptr)))
		{
			ShellFolder folder;
			hr = m_interface->BindToObject(itemId.Get(), nullptr, IID_IShellFolder, reinterpret_cast<void**>(&folder));
			if (SUCCEEDED(hr))
			{
				EnumIDList enumIDList;
				if (SUCCEEDED(hr = folder->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &enumIDList)))
				{
					HRESULT getResult = E_FAIL;
					do
					{
						ULONG get = 10, got = 0;
						LPITEMIDLIST folderItemIdList[10] = { 0 };

						getResult = enumIDList->Next(get, folderItemIdList, &got);
						for (int i = 0; i < static_cast<int>(got); ++i)
						{
							STRRET strRet;
							memset(&strRet, 0, sizeof(STRRET));
							strRet.uType = STRRET_CSTR;

							folder->GetDisplayNameOf(folderItemIdList[i], SHGDN_NORMAL, &strRet);
							folder->GetDisplayNameOf(folderItemIdList[i], SHGDN_INFOLDER, &strRet);
							folder->GetDisplayNameOf(folderItemIdList[i], SHGDN_FOREDITING, &strRet);
							folder->GetDisplayNameOf(folderItemIdList[i], SHGDN_FORADDRESSBAR, &strRet);
							folder->GetDisplayNameOf(folderItemIdList[i], SHGDN_FORPARSING | SHGDN_INFOLDER, &strRet);

							SFGAOF attributes = SFGAO_CANCOPY | SFGAO_FOLDER | SFGAO_STREAM;

							ShellFolder childfolder;

							hr = folder->BindToObject(folderItemIdList[i], nullptr, IID_IShellFolder, reinterpret_cast<void**>(&childfolder));

//							if (SUCCEEDED(folder->GetAttributesOf(1, (PCUITEMID_CHILD_ARRAY)itemId.GetAt(i), &attributes)))
							if(SUCCEEDED(hr))
							{
								EnumIDList enumChildIDList;
								if (SUCCEEDED(hr = childfolder->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &enumChildIDList)))
								{
									HRESULT getResult = E_FAIL;
									do
									{
										ULONG get = 10, got = 0;
										LPITEMIDLIST childItemId[10] = { 0 };
										getResult = enumChildIDList->Next(get, childItemId, &got);
										for (int i = 0; i < static_cast<int>(got); ++i)
										{
											STRRET strRet;
											memset(&strRet, 0, sizeof(STRRET));
											strRet.uType = STRRET_CSTR;

											childfolder->GetDisplayNameOf(childItemId[i], SHGDN_NORMAL, &strRet);
											childfolder->GetDisplayNameOf(childItemId[i], SHGDN_INFOLDER, &strRet);
											childfolder->GetDisplayNameOf(childItemId[i], SHGDN_FOREDITING, &strRet);
											childfolder->GetDisplayNameOf(childItemId[i], SHGDN_FORADDRESSBAR, &strRet);
											childfolder->GetDisplayNameOf(childItemId[i], SHGDN_FORPARSING, &strRet);

											CoTaskMemFree(childItemId[i]);
											childItemId[i] = nullptr;
										}
									} while (getResult == S_OK);
								}


							}
							CoTaskMemFree(folderItemIdList[i]);
							folderItemIdList[i] = nullptr;
						}
					} while (getResult == S_OK);
				}
			}
		}
		return hr;
	}

	void Tokenize(const std::wstring& str, std::vector<std::wstring>& tokens, const std::wstring& delimiters /*= _T(" ")*/)
	{
		// Skip delimiters at beginning.
		std::wstring::size_type lastPos = str.find_first_not_of(delimiters, 0);
		// Find first "non-delimiter".
		std::wstring::size_type pos = str.find_first_of(delimiters, lastPos);

		while ((pos != std::wstring::npos) || (lastPos != std::wstring::npos))
		{
			// Found a token, add it to the vector.
			tokens.push_back(str.substr(lastPos, pos - lastPos));
			// Skip delimiters.  Note the "not_of"
			lastPos = str.find_first_not_of(delimiters, pos);
			// Find next "non-delimiter"
			pos = str.find_first_of(delimiters, lastPos);
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

}