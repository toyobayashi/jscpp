#ifndef __JSCPP_ERROR_HPP__
#define __JSCPP_ERROR_HPP__

#include "config.h"

#if JSCPP_USE_ERROR
#include <string>
#include <exception>
#endif

namespace js {

#if JSCPP_USE_ERROR

class String;

class JSCPP_API Error : public std::exception {
public:
  Error(const String& msg);
  virtual ~Error() noexcept override = default;
  char const* what() const noexcept override;
protected:
  std::string _msg;
};

#endif

}

#endif
