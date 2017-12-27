#include "stdafx.h"
#include "resource.h"
#include "LocaleResources.h"

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
