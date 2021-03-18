#ifndef __JSCPP_PATH_HPP__
#define __JSCPP_PATH_HPP__

#include "String.hpp"

namespace js {

namespace path {

namespace win32 {
  JSCPP_API bool isAbsolute(const String&);
}

namespace posix {
  JSCPP_API bool isAbsolute(const String&);
}

#ifdef _WIN32

inline bool isAbsolute(const String& path) { return win32::isAbsolute(path); }

#else

inline bool isAbsolute(const String& path) { return posix::isAbsolute(path); }

#endif

}

}


#endif