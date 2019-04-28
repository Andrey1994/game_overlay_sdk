#ifndef STRINGUTILS
#define STRINGUTILS

#include <windows.h>
#include <string>

std::wstring ConvertUTF8StringToUTF16String (const std::string& input);

#endif