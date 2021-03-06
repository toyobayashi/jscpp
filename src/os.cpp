#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <userenv.h>
#else

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <cstdlib>
#include <cstring>
#include <cerrno>

#if defined(__ANDROID_API__) && __ANDROID_API__ < 21
# include <dlfcn.h>  /* for dlsym */
#endif

#endif

#include "jscpp/os.hpp"
#include "jscpp/Process.hpp"
#include "./internal/winerr.hpp"
#include "./internal/throw.hpp"

namespace js {

namespace os {

String tmpdir() {
  String path;
  const std::map<String, String>& env = process.env;
#ifdef _WIN32
  if (env.find(L"TEMP") != env.end() && env.at(L"TEMP").length() > 0) {
    path = env.at(L"TEMP");
  } else if (env.find(L"TMP") != env.end() && env.at(L"TMP").length() > 0) {
    path = env.at(L"TMP");
  } else if (env.find(L"SystemRoot") != env.end() && env.at(L"SystemRoot").length() > 0) {
    path = env.at(L"SystemRoot") + "\\temp";
  } else if (env.find(L"windir") != env.end() && env.at(L"windir").length() > 0) {
    path = env.at(L"windir") + L"\\temp";
  } else {
    wchar_t tempPath[MAX_PATH + 1] = { 0 };
    int len = GetTempPathW(MAX_PATH + 1, tempPath);
    if (len == 0) {
      path = L"C:\\temp";
    } else {
      path = String(tempPath) + L"\\temp";
    }
  }

  if (path.length() > 1 && path[path.length() - 1] == L'\\' && path[path.length() - 2] != L':') {
    path = path.slice(0, -1);
  }
#else
  if (env.find(L"TMPDIR") != env.end() && env.at(L"TMPDIR").length() > 0) {
    path = env.at(L"TMPDIR");
  } else if (env.find(L"TMP") != env.end() && env.at(L"TMP").length() > 0) {
    path = env.at(L"TMP");
  } else if (env.find(L"TEMP") != env.end() && env.at(L"TEMP").length() > 0) {
    path = env.at(L"TEMP");
  } else {
#if defined(__ANDROID__)
    path = L"/data/local/tmp";
#else
    path = L"/tmp";
#endif
  }

  if (path.length() > 1 && path[path.length() - 1] == L'/') {
    path = path.slice(0, -1);
  }
#endif
  return path;
}

String homedir() {
  const std::map<String, String>& env = process.env;
#ifdef _WIN32

  if (env.find(L"USERPROFILE") != env.end()) {
    return env.at(L"USERPROFILE");
  }

  HANDLE token;
  if (OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &token) == 0) {
    internal::throwError(internal::getWinErrorMessage(GetLastError()));
  }

  wchar_t path[MAX_PATH];
  DWORD bufsize = sizeof(path) / sizeof(path[0]);

  if (!GetUserProfileDirectoryW(token, path, &bufsize)) {
    CloseHandle(token);
    internal::throwError(internal::getWinErrorMessage(GetLastError()));
  }

  CloseHandle(token);

  return path;
#else
  if (env.find(L"HOME") != env.end()) {
    return env.at(L"HOME");
  }
#ifdef __EMSCRIPTEN__
  return L"/";
#else

  struct passwd pw;
  struct passwd* result;
  std::size_t bufsize;
#if defined(__ANDROID_API__) && __ANDROID_API__ < 21
  int (*getpwuid_r)(uid_t, struct passwd*, char*, size_t, struct passwd**);

  getpwuid_r = dlsym(RTLD_DEFAULT, "getpwuid_r");
  if (getpwuid_r == NULL) {
    internal::throwError(L"Can not call getpwuid_r.");
  }
#endif
  long initsize = sysconf(_SC_GETPW_R_SIZE_MAX);

  if (initsize <= 0)
    bufsize = 4096;
  else
    bufsize = (size_t) initsize;

  uid_t uid = geteuid();
  char* buf = (char*)malloc(bufsize);
  int r;
  
  for (;;) {
    free(buf);
    buf = (char*)malloc(bufsize);

    if (buf == NULL) {
      internal::throwError(strerror(errno));
    }

    r = getpwuid_r(uid, &pw, buf, bufsize, &result);

    if (r != ERANGE)
      break;

    bufsize *= 2;
  }

  if (r != 0) {
    free(buf);
    internal::throwError(L"getpwuid_r() failed.");
  }

  if (result == NULL) {
    free(buf);
    internal::throwError(strerror(ENOENT));
  }

  return pw.pw_dir;
#endif
#endif
}

}

}
