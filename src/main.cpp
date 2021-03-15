// #include <iostream>
#include "gtest/gtest.h"
// #define JSCPP_FORCE_UTF8
#include "jscpp/String.hpp"
// #include <cstdlib>

using namespace js;

TEST(jscppString, utf8) {
  std::string original = "\x7a\xc3\x9f\xe6\xb0\xb4\xf0\x9f\x8d\x8c";
  std::wstring r = fromUtf8(original);
#ifdef _WIN32
  EXPECT_EQ(r.size(), 5);
  EXPECT_EQ(r[0], 122);
  EXPECT_EQ(r[1], 223);
  EXPECT_EQ(r[2], 27700);
  EXPECT_EQ(r[3], 55356);
  EXPECT_EQ(r[4], 57164);
#else
  EXPECT_EQ(r.size(), 4);
  EXPECT_EQ(r[0], 122);
  EXPECT_EQ(r[1], 223);
  EXPECT_EQ(r[2], 27700);
  EXPECT_EQ(r[3], 127820);
#endif
  std::string o = toUtf8(r);
  EXPECT_STREQ(o.c_str(), original.c_str());
}

/* int main (int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
} */

/* int main () {
  std::wstring r = fromUtf8("\x7a\xc3\x9f\xe6\xb0\xb4\xf0\x9f\x8d\x8c");
  std::string u = toUtf8(r);
  // std::wstring r = fromUtf8(u8"z\u00df\u6c34\U0001f34c");
#ifdef _WIN32
  // system("chcp 65001");
#endif
  size_t len = r.length();
  for (size_t i = 0; i < len; i++) {
    std::cout << (unsigned int)r[i] << ", ";
  }
  std::cout << std::endl;

  len = u.length();
  for (size_t i = 0; i < len; i++) {
    std::cout << (int)u[i] << ", ";
  }
  std::cout << std::endl;
  std::cout << u << std::endl;

  String str = 413.8;
  std::cout << str.padStart(15, "998") << "666" << std::endl;

  String str2 = L"   啊aB被aBqqqaB我我我aB     ";
  std::cout << str2.toUpperCase() << std::endl;
  std::cout << str2.toLowerCase() << std::endl;

  auto a = str2.trim();
  auto b = str2.trimStart();
  auto c = str2.trimEnd();
  std::cout << a << std::endl;
  std::cout << b << std::endl;
  std::cout << c << std::endl;
  std::cout << str2 << std::endl;

  String v = "啊啊啊";
  std::cout << v << std::endl;
  return 0;
} */
