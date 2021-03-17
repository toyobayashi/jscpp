#ifndef __JSCPP_PROCESS_HPP__
#define __JSCPP_PROCESS_HPP__

#include <map>
#include "String.hpp"

namespace js {

class JSCPP_API Process {
public:
  static int getPid() noexcept;
  static std::map<String, String> getEnv();
  static String getPlatform() noexcept;
public:
  int pid;
  std::map<String, String> env;
  String platform;

  Process();

  String cwd() const noexcept;
};

extern JSCPP_API const Process process;

}

#endif
