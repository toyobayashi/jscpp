#include "gtest/gtest.h"
#include "jscpp/path.hpp"
#include "jscpp/Process.hpp"

using namespace js;

TEST(jscppPath, isAbsolute) {
  EXPECT_TRUE(path::posix::isAbsolute("/foo/bar"));
  EXPECT_TRUE(path::posix::isAbsolute("/baz/.."));
  EXPECT_FALSE(path::posix::isAbsolute("qux/"));
  EXPECT_FALSE(path::posix::isAbsolute("."));

  EXPECT_TRUE(path::win32::isAbsolute("//server"));
  EXPECT_TRUE(path::win32::isAbsolute("\\\\server"));
  EXPECT_TRUE(path::win32::isAbsolute("C:/foo/.."));
  EXPECT_TRUE(path::win32::isAbsolute("C:\\foo\\.."));
  EXPECT_FALSE(path::win32::isAbsolute("bar\\baz"));
  EXPECT_FALSE(path::win32::isAbsolute("bar/baz"));
  EXPECT_FALSE(path::win32::isAbsolute("."));
}

TEST(jscppPath, resolve) {
  EXPECT_EQ(path::resolve(), process.cwd());
#ifdef _WIN32
  EXPECT_EQ(path::resolve("/foo/bar", "/tmp/file/").substring(1), L":\\tmp\\file");
  EXPECT_EQ(path::resolve("/foo/bar", "./baz").substring(1), L":\\foo\\bar\\baz");
  EXPECT_EQ(path::resolve("/foo/bar", "./baz", "././aaa").substring(1), L":\\foo\\bar\\baz\\aaa");
#else
  EXPECT_EQ(path::resolve("/foo/bar", "/tmp/file/"), L"/tmp/file");
  EXPECT_EQ(path::resolve("/foo/bar", "./baz"), L"/foo/bar/baz");
  EXPECT_EQ(path::resolve("/foo/bar", "./baz", "././aaa"), L"/foo/bar/baz/aaa");
#endif

  EXPECT_EQ(path::resolve("wwwroot", "static_files/png/", "../gif/image.gif"), path::join(process.cwd(), "wwwroot/static_files/gif/image.gif"));
}

TEST(jscppPath, normalize) {
  EXPECT_EQ(path::posix::normalize("/foo/bar//baz/asdf/quux/.."), L"/foo/bar/baz/asdf");
  EXPECT_EQ(path::win32::normalize("C:////temp\\\\/\\/\\/foo/bar"), L"C:\\temp\\foo\\bar");
}

TEST(jscppPath, join) {
  EXPECT_EQ(path::join(), L".");
  EXPECT_EQ(path::join("foo"), L"foo");
  EXPECT_EQ(path::join("./foo"), L"foo");
  EXPECT_EQ(path::join("./foo", ""), L"foo");
  EXPECT_EQ(path::join("./foo", "."), L"foo");
  EXPECT_EQ(path::join("./foo", "./a", ".."), L"foo");
  EXPECT_EQ(path::join("./foo", "./a", "b", "../.."), L"foo");
  EXPECT_EQ(path::posix::join("/foo", "bar", "baz/asdf", "quux", "..", "a", "bbb"), L"/foo/bar/baz/asdf/a/bbb");
  EXPECT_EQ(path::posix::join(L"中文", L"文件夹/1/2", ".."), L"中文/文件夹/1");

  EXPECT_EQ(path::win32::join("/foo", "bar", "baz/asdf", "quux", "..", "a", "bbb"), L"\\foo\\bar\\baz\\asdf\\a\\bbb");
  EXPECT_EQ(path::win32::join(L"中文", L"文件夹/1/2", ".."), L"中文\\文件夹\\1");
}
