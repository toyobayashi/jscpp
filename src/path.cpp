#include "jscpp/path.hpp"
#include "jscpp/Process.hpp"

#include <memory>

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
  resolvedTail = normalizeString(resolvedTail, !resolvedAbsolute, L"\\",
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

String normalize(const String& p) {
  String path = p;
  size_t len = path.length();
  if (len == 0)
    return L".";
  int rootEnd = 0;
  std::unique_ptr<String> device;

  bool isAbsolute = false;
  const uint16_t code = path.charCodeAt(0);

  // Try to match a root
  if (len == 1) {
    // `path` contains just a single char, exit early to avoid
    // unnecessary work
    return isPosixPathSeparator(code) ? L"\\" : path;
  }
  if (isPathSeparator(code)) {
    // Possible UNC root

    // If we started with a separator, we know we at least have an absolute
    // path of some kind (UNC or otherwise)
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
        String firstPart = path.slice(last, j);
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
          if (j == len) {
            // We matched a UNC root only
            // Return the normalized version of the UNC root since there
            // is nothing left to process
            return String(L"\\\\") + firstPart + L"\\" + path.slice(last) + L"\\";
          }
          if (j != last) {
            // We matched a UNC root with leftovers
            device.reset(new String(String(L"\\\\") + firstPart + L"\\" + path.slice(last, j)));
            rootEnd = j;
          }
        }
      }
    } else {
      rootEnd = 1;
    }
  } else if (isWindowsDeviceRoot(code) && path.charCodeAt(1) == CHAR_COLON) {
    // Possible device root
    device.reset(new String(path.slice(0, 2)));
    rootEnd = 2;
    if (len > 2 && isPathSeparator(path.charCodeAt(2))) {
      // Treat separator following drive name as an absolute path
      // indicator
      isAbsolute = true;
      rootEnd = 3;
    }
  }

  String tail = rootEnd < len ?
    normalizeString(path.slice(rootEnd), !isAbsolute, L"\\", isPathSeparator) :
    L"";
  if (tail.length() == 0 && !isAbsolute)
    tail = L".";
  if (tail.length() > 0 && isPathSeparator(path.charCodeAt(len - 1)))
    tail += L"\\";
  if (device == nullptr) {
    return isAbsolute ? (L"\\" + tail) : tail;
  }
  return isAbsolute ? (*device + L"\\" + tail) : (*device + tail);
}

namespace {
String win32InternalJoin(const std::vector<String>& args) {
  std::unique_ptr<String> joined;
  String firstPart;
  for (size_t i = 0; i < args.size(); ++i) {
    const String& arg = args[i];
    if (arg.length() > 0) {
      if (joined == nullptr) {
        firstPart = arg;
        joined.reset(new String(arg));
      } else {
        *joined += (L"\\" + arg);
      }
    }
  }

  if (joined == nullptr)
    return L".";

  // Make sure that the joined path doesn't start with two slashes, because
  // normalize() will mistake it for a UNC path then.
  //
  // This step is skipped when it is very clear that the user actually
  // intended to point at a UNC path. This is assumed when the first
  // non-empty string arguments starts with exactly two slashes followed by
  // at least one more non-slash character.
  //
  // Note that for normalize() to treat a path as a UNC path it needs to
  // have at least 2 components, so we don't filter for that here.
  // This means that the user can use join to construct UNC paths from
  // a server name and a share name; for example:
  //   path.join('//server', 'share') -> '\\\\server\\share\\')
  bool needsReplace = true;
  size_t slashCount = 0;
  if (isPathSeparator(firstPart.charCodeAt(0))) {
    ++slashCount;
    size_t firstLen = firstPart.length();
    if (firstLen > 1 && isPathSeparator(firstPart.charCodeAt(1))) {
      ++slashCount;
      if (firstLen > 2) {
        if (isPathSeparator(firstPart.charCodeAt(2)))
          ++slashCount;
        else {
          // We matched a UNC path in the first part
          needsReplace = false;
        }
      }
    }
  }
  if (needsReplace) {
    // Find any more consecutive slashes we need to replace
    while (slashCount < joined->length() &&
            isPathSeparator(joined->charCodeAt(slashCount))) {
      slashCount++;
    }

    // Replace the slashes if needed
    if (slashCount >= 2)
      joined.reset(new String(L"\\" + joined->slice((int)slashCount)));
  }

  return win32::normalize(*joined);
}
}

String join() { return L"."; }
String join(const String& arg1) {
  std::vector<String> args = { arg1 };
  return win32InternalJoin(args);
}
String join(const String& arg1, const String& arg2) {
  std::vector<String> args = { arg1, arg2 };
  return win32InternalJoin(args);
}

// It will solve the relative path from `from` to `to`, for instance:
//  from = 'C:\\orandea\\test\\aaa'
//  to = 'C:\\orandea\\impl\\bbb'
// The output of the function should be: '..\\..\\impl\\bbb'
String relative(const String& f, const String& t) {
  if (f == t)
    return L"";

  String fromOrig = win32::resolve(f);
  String toOrig = win32::resolve(t);

  if (fromOrig == toOrig)
    return L"";

  String from = fromOrig.toLowerCase();
  String to = toOrig.toLowerCase();

  if (from == to)
    return L"";

  // Trim any leading backslashes
  int fromStart = 0;
  while (fromStart < (int)from.length() &&
          from.charCodeAt(fromStart) == CHAR_BACKWARD_SLASH) {
    fromStart++;
  }
  // Trim trailing backslashes (applicable to UNC paths only)
  int fromEnd = (int)from.length();
  while (fromEnd - 1 > fromStart &&
          from.charCodeAt(fromEnd - 1) == CHAR_BACKWARD_SLASH) {
    fromEnd--;
  }
  int fromLen = fromEnd - fromStart;

  // Trim any leading backslashes
  int toStart = 0;
  while (toStart < (int)to.length() &&
          to.charCodeAt(toStart) == CHAR_BACKWARD_SLASH) {
    toStart++;
  }
  // Trim trailing backslashes (applicable to UNC paths only)
  int toEnd = (int)to.length();
  while (toEnd - 1 > toStart &&
          to.charCodeAt(toEnd - 1) == CHAR_BACKWARD_SLASH) {
    toEnd--;
  }
  int toLen = toEnd - toStart;

  // Compare paths to find the longest common path from root
  int length = fromLen < toLen ? fromLen : toLen;
  int lastCommonSep = -1;
  int i = 0;
  for (; i < length; i++) {
    uint16_t fromCode = from.charCodeAt(fromStart + i);
    if (fromCode != to.charCodeAt(toStart + i))
      break;
    else if (fromCode == CHAR_BACKWARD_SLASH)
      lastCommonSep = i;
  }

  // We found a mismatch before the first common path separator was seen, so
  // return the original `to`.
  if (i != length) {
    if (lastCommonSep == -1)
      return toOrig;
  } else {
    if (toLen > length) {
      if (to.charCodeAt(toStart + i) == CHAR_BACKWARD_SLASH) {
        // We get here if `from` is the exact base path for `to`.
        // For example: from='C:\\foo\\bar'; to='C:\\foo\\bar\\baz'
        return toOrig.slice(toStart + i + 1);
      }
      if (i == 2) {
        // We get here if `from` is the device root.
        // For example: from='C:\\'; to='C:\\foo'
        return toOrig.slice(toStart + i);
      }
    }
    if (fromLen > length) {
      if (from.charCodeAt(fromStart + i) == CHAR_BACKWARD_SLASH) {
        // We get here if `to` is the exact base path for `from`.
        // For example: from='C:\\foo\\bar'; to='C:\\foo'
        lastCommonSep = i;
      } else if (i == 2) {
        // We get here if `to` is the device root.
        // For example: from='C:\\foo\\bar'; to='C:\\'
        lastCommonSep = 3;
      }
    }
    if (lastCommonSep == -1)
      lastCommonSep = 0;
  }

  String out;
  // Generate the relative path based on the path difference between `to` and
  // `from`
  for (i = fromStart + lastCommonSep + 1; i <= fromEnd; ++i) {
    if (i == fromEnd || from.charCodeAt(i) == CHAR_BACKWARD_SLASH) {
      out += out.length() == 0 ? L".." : L"\\..";
    }
  }

  toStart += lastCommonSep;

  // Lastly, append the rest of the destination (`to`) path that comes after
  // the common path parts
  if (out.length() > 0)
    return out + toOrig.slice(toStart, toEnd);

  if (toOrig.charCodeAt(toStart) == CHAR_BACKWARD_SLASH)
    ++toStart;
  return toOrig.slice(toStart, toEnd);
}

String toNamespacedPath(const String& path) {
  // Note: this will *probably* throw somewhere.
  if (path.length() == 0) {
    return L"";
  }

  String resolvedPath = win32::resolve(path);

  if (resolvedPath.length() <= 2)
    return path;

  if (resolvedPath.charCodeAt(0) == CHAR_BACKWARD_SLASH) {
    // Possible UNC root
    if (resolvedPath.charCodeAt(1) == CHAR_BACKWARD_SLASH) {
      uint16_t code = resolvedPath.charCodeAt(2);
      if (code != CHAR_QUESTION_MARK && code != CHAR_DOT) {
        // Matched non-long UNC root, convert the path to a long UNC path
        return L"\\\\?\\UNC\\" + resolvedPath.slice(2);
      }
    }
  } else if (isWindowsDeviceRoot(resolvedPath.charCodeAt(0)) &&
              resolvedPath.charCodeAt(1) == CHAR_COLON &&
              resolvedPath.charCodeAt(2) == CHAR_BACKWARD_SLASH) {
    // Matched device root, convert the path to a long UNC path
    return L"\\\\?\\" + resolvedPath;
  }

  return path;
}

String dirname(const String& path) {
  int len = (int)path.length();
  if (len == 0)
    return L".";
  int rootEnd = -1;
  int offset = 0;
  uint16_t code = path.charCodeAt(0);

  if (len == 1) {
    // `path` contains just a path separator, exit early to avoid
    // unnecessary work or a dot.
    return isPathSeparator(code) ? path : '.';
  }

  // Try to match a root
  if (isPathSeparator(code)) {
    // Possible UNC root

    rootEnd = offset = 1;

    if (isPathSeparator(path.charCodeAt(1))) {
      // Matched double path separator at beginning
      int j = 2;
      int last = j;
      // Match 1 or more non-path separators
      while (j < len && !isPathSeparator(path.charCodeAt(j))) {
        j++;
      }
      if (j < len && j != last) {
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
          if (j == len) {
            // We matched a UNC root only
            return path;
          }
          if (j != last) {
            // We matched a UNC root with leftovers

            // Offset by 1 to include the separator after the UNC root to
            // treat it as a "normal root" on top of a (UNC) root
            rootEnd = offset = j + 1;
          }
        }
      }
    }
  // Possible device root
  } else if (isWindowsDeviceRoot(code) && path.charCodeAt(1) == CHAR_COLON) {
    rootEnd = len > 2 && isPathSeparator(path.charCodeAt(2)) ? 3 : 2;
    offset = rootEnd;
  }

  int end = -1;
  bool matchedSlash = true;
  for (int i = len - 1; i >= offset; --i) {
    if (isPathSeparator(path.charCodeAt(i))) {
      if (!matchedSlash) {
        end = i;
        break;
      }
    } else {
      // We saw the first non-path separator
      matchedSlash = false;
    }
  }

  if (end == -1) {
    if (rootEnd == -1)
      return L".";

    end = rootEnd;
  }
  return path.slice(0, end);
}

String basename(const String& path, const String& ext) {
  int start = 0;
  int end = -1;
  bool matchedSlash = true;

  // Check for a drive letter prefix so as not to mistake the following
  // path separator as an extra separator at the end of the path that can be
  // disregarded
  if (path.length() >= 2 &&
      isWindowsDeviceRoot(path.charCodeAt(0)) &&
      path.charCodeAt(1) == CHAR_COLON) {
    start = 2;
  }

  if (ext.length() > 0 && ext.length() <= path.length()) {
    if (ext == path)
      return L"";
    int extIdx = (int)ext.length() - 1;
    int firstNonSlashEnd = -1;
    for (int i = (int)path.length() - 1; i >= start; --i) {
      uint16_t code = path.charCodeAt(i);
      if (isPathSeparator(code)) {
        // If we reached a path separator that was not part of a set of path
        // separators at the end of the string, stop now
        if (!matchedSlash) {
          start = i + 1;
          break;
        }
      } else {
        if (firstNonSlashEnd == -1) {
          // We saw the first non-path separator, remember this index in case
          // we need it if the extension ends up not matching
          matchedSlash = false;
          firstNonSlashEnd = i + 1;
        }
        if (extIdx >= 0) {
          // Try to match the explicit extension
          if (code == ext.charCodeAt(extIdx)) {
            extIdx--;
            if (extIdx == -1) {
              // We matched the extension, so mark this as the end of our path
              // component
              end = i;
            }
          } else {
            // Extension does not match, so our result is the entire path
            // component
            extIdx = -1;
            end = firstNonSlashEnd;
          }
        }
      }
    }

    if (start == end)
      end = firstNonSlashEnd;
    else if (end == -1)
      end = (int)path.length();
    return path.slice(start, end);
  }
  for (int i = (int)path.length() - 1; i >= start; --i) {
    if (isPathSeparator(path.charCodeAt(i))) {
      // If we reached a path separator that was not part of a set of path
      // separators at the end of the string, stop now
      if (!matchedSlash) {
        start = i + 1;
        break;
      }
    } else if (end == -1) {
      // We saw the first non-path separator, mark this as the end of our
      // path component
      matchedSlash = false;
      end = i + 1;
    }
  }

  if (end == -1)
    return L"";
  return path.slice(start, end);
}

String extname(const String& path) {
  int start = 0;
  int startDot = -1;
  int startPart = 0;
  int end = -1;
  bool matchedSlash = true;
  // Track the state of characters (if any) we see before our first dot and
  // after any path separator we find
  int preDotState = 0;

  // Check for a drive letter prefix so as not to mistake the following
  // path separator as an extra separator at the end of the path that can be
  // disregarded

  if (path.length() >= 2 &&
      path.charCodeAt(1) == CHAR_COLON &&
      isWindowsDeviceRoot(path.charCodeAt(0))) {
    start = startPart = 2;
  }

  for (int i = (int)path.length() - 1; i >= start; --i) {
    uint16_t code = path.charCodeAt(i);
    if (isPathSeparator(code)) {
      // If we reached a path separator that was not part of a set of path
      // separators at the end of the string, stop now
      if (!matchedSlash) {
        startPart = i + 1;
        break;
      }
      continue;
    }
    if (end == -1) {
      // We saw the first non-path separator, mark this as the end of our
      // extension
      matchedSlash = false;
      end = i + 1;
    }
    if (code == CHAR_DOT) {
      // If this is our first dot, mark it as the start of our extension
      if (startDot == -1)
        startDot = i;
      else if (preDotState != 1)
        preDotState = 1;
    } else if (startDot != -1) {
      // We saw a non-dot and non-path separator before our dot, so we should
      // have a good chance at having a non-empty extension
      preDotState = -1;
    }
  }

  if (startDot == -1 ||
      end == -1 ||
      // We saw a non-dot character immediately before the dot
      preDotState == 0 ||
      // The (right-most) trimmed path component is exactly '..'
      (preDotState == 1 &&
        startDot == end - 1 &&
        startDot == startPart + 1)) {
    return L"";
  }
  return path.slice(startDot, end);
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
  resolvedPath = normalizeString(resolvedPath, !resolvedAbsolute, L"/",
                                  isPosixPathSeparator);

  if (resolvedAbsolute) {
    return L"/" + resolvedPath;
  }
  return resolvedPath.length() > 0 ? resolvedPath : L".";
}

String normalize(const String& p) {
  String path = p;
  if (path.length() == 0)
    return L".";

  bool isAbsolute = path.charCodeAt(0) == CHAR_FORWARD_SLASH;
  bool trailingSeparator =
    path.charCodeAt(path.length() - 1) == CHAR_FORWARD_SLASH;

  // Normalize the path
  path = normalizeString(path, !isAbsolute, L"/", isPosixPathSeparator);

  if (path.length() == 0) {
    if (isAbsolute)
      return L"/";
    return trailingSeparator ? L"./" : L".";
  }
  if (trailingSeparator)
    path += L"/";

  return isAbsolute ? (L"/" + path) : path;
}

namespace {
String posixInternalJoin(const std::vector<String>& args) {
  std::unique_ptr<String> joined;
  for (size_t i = 0; i < args.size(); ++i) {
    const String& arg = args[i];
    if (arg.length() > 0) {
      if (joined == nullptr) {
        joined.reset(new String(arg));
      } else {
        *joined += (L"/" + arg);
      }
    }
  }
  if (joined == nullptr)
    return L".";
  return posix::normalize(*joined);
}
}

String join() { return L"."; }
String join(const String& arg1) {
  std::vector<String> args = { arg1 };
  return posixInternalJoin(args);
}
String join(const String& arg1, const String& arg2) {
  std::vector<String> args = { arg1, arg2 };
  return posixInternalJoin(args);
}

String relative(const String& f, const String& t) {
  if (f == t)
    return L"";

  // Trim leading forward slashes.
  String from = posix::resolve(f);
  String to = posix::resolve(t);

  if (from == to)
    return L"";

  int fromStart = 1;
  int fromEnd = (int)from.length();
  int fromLen = fromEnd - fromStart;
  int toStart = 1;
  int toLen = (int)to.length() - toStart;

  // Compare paths to find the longest common path from root
  int length = (fromLen < toLen ? fromLen : toLen);
  int lastCommonSep = -1;
  int i = 0;
  for (; i < length; i++) {
    uint16_t fromCode = from.charCodeAt(fromStart + i);
    if (fromCode != to.charCodeAt(toStart + i))
      break;
    else if (fromCode == CHAR_FORWARD_SLASH)
      lastCommonSep = i;
  }
  if (i == length) {
    if (toLen > length) {
      if (to.charCodeAt(toStart + i) == CHAR_FORWARD_SLASH) {
        // We get here if `from` is the exact base path for `to`.
        // For example: from='/foo/bar'; to='/foo/bar/baz'
        return to.slice(toStart + i + 1);
      }
      if (i == 0) {
        // We get here if `from` is the root
        // For example: from='/'; to='/foo'
        return to.slice(toStart + i);
      }
    } else if (fromLen > length) {
      if (from.charCodeAt(fromStart + i) == CHAR_FORWARD_SLASH) {
        // We get here if `to` is the exact base path for `from`.
        // For example: from='/foo/bar/baz'; to='/foo/bar'
        lastCommonSep = i;
      } else if (i == 0) {
        // We get here if `to` is the root.
        // For example: from='/foo/bar'; to='/'
        lastCommonSep = 0;
      }
    }
  }

  String out;
  // Generate the relative path based on the path difference between `to`
  // and `from`.
  for (i = fromStart + lastCommonSep + 1; i <= fromEnd; ++i) {
    if (i == fromEnd || from.charCodeAt(i) == CHAR_FORWARD_SLASH) {
      out += out.length() == 0 ? L".." : L"/..";
    }
  }

  // Lastly, append the rest of the destination (`to`) path that comes after
  // the common path parts.
  return out + to.slice(toStart + lastCommonSep);
}

String toNamespacedPath(const String& path) {
  return path;
}

String dirname(const String& path) {
  if (path.length() == 0)
    return L".";
  bool hasRoot = path.charCodeAt(0) == CHAR_FORWARD_SLASH;
  int end = -1;
  bool matchedSlash = true;
  for (int i = (int)path.length() - 1; i >= 1; --i) {
    if (path.charCodeAt(i) == CHAR_FORWARD_SLASH) {
      if (!matchedSlash) {
        end = i;
        break;
      }
    } else {
      // We saw the first non-path separator
      matchedSlash = false;
    }
  }

  if (end == -1)
    return hasRoot ? L"/" : L".";
  if (hasRoot && end == 1)
    return L"//";
  return path.slice(0, end);
}

String basename(const String& path, const String& ext) {
  int start = 0;
  int end = -1;
  bool matchedSlash = true;

  if (ext.length() > 0 && ext.length() <= path.length()) {
    if (ext == path)
      return L"";
    int extIdx = (int)ext.length() - 1;
    int firstNonSlashEnd = -1;
    for (int i = (int)path.length() - 1; i >= 0; --i) {
      uint16_t code = path.charCodeAt(i);
      if (code == CHAR_FORWARD_SLASH) {
        // If we reached a path separator that was not part of a set of path
        // separators at the end of the string, stop now
        if (!matchedSlash) {
          start = i + 1;
          break;
        }
      } else {
        if (firstNonSlashEnd == -1) {
          // We saw the first non-path separator, remember this index in case
          // we need it if the extension ends up not matching
          matchedSlash = false;
          firstNonSlashEnd = i + 1;
        }
        if (extIdx >= 0) {
          // Try to match the explicit extension
          if (code == ext.charCodeAt(extIdx)) {
            extIdx--;
            if (extIdx == -1) {
              // We matched the extension, so mark this as the end of our path
              // component
              end = i;
            }
          } else {
            // Extension does not match, so our result is the entire path
            // component
            extIdx = -1;
            end = firstNonSlashEnd;
          }
        }
      }
    }

    if (start == end)
      end = firstNonSlashEnd;
    else if (end == -1)
      end = (int)path.length();
    return path.slice(start, end);
  }
  for (int i = (int)path.length() - 1; i >= 0; --i) {
    if (path.charCodeAt(i) == CHAR_FORWARD_SLASH) {
      // If we reached a path separator that was not part of a set of path
      // separators at the end of the string, stop now
      if (!matchedSlash) {
        start = i + 1;
        break;
      }
    } else if (end == -1) {
      // We saw the first non-path separator, mark this as the end of our
      // path component
      matchedSlash = false;
      end = i + 1;
    }
  }

  if (end == -1)
    return L"";
  return path.slice(start, end);
}

String extname(const String& path) {
  int startDot = -1;
  int startPart = 0;
  int end = -1;
  bool matchedSlash = true;
  // Track the state of characters (if any) we see before our first dot and
  // after any path separator we find
  int preDotState = 0;
  for (int i = (int)path.length() - 1; i >= 0; --i) {
    uint16_t code = path.charCodeAt(i);
    if (code == CHAR_FORWARD_SLASH) {
      // If we reached a path separator that was not part of a set of path
      // separators at the end of the string, stop now
      if (!matchedSlash) {
        startPart = i + 1;
        break;
      }
      continue;
    }
    if (end == -1) {
      // We saw the first non-path separator, mark this as the end of our
      // extension
      matchedSlash = false;
      end = i + 1;
    }
    if (code == CHAR_DOT) {
      // If this is our first dot, mark it as the start of our extension
      if (startDot == -1)
        startDot = i;
      else if (preDotState != 1)
        preDotState = 1;
    } else if (startDot != -1) {
      // We saw a non-dot and non-path separator before our dot, so we should
      // have a good chance at having a non-empty extension
      preDotState = -1;
    }
  }

  if (startDot == -1 ||
      end == -1 ||
      // We saw a non-dot character immediately before the dot
      preDotState == 0 ||
      // The (right-most) trimmed path component is exactly '..'
      (preDotState == 1 &&
        startDot == end - 1 &&
        startDot == startPart + 1)) {
    return L"";
  }
  return path.slice(startDot, end);
}

}

}

}
