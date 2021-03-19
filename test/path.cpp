#include "gtest/gtest.h"
#include "jscpp/path.hpp"
#include "jscpp/Process.hpp"
#include "jscpp/Console.hpp"

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

TEST(jscppPath, relative) {
  EXPECT_EQ(path::posix::relative("/data/orandea/test/aaa", "/data/orandea/impl/bbb"), L"../../impl/bbb");
  EXPECT_EQ(path::win32::relative("C:\\orandea\\test\\aaa", "C:\\orandea\\impl\\bbb"), L"..\\..\\impl\\bbb");
}

TEST(jscppPath, dirname) {
  EXPECT_EQ(path::dirname("/foo/bar/baz/asdf/quux"), L"/foo/bar/baz/asdf");
  EXPECT_EQ(path::dirname("/foo/bar/baz/asdf/.."), L"/foo/bar/baz/asdf");
  EXPECT_EQ(path::dirname("."), L".");
  EXPECT_EQ(path::dirname(".."), L".");
}

TEST(jscppPath, basename) {
  EXPECT_EQ(path::basename("/foo/bar/baz/asdf/quux.html"), L"quux.html");
  EXPECT_EQ(path::basename("/foo/bar/baz/asdf/quux.html", ".html"), L"quux");
  EXPECT_EQ(path::basename("."), L".");
  EXPECT_EQ(path::basename(".."), L"..");
}

TEST(jscppPath, extname) {
  EXPECT_EQ(path::extname("index.html"), L".html");
  EXPECT_EQ(path::extname("index.coffee.md"), L".md");
  EXPECT_EQ(path::extname("index."), L".");
  EXPECT_EQ(path::extname("index"), L"");
  EXPECT_EQ(path::extname(".index"), L"");
  EXPECT_EQ(path::extname(".index.md"), L".md");
}

TEST(jscppPath, parseAndFormat) {
  path::ParsedPath p = path::posix::parse("/home/user/dir/file.txt");
  EXPECT_EQ(p.root, L"/");
  EXPECT_EQ(p.dir, L"/home/user/dir");
  EXPECT_EQ(p.base, L"file.txt");
  EXPECT_EQ(p.ext, L".txt");
  EXPECT_EQ(p.name, L"file");

  path::ParsedPath p2 = path::win32::parse("C:\\path\\dir\\file.txt");
  EXPECT_EQ(p2.root, L"C:\\");
  EXPECT_EQ(p2.dir, L"C:\\path\\dir");
  EXPECT_EQ(p2.base, L"file.txt");
  EXPECT_EQ(p2.ext, L".txt");
  EXPECT_EQ(p2.name, L"file");

  path::ParsedPath obj1;
  obj1.root = L"/ignored";
  obj1.dir = L"/home/user/dir";
  obj1.base = L"file.txt";
  EXPECT_EQ(path::posix::format(obj1), L"/home/user/dir/file.txt");

  path::ParsedPath obj2;
  obj2.root = L"/";
  obj2.base = L"file.txt";
  obj2.ext = L"ignored";
  EXPECT_EQ(path::posix::format(obj2), L"/file.txt");

  path::ParsedPath obj3;
  obj3.root = L"/";
  obj3.name = L"file";
  obj3.ext = L".txt";
  EXPECT_EQ(path::posix::format(obj3), L"/file.txt");

  path::ParsedPath obj4;
  obj4.dir = L"C:\\path\\dir";
  obj4.name = L"file.txt";
  EXPECT_EQ(path::win32::format(obj4), L"C:\\path\\dir\\file.txt");
}

TEST(jscppPath, constants) {
  EXPECT_EQ(path::win32::sep, L"\\");
  EXPECT_EQ(path::posix::sep, L"/");
  EXPECT_EQ(path::win32::delimiter, L";");
  EXPECT_EQ(path::posix::delimiter, L":");
#ifdef _WIN32
  EXPECT_EQ(path::sep, L"\\");
  EXPECT_EQ(path::delimiter, L";");
#else
  EXPECT_EQ(path::sep, L"/");
  EXPECT_EQ(path::delimiter, L":");
#endif
  console.log(__filename);
  console.log(__dirname);
}
