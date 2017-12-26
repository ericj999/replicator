#pragma once

#include <Shlobj.h>
#include <propkey.h>
#include "ComInterface.h"
#include "ComMemObj.h"

namespace ShellWrapper
{
	class ShellItem : public ComInterface<IShellItem>
	{
	public:
		ShellItem() {}
		~ShellItem() {}

		ShellItem CreateChildItem(const std::wstring& name, DWORD attributes);
		ShellItem OpenChildItem(const std::wstring& name);
		bool IsFolder();

	};

	class ShellItem2 : public ComInterface<IShellItem2>
	{
	public:
		ShellItem2() {}
		~ShellItem2() {}

		std::wstring GetName(SIGDN signdn = SIGDN_NORMALDISPLAY);
		bool IsFolder();
	};

	class ShellFolder : public ComInterface<IShellFolder>
	{
	public:
		ShellFolder() {}
		ShellFolder(std::wstring const& path);
		~ShellFolder() {}

		void Open(std::wstring const& path);

		std::wstring GetDisplayNameOf(PCUITEMID_CHILD pidl, SHGDNF flags);
		ShellFolder CreateSubFolder(const std::wstring& subFolderName);
		ShellItem CreateFileItem(const std::wstring& filename);
		ShellItem2 OpenFileItem(const std::wstring& filename);

	protected:
		void OpenDirect(std::wstring const& path);
		void OpenByEnumeration(const std::vector<std::wstring>& parsingPath);
	};

	class Stream : public ComInterface<IStream>
	{
	public:
		Stream() {}
		~Stream() {}
	};

	class BindCtx : public ComInterface<IBindCtx>
	{
	public:
		BindCtx(DWORD modes)
		{
			if (FAILED(CreateBindCtx(0, &m_interface)))
				throw std::runtime_error("Failed to create bind object.");

			BIND_OPTS boptions = { sizeof(BIND_OPTS), 0, modes, 0 };
			m_interface->SetBindOptions(&boptions);
		}
		~BindCtx() {}
	};

	class EnumIDList : public ComInterface<IEnumIDList>
	{
	public:
		EnumIDList() {}
		~EnumIDList() {}
	};
}
