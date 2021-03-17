#ifndef __JSCPP_CONSOLE_HPP__
#define __JSCPP_CONSOLE_HPP__

#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include "Windows.h"

  #define COLOR_RED_BRIGHT (FOREGROUND_RED | FOREGROUND_INTENSITY)
  #define COLOR_YELLOW_BRIGHT (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
  #define COLOR_GREEN_BRIGHT (FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#else
#define COLOR_RED_BRIGHT ("\x1b[31;1m")
#define COLOR_YELLOW_BRIGHT ("\x1b[33;1m")
#define COLOR_GREEN_BRIGHT ("\x1b[32;1m")
#define COLOR_RESET ("\x1b[0m")

#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#endif

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten/val.h>
#endif

#include "String.hpp"

namespace js {

class JSCPP_API Console {
private:
#ifdef _WIN32
  static WORD _setConsoleTextAttribute(HANDLE hConsole, WORD wAttr);
#endif
  static std::string _format(const String&);
  static std::string _format(const std::vector<unsigned char>&);

  template <typename T>
  static std::string _format(const std::vector<T>& arr) {
    size_t len = arr.size();
    std::ostringstream oss;
    if (len == 0) {
      oss << "[]";
      return oss.str();
    }
    oss << "[ ";
    for (size_t i = 0; i < len; i++) {
      oss << arr[i];
      if (i != len - 1) {
        oss << ", ";
      }
    }
    oss << " ]";
    return oss.str();
  }

  template <typename K, typename V>
  static std::string _format(const std::map<K, V>& strobj) {
    size_t len = strobj.size();
    std::ostringstream oss;
    if (len == 0) {
      oss << "{}";
      return oss.str();
    }
    oss << "{\n";
    size_t i = 0;
    for (auto& p : strobj) {
      oss << std::string("  \"") << p.first << "\": \"" << p.second << "\"";
      if (i != len - 1) {
        oss << ",\n";
      } else {
        oss << "\n";
      }
      i++;
    }
    oss << "}";
    return oss.str();
  }

  template <typename T>
  static std::string _format(const T& arg) {
    std::ostringstream oss;
    oss << arg;
    return oss.str();
  }

  template <typename... Args>
  void _logerror(const String& format, Args... args) {
    String f = format + "\n";
    fprintf(stderr, f.str().c_str(), args...);
  }

  template <typename T>
  void _logerror(const T& arg) {
    _err << _format(arg) << std::endl;
  }

  std::ostream& _out;
  std::ostream& _err;

public:
  Console() noexcept;
  Console(const Console&) = delete;
  Console& operator=(const Console&) = delete;

  template <typename... Args>
  void write(const String& format, Args... args) {
    printf(format.str().c_str(), args...);
    _out.flush();
  }

  template <typename T>
  void write(const T& arg) {
    _out << _format(arg);
    _out.flush();
  }

  template <typename... Args>
  void log(const String& format, Args... args) {
    String f = format + "\n";
    printf(f.str().c_str(), args...);
  }

  template <typename T>
  void log(const T& arg) {
    _out << _format(arg) << std::endl;
  }

  template <typename T, typename... Args>
  void info(const T& arg, Args... args) {
#if defined(_WIN32)
    HANDLE hconsole = GetStdHandle(STD_OUTPUT_HANDLE);
    WORD original = Console::_setConsoleTextAttribute(hconsole, COLOR_GREEN_BRIGHT);
    log(arg, args...);
    Console::_setConsoleTextAttribute(hconsole, original);
#elif defined(__EMSCRIPTEN__)
    log(arg, args...);
#else
    _out << COLOR_GREEN_BRIGHT;
    log(arg, args...);
    _out << COLOR_RESET;
    _out.flush();
#endif
  }

  template <typename T, typename... Args>
  void warn(const T& arg, Args... args) {
#if defined(_WIN32)
    HANDLE hconsole = GetStdHandle(STD_ERROR_HANDLE);
    WORD original = Console::_setConsoleTextAttribute(hconsole, COLOR_YELLOW_BRIGHT);
    _logerror(arg, args...);
    Console::_setConsoleTextAttribute(hconsole, original);
#elif defined(__EMSCRIPTEN__)
    _logerror(arg, args...);
#else
    _err << COLOR_YELLOW_BRIGHT;
    _logerror(arg, args...);
    _err << COLOR_RESET;
    _err.flush();
#endif
  }

  template <typename T, typename... Args>
  void error(const T& arg, Args... args) {
#if defined(_WIN32)
    HANDLE hconsole = GetStdHandle(STD_ERROR_HANDLE);
    WORD original = Console::_setConsoleTextAttribute(hconsole, COLOR_RED_BRIGHT);
    _logerror(arg, args...);
    Console::_setConsoleTextAttribute(hconsole, original);
#elif defined(__EMSCRIPTEN__)
    _logerror(arg, args...);
#else
    _err << COLOR_RED_BRIGHT;
    _logerror(arg, args...);
    _err << COLOR_RESET;
    _err.flush();
#endif
  }
  unsigned short get_terminal_width() const;
  void clear();
  void clear_line(short lineNumber = 0);
};

extern JSCPP_API Console console;

}

#endif
