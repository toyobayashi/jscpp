#include "gtest/gtest.h"
#include "jscpp/path.hpp"
#include "jscpp/fs.hpp"
#include "jscpp/Error.hpp"

using namespace js;

TEST(jscppFilesystem, stat) {
#if JSCPP_USE_ERROR
  EXPECT_THROW(fs::stat("noexists"), Error);
  EXPECT_THROW(fs::lstat("noexists"), Error);
#else
  EXPECT_DEATH_IF_SUPPORTED(fs::stat("noexists"), "stat");
  EXPECT_DEATH_IF_SUPPORTED(fs::lstat("noexists"), "lstat");
#endif

  fs::Stats stats1 = fs::stat(__dirname);
  EXPECT_TRUE(stats1.isDirectory());

  fs::Stats stats2 = fs::lstat(__dirname);
  EXPECT_TRUE(stats2.isDirectory());
}
