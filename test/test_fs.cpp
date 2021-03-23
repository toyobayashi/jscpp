#include "gtest/gtest.h"
#include "jscpp/path.hpp"
#include "jscpp/fs.hpp"
#include "jscpp/Error.hpp"
#include "jscpp/Console.hpp"
#include "jscpp/Process.hpp"

using namespace js;

#if JSCPP_USE_ERROR
#define JSCPP_EXPECT_THROW(exp, msg) EXPECT_THROW(exp, Error)
#else
#define JSCPP_EXPECT_THROW(exp, msg) EXPECT_DEATH_IF_SUPPORTED(exp, msg)
#endif

TEST(jscppFilesystem, statAndLstat) {
  JSCPP_EXPECT_THROW(fs::stat("noexists"), "stat");
  JSCPP_EXPECT_THROW(fs::lstat("noexists"), "lstat");

  fs::Stats stats1 = fs::stat(__dirname);
  EXPECT_TRUE(stats1.isDirectory());

  fs::Stats stats2 = fs::lstat(__dirname);
  EXPECT_TRUE(stats2.isDirectory());

#ifndef __EMSCRIPTEN__
  EXPECT_TRUE(fs::stat(__filename).isFile());
  EXPECT_TRUE(fs::lstat(__filename).isFile());
#endif

  fs::symlink(__dirname, "slk3", fs::SymlinkType::ST_JUNCTION);
  EXPECT_TRUE(fs::exists("slk3"));
  fs::remove("slk3");
  EXPECT_FALSE(fs::exists("slk3"));
#ifndef __EMSCRIPTEN__
  fs::symlink(__filename, "slk");
  console.log(fs::realpath("slk"));
  EXPECT_TRUE(fs::exists("slk"));
  fs::Stats stat = fs::stat("slk");
  fs::Stats stat2 = fs::lstat("slk");
  console.log(stat.size);
  console.log(stat.isSymbolicLink());
  console.log(stat2.size);
  console.log(stat2.isSymbolicLink());
  fs::remove("slk");
  EXPECT_FALSE(fs::exists("slk"));
#endif

  fs::symlink("notexists", "slk2");
  EXPECT_TRUE(fs::exists("slk2"));
  fs::remove("slk2");
  EXPECT_FALSE(fs::exists("slk2"));
}

TEST(jscppFilesystem, readdir) {
  std::vector<String> items = fs::readdir(__dirname);
  console.log(items);
}

TEST(jscppFilesystem, exists) {
#ifndef __EMSCRIPTEN__
  EXPECT_TRUE(fs::exists(__filename));
#endif
  EXPECT_FALSE(fs::exists(L"noexists"));
}

TEST(jscppFilesystem, mkdirs) {
  EXPECT_NO_THROW(fs::mkdirs(__dirname));

  String mkdir1 = String(L"mkdir_") + process.platform + "_1";
  fs::mkdirs(mkdir1);
  EXPECT_TRUE(fs::exists(mkdir1));
  fs::remove(mkdir1);
  EXPECT_FALSE(fs::exists(mkdir1));

  String root = String("mkdir_") + process.platform + "_2";
  String mkdir2 = path::join(root, "subdir/a/b/c");
  fs::mkdirs(mkdir2);
  EXPECT_TRUE(fs::exists(mkdir2));
  fs::remove(root);
  EXPECT_FALSE(fs::exists(mkdir2));
  EXPECT_FALSE(fs::exists(root));

  String mkdir3 = __filename;
#ifndef __EMSCRIPTEN__
  JSCPP_EXPECT_THROW(fs::mkdirs(mkdir3), strerror(EEXIST));
#endif

  String mkdir4 = path::join(__filename, "fail");
#ifndef __EMSCRIPTEN__
  JSCPP_EXPECT_THROW(fs::mkdirs(mkdir4), strerror(ENOENT));
#else
  fs::mkdirs(mkdir4);
  console.log(fs::exists(mkdir4));
  console.log(fs::readdir(mkdir3));
  fs::remove(mkdir4);
  console.log(fs::exists(mkdir4));
  console.log(fs::readdir(mkdir3));
#endif
}

TEST(jscppFilesystem, copy) {
  String s = L"testwrite.txt";
  fs::writeFile(s, "666");
  auto d = path::basename(__filename + L".txt");
  fs::copyFile(s, d);
  fs::remove(s);

  EXPECT_TRUE(fs::exists(d));
  JSCPP_EXPECT_THROW(fs::copyFile(s, d, true), "");
  fs::remove(d);
  EXPECT_FALSE(fs::exists(d));
  fs::remove(s);
  EXPECT_FALSE(fs::exists(s));

  JSCPP_EXPECT_THROW(fs::copyFile("notexist", "any"), "");

  fs::mkdirs("./tmp/mkdir/a/b/c");
  fs::writeFile("./tmp/mkdir/a/b/c.txt", "1\n");
  fs::writeFile("./tmp/mkdir/a/b.txt", "1\n");
  fs::copy("./tmp/mkdir", "./tmp/copy");
  EXPECT_TRUE(fs::exists("./tmp/copy"));
  JSCPP_EXPECT_THROW(fs::copy("./tmp/mkdir", "./tmp/copy", true), "");
  JSCPP_EXPECT_THROW(fs::copy("./tmp/mkdir", "./tmp/mkdir/subdir"), "");
  // try {
  //   fs::copy("./tmp/mkdir", "./tmp/mkdir/subdir");
  //   return -1;
  // } catch (const std::exception& e) {
  //   console::error(e.what());
  // }

  fs::remove("./tmp");
  EXPECT_FALSE(fs::exists("./tmp"));
}

TEST(jscppFilesystem, readAndWrite) {
  String data = process.platform + L"测试\r\n";
  String append = L"append";

  EXPECT_NO_THROW(fs::writeFile("testwrite.txt", data));


  EXPECT_EQ(fs::readFileAsString("testwrite.txt"), data);
  fs::appendFile("testwrite.txt", append);
  EXPECT_EQ(fs::readFileAsString("testwrite.txt"), (data + append));
  fs::remove("testwrite.txt");

  fs::mkdirs("testmkdir");
  JSCPP_EXPECT_THROW(fs::readFileAsString("testmkdir"), "");
  fs::remove("testmkdir");
  JSCPP_EXPECT_THROW(fs::readFileAsString("notexists"), "No such file or directory");
}
