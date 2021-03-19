#ifndef __JSCPP_PATH_HPP__
#define __JSCPP_PATH_HPP__

#include "String.hpp"

namespace js {

namespace path {

class JSCPP_API ParsedPath {
public:
  String root;
  String dir;
  String base;
  String ext;
  String name;
};

namespace win32 {
  JSCPP_API bool isAbsolute(const String& path);
  JSCPP_API String resolve(const String& arg1 = L"", const String& arg2 = L"");

  template <typename... Args>
  inline String resolve(const String& arg1, const String& arg2, const Args&... args) {
    String tmp = win32::resolve(arg1, arg2);
    return win32::resolve(tmp, args...);
  }

  JSCPP_API String normalize(const String& path);
  JSCPP_API String join();
  JSCPP_API String join(const String& arg1);
  JSCPP_API String join(const String& arg1, const String& arg2);

  template <typename... Args>
  inline String join(const String& arg1, const String& arg2, const Args&... args) {
    String tmp = win32::join(arg1, arg2);
    return win32::join(tmp, args...);
  }

  JSCPP_API String relative(const String& from, const String& to);
  JSCPP_API String toNamespacedPath(const String& path);
  JSCPP_API String dirname(const String& path);
  JSCPP_API String basename(const String& path, const String& ext = L"");
  JSCPP_API String extname(const String& path);
  JSCPP_API String format(const ParsedPath& pathObject);
  JSCPP_API ParsedPath parse(const String& path);
}

namespace posix {
  JSCPP_API bool isAbsolute(const String& path);
  JSCPP_API String resolve(const String& arg1 = L"", const String& arg2 = L"");

  template <typename... Args>
  inline String resolve(const String& arg1, const String& arg2, const Args&... args) {
    String tmp = posix::resolve(arg1, arg2);
    return posix::resolve(tmp, args...);
  }

  JSCPP_API String normalize(const String& path);
  JSCPP_API String join();
  JSCPP_API String join(const String& arg1);
  JSCPP_API String join(const String& arg1, const String& arg2);

  template <typename... Args>
  inline String join(const String& arg1, const String& arg2, const Args&... args) {
    String tmp = posix::join(arg1, arg2);
    return posix::join(tmp, args...);
  }

  JSCPP_API String relative(const String& from, const String& to);
  JSCPP_API String toNamespacedPath(const String& path);
  JSCPP_API String dirname(const String& path);
  JSCPP_API String basename(const String& path, const String& ext = L"");
  JSCPP_API String extname(const String& path);
  JSCPP_API String format(const ParsedPath& pathObject);
  JSCPP_API ParsedPath parse(const String& path);
}

#ifdef _WIN32

inline bool isAbsolute(const String& path) { return win32::isAbsolute(path); }

template <typename... Args>
inline String resolve(const Args&... args) {
  return win32::resolve(args...);
}

inline String normalize(const String& path) { return win32::normalize(path); }

template <typename... Args>
inline String join(const Args&... args) {
  return win32::join(args...);
}

inline String relative(const String& from, const String& to) { return win32::relative(from, to); }
inline String toNamespacedPath(const String& path) { return win32::toNamespacedPath(path); }
inline String dirname(const String& path) { return win32::dirname(path); }
inline String basename(const String& path, const String& ext = L"") { return win32::basename(path, ext); }
inline String extname(const String& path) { return win32::extname(path); }
inline String format(const ParsedPath& pathObject) { return win32::format(pathObject); }
inline ParsedPath parse(const String& path) { return win32::parse(path); }

#else

inline bool isAbsolute(const String& path) { return posix::isAbsolute(path); }

template <typename... Args>
inline String resolve(const Args&... args) {
  return posix::resolve(args...);
}

inline String normalize(const String& path) { return posix::normalize(path); }

template <typename... Args>
inline String join(const Args&... args) {
  return posix::join(args...);
}

inline String relative(const String& from, const String& to) { return posix::relative(from, to); }
inline String toNamespacedPath(const String& path) { return posix::toNamespacedPath(path); }
inline String dirname(const String& path) { return posix::dirname(path); }
inline String basename(const String& path, const String& ext = L"") { return posix::basename(path, ext); }
inline String extname(const String& path) { return posix::extname(path); }
inline String format(const ParsedPath& pathObject) { return posix::format(pathObject); }
inline ParsedPath parse(const String& path) { return posix::parse(path); }

#endif

}

}

#endif
