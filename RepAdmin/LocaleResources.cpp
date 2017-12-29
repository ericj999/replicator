#include "stdafx.h"
#include "resource.h"
#include "LocaleResources.h"
#include "StringT.h"

struct ResourceString
{
	UINT id;
	PCWSTR defaultStr;
	std::wstring locString;
};

std::wstring nullStr;

ResourceString g_ResourceStrings[] =
{
	{ IDS_MSG_ABORTED, L"Replication was aborted." },
	{ IDS_MSG_FOUND, L"[%d] Found \"%s\"." },
	{ IDS_MSG_ADDING, L"Adding \"%s\". Source:\"%s\"" },
	{ IDS_MSG_UPDATING, L"Updating \"%s\"." },
	{ IDS_MSG_SKIPPED, L"Skipped \"%s\"." },
	{ IDS_MSG_FAILED_TO_ADD, L"Failed to add \"%s\"" },
	{ IDS_MSG_FAILED_TO_UPDATE, L"Failed to update \"%s\"" },
	{ IDS_MSG_FAILED_TO_CREATE_FOLDER, L"Failed to create folder \"%s\"." }
};

struct ExceptionString
{
	char* exception;
	UINT id;
};

ExceptionString g_exceptionStrings[] =
{
	{ EXCEPSTR_CREATE_INSTANCE_FAILURE , IDS_EXCEP_CREATE_INSTANCE_FAILURE },
	{ EXCEPSTR_DEST_FOLDER_FORMAT_EMPTY, IDS_EXCEP_DEST_FOLDER_FORMAT_EMPTY },
	{ EXCEPSTR_MISMATCHED_FOLDER_PATHS, IDS_EXCEP_MISMATCHED_FOLDER_PATHS },
	{ EXCEPSTR_CREATE_BIND_OBJ_FAILURE, IDS_EXCEP_CREATE_BIND_OBJ_FAILURE },
	{ EXCEPSTR_OPEN_FOLDER_FAILURE, IDS_EXCEP_ACCESS_PORTABLE_DEVICE_FAILURE },
	{ EXCEPSTR_ACCESS_PORTABLE_DEVICE_FAILURE, IDS_EXCEP_ACCESS_PORTABLE_DEVICE_FAILURE }
};

constexpr size_t g_exceptionStringsSize = sizeof(g_exceptionStrings) / sizeof(ExceptionString);

void InitLocaleResource()
{
	for (size_t index = 0; index < static_cast<size_t>(StringResource::max); ++index)
	{
		CString str;
		if (str.LoadString(g_ResourceStrings[index].id) && (str.GetLength() > 0))
			g_ResourceStrings[index].locString = str;
		else
			g_ResourceStrings[index].locString = g_ResourceStrings[index].defaultStr;
	}
}


const std::wstring& GetLocalizedString(StringResource id)
{
	if ((id >= 0) && (id < StringResource::max))
		return g_ResourceStrings[static_cast<int>(id)].locString;
	else
		return nullStr;
}

std::wstring GetLocalizedString(const std::string& str)
{
	std::wstring wstr;
	std::vector<std::string> elements;

	String::Tokenize<char>(str, elements, "|");
	if (elements.size() > 0)
	{
		for (size_t index = 0; index < g_exceptionStringsSize; ++index)
		{
			if (elements[0].compare(g_exceptionStrings[index].exception) == 0)
			{
				CString cstr;

				if (cstr.LoadString(g_exceptionStrings[index].id))
				{
					wstr = cstr;
					for (size_t i = 1; i < elements.size(); ++i)
					{
						std::wstringstream token;
						
						token << L"{" << ToStringT(i) << L"}";

						auto found = wstr.find(token.str());
						if(found != std::wstring::npos)
						{
							wstr.replace(found, token.str().length(), String::StringToStringT(elements[i]));
						}
					}
				}
				break;
			}
		}
	}
	return wstr;
}

