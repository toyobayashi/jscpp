#include "jscpp/path.hpp"
#include "jscpp/Process.hpp"

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

String _format(const String& sep, const ParsedPath& pathObject) {
  String dir = pathObject.dir.length() != 0 ? pathObject.dir : pathObject.root;
  String base = pathObject.base.length() != 0 ? pathObject.base : (pathObject.name + pathObject.ext);
  if (dir.length() == 0) {
    return base;
  }
  return dir == pathObject.root ? (dir + base) : (dir + sep + base);
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

String resolve(const String& arg1, const String& arg2) {
  std::vector<String> args = { arg1, arg2 };
  String resolvedDevice;
  String resolvedTail;
  bool resolvedAbsolute = false;

  for (int i = (int)args.size() - 1; i >= -1; i--) {
    String path;
    if (i >= 0) {
      path = args[i];

      // Skip empty entries
      if (path.length() == 0) {
        continue;
      }
    } else if (resolvedDevice.length() == 0) {
      path = process.cwd();
    } else {
      // Windows has the concept of drive-specific current working
      // directories. If we've resolved a drive letter but not yet an
      // absolute path, get cwd for that drive, or the process cwd if
      // the drive cwd is not available. We're sure the device is not
      // a UNC path at this points, because UNC paths are always absolute.
      String tmp = String(L"=") + resolvedDevice;
      String env;
      if (process.env.find(tmp) != process.env.end()) {
        env = process.env.at(tmp);
      }
      path = env.length() != 0 ? env : process.cwd();

      // Verify that a cwd was found and that it actually points
      // to our drive. If not, default to the drive's root.
      if (
          (!(path.slice(0, 2).toLowerCase() == resolvedDevice.toLowerCase()) &&
          path.charCodeAt(2) == CHAR_BACKWARD_SLASH)) {
        path = resolvedDevice + L"\\";
      }
    }

    const size_t len = path.length();
    int rootEnd = 0;
    String device;
    bool isAbsolute = false;
    const uint16_t code = path.charCodeAt(0);

    // Try to match a root
    if (len == 1) {
      if (isPathSeparator(code)) {
        // `path` contains just a path separator
        rootEnd = 1;
        isAbsolute = true;
      }
    } else if (isPathSeparator(code)) {
      // Possible UNC root

      // If we started with a separator, we know we at least have an
      // absolute path of some kind (UNC or otherwise)
      isAbsolute = true;

      if (isPathSeparator(path.charCodeAt(1))) {
        // Matched double path separator at beginning
        int j = 2;
        int last = j;
        // Match 1 or more non-path separators
        while (j < len && !isPathSeparator(path.charCodeAt(j))) {
          j++;
        }
        if (j < len && j != last) {
          const String firstPart = path.slice(last, j);
          // Matched!
          last = j;
          // Match 1 or more path separators
          while (j < len && isPathSeparator(path.charCodeAt(j))) {
            j++;
          }
          if (j < len && j != last) {
            // Matched!
            last = j;
            // Match 1 or more non-path separators
            while (j < len && !isPathSeparator(path.charCodeAt(j))) {
              j++;
            }
            if (j == len || j != last) {
              // We matched a UNC root
              device = String(L"\\\\") + firstPart + L"\\" + path.slice(last, j);
              rootEnd = j;
            }
          }
        }
      } else {
        rootEnd = 1;
      }
    } else if (isWindowsDeviceRoot(code) &&
                path.charCodeAt(1) == CHAR_COLON) {
      // Possible device root
      device = path.slice(0, 2);
      rootEnd = 2;
      if (len > 2 && isPathSeparator(path.charCodeAt(2))) {
        // Treat separator following drive name as an absolute path
        // indicator
        isAbsolute = true;
        rootEnd = 3;
      }
    }

    if (device.length() > 0) {
      if (resolvedDevice.length() > 0) {
        if (!(device.toLowerCase() == resolvedDevice.toLowerCase()))
          // This path points to another device so it is not applicable
          continue;
      } else {
        resolvedDevice = device;
      }
    }

    if (resolvedAbsolute) {
      if (resolvedDevice.length() > 0)
        break;
    } else {
      resolvedTail = path.slice(rootEnd) + L"\\" + resolvedTail;
      resolvedAbsolute = isAbsolute;
      if (isAbsolute && resolvedDevice.length() > 0) {
        break;
      }
    }
  }

  // At this point the path should be resolved to a full absolute path,
  // but handle relative paths to be safe (might happen when process.cwd()
  // fails)

  // Normalize the tail path
  resolvedTail = normalizeString(resolvedTail, !resolvedAbsolute, '\\',
                                  isPathSeparator);

  if (resolvedAbsolute) {
    return resolvedDevice + L"\\" + resolvedTail;
  }
  String r = resolvedDevice + resolvedTail;
  if (r.length() > 0) {
    return r;
  }
  return L".";
}

}


namespace posix {

bool isAbsolute(const String& path) { return path.length() > 0 && path.charCodeAt(0) == CHAR_FORWARD_SLASH; }

String resolve(const String& arg1, const String& arg2) {
  std::vector<String> args = { arg1, arg2 };
  String resolvedPath;
  bool resolvedAbsolute = false;

  for (int i = (int)args.size() - 1; i >= -1 && !resolvedAbsolute; i--) {
    String path = i >= 0 ? args[i] : process.cwd();

    // Skip empty entries
    if (path.length() == 0) {
      continue;
    }

    resolvedPath = path + L"/" + resolvedPath;
    resolvedAbsolute = path.charCodeAt(0) == CHAR_FORWARD_SLASH;
  }

  // At this point the path should be resolved to a full absolute path, but
  // handle relative paths to be safe (might happen when process.cwd() fails)

  // Normalize the path
  resolvedPath = normalizeString(resolvedPath, !resolvedAbsolute, '/',
                                  isPosixPathSeparator);

  if (resolvedAbsolute) {
    return L"/" + resolvedPath;
  }
  return resolvedPath.length() > 0 ? resolvedPath : L".";
}

}

}

}
