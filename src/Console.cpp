#include "jscpp/Console.hpp"

#include <cstring>

#ifndef _WIN32
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#endif

namespace js {

#ifdef _WIN32
WORD Console::_setConsoleTextAttribute(HANDLE hConsole, WORD wAttr) {
  CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
  if (!GetConsoleScreenBufferInfo(hConsole, &csbiInfo)) return FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
  WORD originalAttr = csbiInfo.wAttributes;
  SetConsoleTextAttribute(hConsole, wAttr);
  return originalAttr;
}
#endif

std::string Console::_format(const String& str) {
  return str.str();
}

std::string Console::_format(const std::vector<unsigned char>& buf) {
  char _map[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
  std::ostringstream oss;
  oss << "<Buffer ";
  for (size_t i = 0; i < buf.size(); i++) {
    oss << _map[buf[i] >> 4];
    oss << _map[buf[i] & 0x0f];
    if (i != buf.size() - 1) {
      oss << " ";
    }
  }
  oss << ">";
  return oss.str();
}

Console::Console() noexcept: _out(std::cout), _err(std::cerr) {}

void Console::clear() {
#ifdef _WIN32
  HANDLE _consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  COORD coordScreen = { 0, 0 };
  DWORD cCharsWritten;
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  DWORD dwConSize;

  if (!GetConsoleScreenBufferInfo(_consoleHandle, &csbi)) {
    return;
  }

  dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

  if (!FillConsoleOutputCharacter(_consoleHandle,
    (TCHAR) ' ',
    dwConSize,
    coordScreen,
    &cCharsWritten))
  {
    return;
  }

  if (!GetConsoleScreenBufferInfo(_consoleHandle, &csbi)) {
    return;
  }

  if (!FillConsoleOutputAttribute(_consoleHandle,
    csbi.wAttributes,
    dwConSize,
    coordScreen,
    &cCharsWritten))
  {
    return;
  }

  SetConsoleCursorPosition(_consoleHandle, coordScreen);
#else
  _out << "\033[2J\033[1;1H";
  _out.flush();
#endif
}

unsigned short Console::getTerminalWidth() const {
#ifdef _WIN32
  CONSOLE_SCREEN_BUFFER_INFO bInfo;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &bInfo);
  return (unsigned short) bInfo.dwSize.X;
#else
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  return w.ws_col;
#endif
}

void Console::clearLine(short lineNumber) {
#ifdef _WIN32
  HANDLE _consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if (!GetConsoleScreenBufferInfo(_consoleHandle, &csbi)) return;
  short tmp = csbi.dwCursorPosition.Y - lineNumber;
  COORD targetFirstCellPosition = { 0, tmp < 0 ? 0 : tmp };
  DWORD size = csbi.dwSize.X * (lineNumber + 1);
  DWORD cCharsWritten;

  if (!FillConsoleOutputCharacterW(_consoleHandle, L' ', size, targetFirstCellPosition, &cCharsWritten)) return;
  if (!GetConsoleScreenBufferInfo(_consoleHandle, &csbi)) return;
  if (!FillConsoleOutputAttribute(_consoleHandle, csbi.wAttributes, size, targetFirstCellPosition, &cCharsWritten)) return;
  SetConsoleCursorPosition(_consoleHandle, targetFirstCellPosition);
#else
  unsigned short w = getTerminalWidth() - 1;
  char* b = new char[w + 1];
  memset(b, (int)' ', w);
  *(b + w) = '\0';
  for (short i = 0; i < lineNumber; i++) {
    _out << "\r" << b << "\r\x1b[1A";
  }
  _out << "\r" << b << "\r";
  delete[] b;
  _out.flush();
#endif
}

Console console;

}
