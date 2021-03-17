#ifndef __JSCPP_ENCODING_HPP__
#define __JSCPP_ENCODING_HPP__

#include "config.h"
#include <string>

namespace js {

JSCPP_API std::wstring fromUtf8(const std::string& str);
JSCPP_API std::string toUtf8(const std::wstring& wstr);

JSCPP_API std::wstring fromAcp(const std::string& str);
JSCPP_API std::string toAcp(const std::wstring& wstr);

JSCPP_API std::wstring wstr(const std::string& s);
JSCPP_API std::string str(const std::wstring& s);

}

#endif
