#pragma once

#include <Shlobj.h>
#include <propkey.h>
#include "ComInterface.h"
#include "ComMemObj.h"

namespace ShellWrapper
{
	class ShellFolder : public ComInterface<IShellFolder>
	{
	public:
		ShellFolder() {}
		ShellFolder(std::wstring const& path);
		~ShellFolder() {}

		void Open(std::wstring const& path);

		std::wstring GetDisplayNameOf(PCUITEMID_CHILD pidl, SHGDNF flags);
	};

	class ShellItem : public ComInterface<IShellItem2>
	{
	public:
		ShellItem() {}
		~ShellItem() {}

		std::wstring GetName(SIGDN signdn = SIGDN_NORMALDISPLAY);
		bool IsFolder();
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

	std::vector<std::wstring> ParseParsingPath(const std::wstring& path);

}
