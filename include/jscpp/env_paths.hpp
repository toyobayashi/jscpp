#ifndef __JSCPP_ENV_PATHS_HPP__
#define __JSCPP_ENV_PATHS_HPP__

#include "String.hpp"

namespace js {

namespace env_paths {

class JSCPP_API Options {
public:
  String suffix;
};

class JSCPP_API Paths {
public:
  String data;
  String config;
  String cache;
  String log;
  String temp;
};

JSCPP_API Paths create(const String& name);
JSCPP_API Paths create(const String& name, const Options& options);

}

}

#endif
