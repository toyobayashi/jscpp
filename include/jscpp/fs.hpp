#ifndef __JSCPP_FS_HPP__
#define __JSCPP_FS_HPP__

#include "String.hpp"

#include <ctime>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32

#else
#include <dirent.h>
#include <unistd.h>
#endif

namespace js {

namespace fs {

enum class AccessType {
  FOK = 0,
#ifdef _WIN32
  XOK = 0,
#else
  XOK = 1,
#endif
  WOK = 2,
  ROK = 4
};

enum class SymlinkType {
  FILE,
  DIRECTORY,
  JUNCTION
};

class JSCPP_API Stats {
private:
  bool _isLink;
public:
  static Stats create(const String&, bool followLink = false);

  unsigned int dev;
  unsigned short ino;
  unsigned short mode;
  short nlink;
  short gid;
  short uid;
  unsigned int rdev;
  long size;
  time_t atime;
  time_t mtime;
  time_t ctime;

  bool isFile() const noexcept;
  bool isDirectory() const noexcept;
  bool isFifo() const noexcept;
  bool isCharacterDevice() const noexcept;
  bool isSymbolicLink() const noexcept;
  bool isBlockDevice() const noexcept;
  bool isSocket() const noexcept;
};

JSCPP_API Stats stat(const String&);
JSCPP_API Stats lstat(const String&);

}
}

#endif
