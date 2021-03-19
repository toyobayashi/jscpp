#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include "winerr.hpp"

namespace js {
  namespace internal {
    String getWinErrorMessage(unsigned long code) {
      LPVOID buf;
      if (FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_IGNORE_INSERTS
        | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL,
        code,
        GetSystemDefaultLangID() /* MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT) */,
        (LPWSTR) &buf,
        0,
        NULL
      )) {
        std::wstring msg = (wchar_t*)buf;
        LocalFree(buf);
        size_t pos = msg.find_last_of(L"\r\n");
        msg = msg.substr(0, pos - 1);
        return msg;
      } else {
        char buf[10] = { 0 };
        _itoa(GetLastError(), buf, 10);
        return String(L"Cannot format message. Win32 error code: ") + buf;
      }
    }
  }
}

#endif
