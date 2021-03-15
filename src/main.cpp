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

TEST(jscppString, charAt) {
  String str = L"Brave中文 new world";
  EXPECT_STREQ(str.charAt(0).str().c_str(), "B");
  EXPECT_STREQ(str.charAt(1).str().c_str(), "r");
  EXPECT_STREQ(str.charAt(2).str().c_str(), "a");
  EXPECT_STREQ(str.charAt(3).str().c_str(), "v");
  EXPECT_STREQ(str.charAt(4).str().c_str(), "e");
  EXPECT_STREQ(str.charAt(5).data(), L"中");
  EXPECT_STREQ(str.charAt(6).data(), L"文");
  EXPECT_STREQ(str.charAt(7).str().c_str(), " ");
  EXPECT_STREQ(str.charAt(8).str().c_str(), "n");
  EXPECT_STREQ(str.charAt(9).str().c_str(), "e");
  EXPECT_STREQ(str.charAt(999).str().c_str(), "");
}

TEST(jscppString, charCodeAt) {
  String str = L"ABC第E";
  EXPECT_EQ(str.charCodeAt(0), 65);
  EXPECT_EQ(str.charCodeAt(1), 66);
  EXPECT_EQ(str.charCodeAt(2), 67);
  EXPECT_EQ(str.charCodeAt(3), 31532);
  EXPECT_EQ(str.charCodeAt(4), 69);
  EXPECT_EQ(str.charCodeAt(5), 0);
}

TEST(jscppString, codePointAt) {
  EXPECT_EQ(String("ABC").codePointAt(1), 66);
  EXPECT_EQ(String(L"\xD800\xDC00").codePointAt(0), 65536);
  EXPECT_EQ(String("XYZ").codePointAt(42), 0);
}

TEST(jscppString, fromCharCode) {
  EXPECT_STREQ(String::fromCharCode(65, 66, 67).data(), L"ABC");
  EXPECT_STREQ(String::fromCharCode(0x2014).data(), L"—");
  EXPECT_STREQ(String::fromCharCode(0x12014).data(), L"—");
  EXPECT_STREQ(String::fromCharCode(8212).data(), L"—");
}

TEST(jscppString, fromCodePoint) {
  EXPECT_STREQ(String::fromCodePoint(42).data(), L"*");
  EXPECT_STREQ(String::fromCodePoint(0x404).data(), L"\x0404");
  EXPECT_STREQ(String::fromCodePoint(0x2F804).data(), L"\xD87E\xDC04");
  EXPECT_STREQ(String::fromCodePoint(194564).data(), L"\xD87E\xDC04");
  EXPECT_STREQ(String::fromCodePoint({ 0x1D306, 0x61, 0x1D307 }).data(), L"\xD834\xDF06\x61\xD834\xDF07");
  EXPECT_STREQ(String::fromCodePoint(8888888).data(), L"");
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
