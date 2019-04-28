#include "StringUtils.h"
#include <codecvt>
#include <sstream>

std::wstring ConvertUTF8StringToUTF16String (const std::string& input)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring result = converter.from_bytes (input);
    return result;
}