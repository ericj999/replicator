#pragma once

#include <string>
#include "ExceptionString.h"

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
std::wstring GetLocalizedString(const std::string& str);
