#ifndef __JSCPP_ENCODING_HPP__
#define __JSCPP_ENCODING_HPP__

#include "config.h"
#include <string>

#ifdef JSCPP_UTF8
  #define JSCPP_STR ::js::toUtf8
  #define JSCPP_WSTR ::js::fromUtf8
#else
  #define JSCPP_STR ::js::toString
  #define JSCPP_WSTR ::js::fromString
#endif

namespace js {

JSCPP_API std::wstring fromUtf8(const std::string& str);

JSCPP_API std::wstring fromString(const std::string& str);

JSCPP_API std::string toUtf8(const std::wstring& wstr);

JSCPP_API std::string toString(const std::wstring& wstr);

}

#endif
