#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
// #include <direct.h>
// #include <io.h>
#endif

#include "jscpp/fs.hpp"
#include "jscpp/path.hpp"
#include "winerr.hpp"
#include "throw.hpp"
#include "jscpp/Error.hpp"
#include <cerrno>
#include <cstring>

namespace js {
namespace fs {

namespace {
  int internalAccess(const String& p, int mode) {
    String path = path::normalize(p);
#ifdef _WIN32

    DWORD attr = GetFileAttributesW(path.data());

    if (attr == INVALID_FILE_ATTRIBUTES) {
      return GetLastError();
    }

    /*
    * Access is possible if
    * - write access wasn't requested,
    * - or the file isn't read-only,
    * - or it's a directory.
    * (Directories cannot be read-only on Windows.)
    */
    if (!(mode & fs::AccessType::WOK) ||
        !(attr & FILE_ATTRIBUTE_READONLY) ||
        (attr & FILE_ATTRIBUTE_DIRECTORY)) {
      return 0;
    } else {
      return EPERM;
    }
#else
    if (::access(path.str().c_str(), mode) != 0) {
      return errno;
    }
#endif
    return 0;
  }
}

void access(const String& p, int mode) {
  int r = internalAccess(p, mode);
#ifdef _WIN32
  if (r != 0) {
    internal::throwError(internal::getWinErrorMessage(r) + L" access \"" + p + L"\"");
  }
#else
  if (r != 0) {
    internal::throwError(String(strerror(r)) + L" access \"" + p + L"\"");
  }
#endif
}

void chmod(const String& p, int mode) {
  String path = path::normalize(p);
#ifdef _WIN32
  if (::_wchmod(path.data(), mode) != 0) {
    internal::throwError(String(strerror(errno)) + L" chmod \"" + p + L"\"");
  }
#else
  if (::chmod(path.str().c_str(), mode) != 0) {
    internal::throwError(String(strerror(errno)) + L" chmod \"" + p + L"\"");
  }
#endif
}

bool exists(const String& p) {
#if JSCPP_USE_ERROR
  try {
    fs::access(p, fs::AccessType::FOK);
    return true;
  } catch (const Error&) {
    try {
      fs::lstat(p);
      return true;
    } catch (const Error&) {
      return false;
    }
  }
#else
  if (internalAccess(p, fs::AccessType::FOK) == 0) {
    return true;
  }
  fs::Stats stats;
  if (fs::Stats::createNoThrow(stats, p, false) == 0) {
    return true;
  }
  return false;
#endif
}

}
}
