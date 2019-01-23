#include "string_convert.h"

#include <windows.h>
#include <vector>

namespace base 
{
    std::string AnsiToUTF8(const std::string& ansi)
    {
        return UnicodeToMultibytes(MultibytesToUnicode(ansi, CP_ACP), CP_UTF8);
    }

    std::wstring MultibytesToUnicode(const std::string& ansi, int cp)
    {
        int _convert = ::MultiByteToWideChar(cp, 0, ansi.c_str(), -1, nullptr, 0);
        if (_convert == 0) {
            return std::wstring();
        }

        std::vector<wchar_t> buffer(_convert + 1);
        _convert = ::MultiByteToWideChar(cp, 0, ansi.c_str(), -1, &buffer.front(), _convert + 1);
        if (_convert == 0) {
            return std::wstring();
        }

        return std::wstring(&buffer.front(), _convert);
    }

    std::string UnicodeToMultibytes(const std::wstring& unicode, int cp)
    {
        int _convert = ::WideCharToMultiByte(cp, 0, unicode.c_str(), -1, nullptr, 0, 0, nullptr);
        if (_convert == 0) {
            return std::string();
        }

        std::vector<char> buffer(_convert + 1);
        _convert = ::WideCharToMultiByte(cp, 0, unicode.c_str(), -1, &buffer.front(), _convert + 1, 0, nullptr);
        if (_convert == 0) {
            return std::string();
        }

        return std::string(&buffer.front(), _convert);
    }
}
