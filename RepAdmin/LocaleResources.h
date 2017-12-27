#pragma once

#include <string>

enum StringResource
{
	aborted = 0,
	found,
	adding,
	updating,
	skipped,
	failedToAdd,
	failedToUpdate,
	failedToCreateFolder,
	max
};

void InitLocaleResource();

const std::wstring& GetLocalizedString(StringResource id);
