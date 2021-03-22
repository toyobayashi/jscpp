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
