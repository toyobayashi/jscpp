#ifndef __JSCPP_WINERR_HPP__
#define __JSCPP_WINERR_HPP__

#ifdef _WIN32

#include "jscpp/String.hpp"

namespace js {
  namespace internal {
    String getWinErrorMessage(unsigned long code);
  }
}

#endif

#endif
