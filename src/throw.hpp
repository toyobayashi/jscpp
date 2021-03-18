#ifndef __JSCPP_THROW_HPP__
#define __JSCPP_THROW_HPP__

#include "jscpp/Error.hpp"

#if !defined(JSCPP_NORETURN)
#if defined(_MSC_VER) && _MSC_VER == 1800
#define JSCPP_NORETURN __declspec(noreturn)
#else
#define JSCPP_NORETURN [[noreturn]]
#endif
#endif

namespace js {

JSCPP_NORETURN void throwError(const String& msg);

}

#endif
