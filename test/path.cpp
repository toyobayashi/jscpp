#include "gtest/gtest.h"
#include "jscpp/path.hpp"

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
