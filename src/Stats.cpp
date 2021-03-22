#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif

#include "jscpp/fs.hpp"
#include "jscpp/path.hpp"
#include "throw.hpp"
#include <cerrno>
#include <cstring>

namespace js {
namespace fs {

int Stats::createNoThrow(Stats& r, const String& p, bool followLink) {
  String path = path::normalize(p);
#ifdef _WIN32
  int code = 0;
  struct _stat info;

  if (followLink) { // stat
    code = _wstat(path.data(), &info);
    if (code != 0) {
      return errno;
    }
    r._isLink = false;
    r.dev = info.st_dev;
    r.ino = info.st_ino;
    r.mode = info.st_mode;
    r.nlink = info.st_nlink;
    r.gid = info.st_gid;
    r.uid = info.st_uid;
    r.rdev = info.st_rdev;
    r.size = info.st_size;
    r.atime = info.st_atime;
    r.mtime = info.st_mtime;
    r.ctime = info.st_ctime;
    return 0;
  }

  DWORD attrs = GetFileAttributesW(path.data());
  if (attrs == INVALID_FILE_ATTRIBUTES) {
    return ENOENT;
  }
  bool isLink = ((attrs & FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT);
  if (isLink) {
    r._isLink = true;
    r.dev = 0;
    r.ino = 0;
    r.mode = 0;
    r.nlink = 0;
    r.gid = 0;
    r.uid = 0;
    r.rdev = 0;
    r.size = 0;
    r.atime = 0;
    r.mtime = 0;
    r.ctime = 0;
  } else {
    code = _wstat(path.data(), &info);
    if (code != 0) {
      return errno;
    }
    r._isLink = false;
    r.dev = info.st_dev;
    r.ino = info.st_ino;
    r.mode = info.st_mode;
    r.nlink = info.st_nlink;
    r.gid = info.st_gid;
    r.uid = info.st_uid;
    r.rdev = info.st_rdev;
    r.size = info.st_size;
    r.atime = info.st_atime;
    r.mtime = info.st_mtime;
    r.ctime = info.st_ctime;
  }
  return 0;
#else
  int code = 0;
  struct stat info;
  std::string pathstr = path.str();
  if (followLink) {
    code = ::stat(pathstr.c_str(), &info);
    if (code != 0) {
      return errno;
    }
  } else {
    code = ::lstat(pathstr.c_str(), &info);
    if (code != 0) {
      return errno;
    }
  }

  // r.path_ = path;
  r._isLink = S_ISLNK(info.st_mode);
  r.dev = info.st_dev;
  r.ino = info.st_ino;
  r.mode = info.st_mode;
  r.nlink = info.st_nlink;
  r.gid = info.st_gid;
  r.uid = info.st_uid;
  r.rdev = info.st_rdev;
  r.size = info.st_size;
  r.atime = info.st_atime;
  r.mtime = info.st_mtime;
  r.ctime = info.st_ctime;
  return 0;
#endif
}

Stats Stats::create(const String& p, bool followLink) {
  Stats out;
  int r = createNoThrow(out, p, followLink);
  if (r != 0) {
    if (followLink) {
      internal::throwError(String(strerror(r)) + L", " + String(L"stat") + L" \"" + p + L"\"");
    } else {
      internal::throwError(String(strerror(r)) + L", " + String(L"lstat") + L" \"" + p + L"\"");
    }
  }
  return out;
}

bool Stats::isFile() const noexcept {
#ifdef _WIN32
  return ((this->mode & S_IFMT) == S_IFREG);
#else
  return S_ISREG(this->mode);
#endif
}

bool Stats::isDirectory() const noexcept {
#ifdef _WIN32
  return ((this->mode & S_IFMT) == S_IFDIR);
#else
  return S_ISDIR(this->mode);
#endif
}

bool Stats::isFifo() const noexcept {
#ifdef _WIN32
  return ((this->mode & S_IFMT) == _S_IFIFO);
#else
  return S_ISFIFO(this->mode);
#endif
}

bool Stats::isCharacterDevice() const noexcept {
#ifdef _WIN32
  return ((this->mode & S_IFMT) == S_IFCHR);
#else
  return S_ISCHR(this->mode);
#endif
}

bool Stats::isSymbolicLink() const noexcept {
  return this->_isLink;
}

bool Stats::isBlockDevice() const noexcept {
#ifdef _WIN32
  return false;
#else
  return S_ISBLK(this->mode);
#endif
}

bool Stats::isSocket() const noexcept {
#ifdef _WIN32
  return false;
#else
  return S_ISSOCK(this->mode);
#endif
}

Stats stat(const String& path) { return Stats::create(path, true); }
Stats lstat(const String& path) { return Stats::create(path, false); }

}
}
