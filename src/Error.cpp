#include "throw.hpp"
#include "jscpp/Error.hpp"
#include "jscpp/String.hpp"

#if !JSCPP_USE_ERROR
#include <iostream>
#include <cstdlib>
#endif

namespace js {

#if JSCPP_USE_ERROR
Error::Error(const String& msg): _msg(msg.str()) {}

char const* Error::what() const noexcept { return _msg.c_str(); }
#endif

namespace internal {
  JSCPP_NORETURN void throwError(const String& msg) {
  #if JSCPP_USE_ERROR
    throw Error(msg);
  #else
    std::cerr << msg << std::endl;
    abort();
  #endif
  }
}

}
