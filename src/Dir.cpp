#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <direct.h>
#include <io.h>
#endif

#include "jscpp/fs.hpp"
#include "jscpp/path.hpp"
#include "throw.hpp"
#include <cerrno>
#include <cstring>

namespace js {
namespace fs {

#ifdef _WIN32

Dirent::~Dirent() {
  if (dirent_) {
    delete dirent_;
    dirent_ = nullptr;
  }
}

Dirent::Dirent() noexcept: dirent_(nullptr) {}

Dirent::Dirent(Dirent&& d) noexcept {
  if (this != &d) {
    dirent_ = d.dirent_;
    d.dirent_ = nullptr;
  }
}

Dirent& Dirent::operator=(Dirent&& d) {
  if (this != &d) {
    this->~Dirent();
    dirent_ = d.dirent_;
    d.dirent_ = nullptr;
  }
  return *this;
}

Dirent::Dirent(struct _wfinddata_t* d) {
  if (d) {
    dirent_ = new struct _wfinddata_t;
    memcpy(dirent_, d, sizeof(struct _wfinddata_t));
  }
}

bool Dirent::isEmpty() const noexcept { return dirent_ == nullptr; }

const struct _wfinddata_t* Dirent::data() const noexcept { return dirent_; }

String Dirent::name() const noexcept {
  if (dirent_) {
    return dirent_->name;
  } else {
    return L"";
  }
}

bool Dirent::isFile() const noexcept {
  if (dirent_) {
    return ((dirent_->attrib & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY) && ((dirent_->attrib & FILE_ATTRIBUTE_REPARSE_POINT) != FILE_ATTRIBUTE_REPARSE_POINT);
  } else {
    return false;
  }
}
bool Dirent::isDirectory() const noexcept {
  if (dirent_) {
    return ((dirent_->attrib & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);
  } else {
    return false;
  }
}
bool Dirent::isFifo() const noexcept {
  return false;
}
bool Dirent::isCharacterDevice() const noexcept {
  return false;
}
bool Dirent::isSymbolicLink() const noexcept {
  if (dirent_) {
    return ((dirent_->attrib & FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT);
  } else {
    return false;
  }
}
bool Dirent::isBlockDevice() const noexcept {
  return false;
}
bool Dirent::isSocket() const noexcept {
  return false;
}

Dir::~Dir() noexcept {
  if (first_data_) {
    delete first_data_;
    first_data_ = nullptr;
  }
  if (dir_ != -1) {
    _findclose(dir_);
    dir_ = -1;
  }
}

Dir::Dir() noexcept: dir_(-1), path_(), first_data_(nullptr) {}

Dir::Dir(Dir&& d) noexcept {
  dir_ = d.dir_;
  d.dir_ = -1;
  path_ = std::move(d.path_);
  first_data_ = d.first_data_;
  d.first_data_ = nullptr;
}

Dir& Dir::operator=(Dir&& d) {
  if (this != &d) {
    path_ = std::move(d.path_);
    this->~Dir();
    dir_ = d.dir_;
    d.dir_ = -1;
    first_data_ = d.first_data_;
    d.first_data_ = nullptr;
  }
  return *this;
}

Dir Dir::create(const String& p) {
  Dir dir;
  dir.path_ = p;
  
  String path = path::normalize(p);
  dir.first_data_ = new struct _wfinddata_t;
  dir.dir_ = _wfindfirst(path::win32::join(path, L"*").data(), dir.first_data_);
  if (dir.dir_ == -1) {
    delete dir.first_data_;
    dir.first_data_ = nullptr;
    internal::throwError(String(strerror(errno)) + L", " + String(L"opendir") + L" \"" + p + L"\"");
  }
  return dir;
}

String Dir::path() const noexcept { return path_; }

void Dir::close() {
  if (first_data_) {
    delete first_data_;
    first_data_ = nullptr;
  }
  if (dir_ != -1) {
    if (0 != _findclose(dir_)) {
      internal::throwError(String(strerror(errno)) + L", " + String(L"closedir") + L" \"" + path_ + L"\"");
    };
    dir_ = -1;
  }
}

fs::Dirent Dir::read() {
  if (first_data_) {
    fs::Dirent tmp(first_data_);
    delete first_data_;
    first_data_ = nullptr;
    return tmp;
  }
  struct _wfinddata_t* file = new struct _wfinddata_t;
  int ret = _wfindnext(dir_, file);
  if (ret == 0) {
    fs::Dirent tmp(file);
    delete file;
    return tmp;
  } else {
    delete file;
    return nullptr;
  }
}

#else
Dirent::~Dirent() { dirent_ = nullptr; }
Dirent::Dirent() noexcept: dirent_(nullptr) {}

Dirent::Dirent(Dirent&& d) noexcept {
  if (this != &d) {
    dirent_ = d.dirent_;
    d.dirent_ = nullptr;
  }
}

Dirent& Dirent::operator=(Dirent&& d) {
  if (this != &d) {
    dirent_ = d.dirent_;
    d.dirent_ = nullptr;
  }
  return *this;
}

Dirent::Dirent(struct ::dirent* d): dirent_(d) {}

bool Dirent::isEmpty() const noexcept { return dirent_ == nullptr; }

const struct ::dirent* Dirent::data() const noexcept { return dirent_; }

String Dirent::name() const noexcept {
  if (dirent_) {
    return dirent_->d_name;
  } else {
    return "";
  }
}

bool Dirent::isFile() const noexcept {
  if (dirent_) {
    return dirent_->d_type == DT_REG;
  } else {
    return false;
  }
}
bool Dirent::isDirectory() const noexcept {
  if (dirent_) {
    return dirent_->d_type == DT_DIR;
  } else {
    return false;
  }
}
bool Dirent::isFifo() const noexcept {
  if (dirent_) {
    return dirent_->d_type == DT_FIFO;
  } else {
    return false;
  }
}
bool Dirent::isCharacterDevice() const noexcept {
  if (dirent_) {
    return dirent_->d_type == DT_FIFO;
  } else {
    return false;
  }
}
bool Dirent::isSymbolicLink() const noexcept {
  if (dirent_) {
    return dirent_->d_type == DT_LNK;
  } else {
    return false;
  }
}
bool Dirent::isBlockDevice() const noexcept {
  if (dirent_) {
    return dirent_->d_type == DT_BLK;
  } else {
    return false;
  }
}
bool Dirent::isSocket() const noexcept {
  if (dirent_) {
    return dirent_->d_type == DT_SOCK;
  } else {
    return false;
  }
}

Dir Dir::create(const String& p) {
  Dir dir;
  dir.path_ = p;
  String path = path::normalize(p);
  if ((dir.dir_ = ::opendir(path.str().c_str())) == nullptr) {
    internal::throwError(String(strerror(errno)) + L", " + String(L"opendir") + L" \"" + p + L"\"");
  }
  return dir;
}

Dir::~Dir() {
  if (dir_) {
    closedir(dir_);
    dir_ = nullptr;
  }
}

Dir::Dir() noexcept: dir_(nullptr), path_() {}

Dir::Dir(Dir&& d) noexcept {
  dir_ = d.dir_;
  d.dir_ = nullptr;
  path_ = std::move(d.path_);
}

Dir& Dir::operator=(Dir&& d) {
  if (this != &d) {
    path_ = std::move(d.path_);
    this->~Dir();
    dir_ = d.dir_;
    d.dir_ = nullptr;
  }
  return *this;
}

String Dir::path() const noexcept { return path_; }

void Dir::close() {
  if (dir_) {
    if (0 != closedir(dir_)) {
      internal::throwError(String(strerror(errno)) + L", " + String(L"closedir") + L" \"" + path_ + L"\"");
    }
    dir_ = nullptr;
  }
}

fs::Dirent Dir::read() const noexcept {
  struct ::dirent *direntp = ::readdir(dir_);
  return direntp;
}
#endif

fs::Dir opendir(const String& p) {
  return fs::Dir::create(p);
}

std::vector<String> readdir(const String& p) {
  String newPath = path::normalize(p);
  fs::Dir dir = fs::opendir(newPath);

  std::vector<String> res;
  String item;

  fs::Dirent dirent;

  while (!((dirent = dir.read()).isEmpty())) {
    item = dirent.name();
    if (!(item == L".") && !(item == L"..")) {
      res.emplace_back(item);
    }
  }

  dir.close();
  return res;
}

}
}
