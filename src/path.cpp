#include "jscpp/path.hpp"

namespace js {

namespace path {

namespace {
// Alphabet chars.
const uint16_t CHAR_UPPERCASE_A = 65; /* A */
const uint16_t CHAR_LOWERCASE_A = 97; /* a */
const uint16_t CHAR_UPPERCASE_Z = 90; /* Z */
const uint16_t CHAR_LOWERCASE_Z = 122; /* z */
const uint16_t CHAR_UPPERCASE_C = 67; /* C */
const uint16_t CHAR_LOWERCASE_B = 98; /* b */
const uint16_t CHAR_LOWERCASE_E = 101; /* e */
const uint16_t CHAR_LOWERCASE_N = 110; /* n */

// Non-alphabetic chars.
const uint16_t CHAR_DOT = 46; /* . */
const uint16_t CHAR_FORWARD_SLASH = 47; /* / */
const uint16_t CHAR_BACKWARD_SLASH = 92; /* \ */
const uint16_t CHAR_VERTICAL_LINE = 124; /* | */
const uint16_t CHAR_COLON = 58; /*  = */
const uint16_t CHAR_QUESTION_MARK = 63; /* ? */
const uint16_t CHAR_UNDERSCORE = 95; /* _ */
const uint16_t CHAR_LINE_FEED = 10; /* \n */
const uint16_t CHAR_CARRIAGE_RETURN = 13; /* \r */
const uint16_t CHAR_TAB = 9; /* \t */
const uint16_t CHAR_FORM_FEED = 12; /* \f */
const uint16_t CHAR_EXCLAMATION_MARK = 33; /* ! */
const uint16_t CHAR_HASH = 35; /* # */
const uint16_t CHAR_SPACE = 32; /*   */
const uint16_t CHAR_NO_BREAK_SPACE = 160; /* \u00A0 */
const uint16_t CHAR_ZERO_WIDTH_NOBREAK_SPACE = 65279; /* \uFEFF */
const uint16_t CHAR_LEFT_SQUARE_BRACKET = 91; /* [ */
const uint16_t CHAR_RIGHT_SQUARE_BRACKET = 93; /* ] */
const uint16_t CHAR_LEFT_ANGLE_BRACKET = 60; /* < */
const uint16_t CHAR_RIGHT_ANGLE_BRACKET = 62; /* > */
const uint16_t CHAR_LEFT_CURLY_BRACKET = 123; /* { */
const uint16_t CHAR_RIGHT_CURLY_BRACKET = 125; /* } */
const uint16_t CHAR_HYPHEN_MINUS = 45; /* - */
const uint16_t CHAR_PLUS = 43; /* + */
const uint16_t CHAR_DOUBLE_QUOTE = 34; /* " */
const uint16_t CHAR_SINGLE_QUOTE = 39; /* ' */
const uint16_t CHAR_PERCENT = 37; /* % */
const uint16_t CHAR_SEMICOLON = 59; /* ; */
const uint16_t CHAR_CIRCUMFLEX_ACCENT = 94; /* ^ */
const uint16_t CHAR_GRAVE_ACCENT = 96; /* ` */
const uint16_t CHAR_AT = 64; /* @ */
const uint16_t CHAR_AMPERSAND = 38; /* & */
const uint16_t CHAR_EQUAL = 61; /* = */

// Digits
const uint16_t CHAR_0 = 48; /* 0 */
const uint16_t CHAR_9 = 57; /* 9 */

#ifdef _WIN32
const wchar_t EOL[] = L"\r\n";
#else
const wchar_t EOL[] = L"\n";
#endif

bool isPathSeparator(uint16_t code) {
  return code == CHAR_FORWARD_SLASH || code == CHAR_BACKWARD_SLASH;
}

bool isPosixPathSeparator(uint16_t code) {
  return code == CHAR_FORWARD_SLASH;
}

bool isWindowsDeviceRoot(uint16_t code) {
  return (code >= CHAR_UPPERCASE_A && code <= CHAR_UPPERCASE_Z) ||
         (code >= CHAR_LOWERCASE_A && code <= CHAR_LOWERCASE_Z);
}

// Resolves . and .. elements in a path with directory names
String normalizeString(const String& path, bool allowAboveRoot, const String& separator, bool (*isPathSeparator)(uint16_t)) {
  String res;
  int lastSegmentLength = 0;
  int lastSlash = -1;
  int dots = 0;
  int code = 0;
  int pathLength = (int)path.length();
  for (int i = 0; i <= pathLength; ++i) {
    if (i < pathLength)
      code = path.charCodeAt(i);
    else if (isPathSeparator(code))
      break;
    else
      code = CHAR_FORWARD_SLASH;

    if (isPathSeparator(code)) {
      if (lastSlash == i - 1 || dots == 1) {
        // NOOP
      } else if (dots == 2) {
        if (res.length() < 2 || lastSegmentLength != 2 ||
            res.charCodeAt(res.length() - 1) != CHAR_DOT ||
            res.charCodeAt(res.length() - 2) != CHAR_DOT) {
          if (res.length() > 2) {
            size_t lastSlashIndex = res.lastIndexOf(separator);
            if (lastSlashIndex == std::wstring::npos) {
              res = L"";
              lastSegmentLength = 0;
            } else {
              res = res.slice(0, (int)lastSlashIndex);
              lastSegmentLength = (int)(res.length() - 1 - res.lastIndexOf(separator));
            }
            lastSlash = i;
            dots = 0;
            continue;
          } else if (res.length() != 0) {
            res = L"";
            lastSegmentLength = 0;
            lastSlash = i;
            dots = 0;
            continue;
          }
        }
        if (allowAboveRoot) {
          res += res.length() > 0 ? (separator + L"..")  : L"..";
          lastSegmentLength = 2;
        }
      } else {
        if (res.length() > 0)
          res += separator + path.slice(lastSlash + 1, i);
        else
          res = path.slice(lastSlash + 1, i);
        lastSegmentLength = i - lastSlash - 1;
      }
      lastSlash = i;
      dots = 0;
    } else if (code == CHAR_DOT && dots != -1) {
      ++dots;
    } else {
      dots = -1;
    }
  }
  return res;
}

}

namespace win32 {

bool isAbsolute(const String& path) {
  size_t len = path.length();
  if (len == 0)
    return false;

  uint16_t code = path.charCodeAt(0);
  return isPathSeparator(code) ||
    // Possible device root
    (len > 2 &&
    isWindowsDeviceRoot(code) &&
    path.charCodeAt(1) == CHAR_COLON &&
    isPathSeparator(path.charCodeAt(2)));
}

}


namespace posix {

bool isAbsolute(const String& path) { return path.length() > 0 && path.charCodeAt(0) == CHAR_FORWARD_SLASH; }

}

}

}
