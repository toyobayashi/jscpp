#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif

#include "jscpp/Process.hpp"

#ifdef _WIN32
#include <direct.h>
#include <process.h>
#else
#include <unistd.h>
#include <dirent.h>
#ifdef __APPLE__
#include <crt_externs.h>
#define environ (*_NSGetEnviron())
#else
extern char** environ;
#endif
#endif

#if defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))
#include <TargetConditionals.h>
#endif

namespace js {

std::map<String, String> Process::getEnv() {
  std::map<String, String> env;
#ifdef _WIN32
  wchar_t* environment = GetEnvironmentStringsW();
  if (environment == nullptr) return env;
  wchar_t* p = environment;
  while (*p) {
    if (*p == L'=') {
      p += wcslen(p) + 1;
      continue;
    }
    String e = p;
    std::vector<String> keyvalue = e.split(L"=");
    env[keyvalue[0]] = keyvalue[1];

    p += wcslen(p) + 1;
  }
  FreeEnvironmentStringsW(environment);
#else
  int env_size = 0;
  while (environ[env_size]) {
    String e = environ[env_size];
    std::vector<String> keyvalue = e.split(L"=");
    env[keyvalue[0]] = keyvalue[1];
    env_size++;
  }
#endif
  return env;
}

int Process::getPid() noexcept {
#ifdef _WIN32
  return _getpid();
#else
  return getpid();
#endif
}

String Process::getPlatform() noexcept {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  return "win32";
#elif defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))
#if defined(TARGET_OS_MAC) && TARGET_OS_MAC
    return "darwin";
    // #define I_OS_DARWIN
    // #ifdef __LP64__
    //   #define I_OS_DARWIN64
    // #else
    //   #define I_OS_DARWIN32
    // #endif
#else
    return "unknown";
#endif
#elif defined(__ANDROID__) || defined(ANDROID)
  return "android";
#elif defined(__linux__) || defined(__linux)
  return "linux";
#elif defined(__EMSCRIPTEN__)
  return "browser";
#else
  return "unknown";
#endif
}
  
Process::Process(): pid(Process::getPid()), env(Process::getEnv()), platform(Process::getPlatform()) {}

String Process::cwd() const noexcept {
#ifdef _WIN32
  wchar_t* buf;
  if ((buf = _wgetcwd(nullptr, 0)) == nullptr) {
    return "";
  }
  String res = buf;
  free(buf);
  return res;
#else
  char* buf;
  if ((buf = getcwd(nullptr, 0)) == nullptr) {
    return "";
  }
  String res = buf;
  free(buf);
  return res;
#endif
}

Process process;

}
