#pragma once

#include <string>

namespace base 
{
    // ANSI to UTF-8
    std::string AnsiToUTF8(const std::string& ansi);

    // code page
    std::wstring MultibytesToUnicode(const std::string& ansi, int cp);
    std::string UnicodeToMultibytes(const std::wstring& unicode, int cp);
}
