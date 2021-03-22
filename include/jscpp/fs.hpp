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

enum AccessType {
  FOK = 0,
#ifdef _WIN32
  XOK = 0,
#else
  XOK = 1,
#endif
  WOK = 2,
  ROK = 4
};

enum SymlinkType {
  FILE,
  DIRECTORY,
  JUNCTION
};

class JSCPP_API Stats {
private:
  bool _isLink;
public:
  static Stats create(const String&, bool followLink = false);
  static int createNoThrow(Stats& out, const String& p, bool followLink = false);

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

class Dirent;
class Dir;

#ifdef _WIN32
class JSCPP_API Dirent {
private:
  struct _wfinddata_t* dirent_;

public:
  Dirent() noexcept;
  ~Dirent();
  Dirent(const Dirent&) = delete;
  Dirent& operator=(const Dirent&) = delete;
  Dirent(Dirent&&) noexcept;
  Dirent& operator=(Dirent&&);

  Dirent(struct _wfinddata_t*);

  bool isEmpty() const noexcept;

  const struct _wfinddata_t* data() const noexcept;

  String name() const noexcept;

  bool isFile() const noexcept;
  bool isDirectory() const noexcept;
  bool isFifo() const noexcept;
  bool isCharacterDevice() const noexcept;
  bool isSymbolicLink() const noexcept;
  bool isBlockDevice() const noexcept;
  bool isSocket() const noexcept;
};

class JSCPP_API Dir {
private:
  intptr_t dir_;
  String path_;
  struct _wfinddata_t* first_data_;
public:
  ~Dir() noexcept;
  Dir() noexcept;
  Dir(const Dir&) = delete;
  Dir& operator=(const Dir&) = delete;
  Dir(Dir&&) noexcept;
  Dir& operator=(Dir&&);

  static Dir create(const String&);
  void close();
  String path() const noexcept;
  fs::Dirent read();
};
#else

class JSCPP_API Dirent {
private:
  struct ::dirent* dirent_;

public:
  ~Dirent();
  Dirent() noexcept;
  Dirent(const Dirent&) = delete;
  Dirent& operator=(const Dirent&) = delete;
  Dirent(Dirent&&) noexcept;
  Dirent& operator=(Dirent&&);

  Dirent(struct ::dirent*);

  bool isEmpty() const noexcept;

  const struct ::dirent* data() const noexcept;

  String name() const noexcept;

  bool isFile() const noexcept;
  bool isDirectory() const noexcept;
  bool isFifo() const noexcept;
  bool isCharacterDevice() const noexcept;
  bool isSymbolicLink() const noexcept;
  bool isBlockDevice() const noexcept;
  bool isSocket() const noexcept;
};

class JSCPP_API Dir {
private:
  DIR* dir_;
  String path_;
public:
  ~Dir();
  Dir() noexcept;
  Dir(const Dir&) = delete;
  Dir& operator=(const Dir& d) = delete;
  Dir(Dir&&) noexcept;
  Dir& operator=(Dir&& d);

  static Dir create(const String& p);
  void close();
  String path() const noexcept;
  fs::Dirent read() const noexcept;
};
#endif

JSCPP_API fs::Dir opendir(const String&);
JSCPP_API std::vector<String> readdir(const String&);

JSCPP_API void access(const String&, int mode = 0);
JSCPP_API void chmod(const String&, int mode);
JSCPP_API bool exists(const String&);

JSCPP_API void mkdir(const String&, int mode = 0777);
JSCPP_API void mkdirs(const String&, int mode = 0777);
JSCPP_API void unlink(const String&);
JSCPP_API void rmdir(const String&);
JSCPP_API void rename(const String&, const String&);
JSCPP_API void remove(const String&);

}
}

#endif
