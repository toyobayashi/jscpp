#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
// #include <direct.h>
// #include <io.h>
#include <winioctl.h>
#include <bcrypt.h>
#endif

#include "jscpp/fs.hpp"
#include "jscpp/path.hpp"
#include "../internal/winerr.hpp"
#include "../internal/throw.hpp"
#include "jscpp/Error.hpp"
#include <cerrno>
#include <cstring>
#include <cstdlib>

#define JSCPP_FS_BUFFER_SIZE 128 * 1024

#ifndef _WIN32
#define JSCPP__PATH_MAX 8192
#endif

namespace js {
namespace fs {

namespace {

#ifdef _WIN32
static int file_symlink_usermode_flag = SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;

static const WCHAR LONG_PATH_PREFIX[] = L"\\\\?\\";
static const WCHAR LONG_PATH_PREFIX_LEN = 4;
static const WCHAR JUNCTION_PREFIX[] = L"\\??\\";
static const WCHAR JUNCTION_PREFIX_LEN = 4;

#define IS_SLASH(c) ((c) == L'\\' || (c) == L'/')
#define IS_LETTER(c) (((c) >= L'a' && (c) <= L'z') || \
  ((c) >= L'A' && (c) <= L'Z'))

#if defined(_MSC_VER) || defined(__MINGW64_VERSION_MAJOR)
  typedef struct _REPARSE_DATA_BUFFER {
    ULONG  ReparseTag;
    USHORT ReparseDataLength;
    USHORT Reserved;
    union {
      struct {
        USHORT SubstituteNameOffset;
        USHORT SubstituteNameLength;
        USHORT PrintNameOffset;
        USHORT PrintNameLength;
        ULONG Flags;
        WCHAR PathBuffer[1];
      } SymbolicLinkReparseBuffer;
      struct {
        USHORT SubstituteNameOffset;
        USHORT SubstituteNameLength;
        USHORT PrintNameOffset;
        USHORT PrintNameLength;
        WCHAR PathBuffer[1];
      } MountPointReparseBuffer;
      struct {
        UCHAR  DataBuffer[1];
      } GenericReparseBuffer;
    };
  } REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;
#endif

typedef struct _FILE_DISPOSITION_INFORMATION {
  BOOLEAN DeleteFile;
} FILE_DISPOSITION_INFORMATION, *PFILE_DISPOSITION_INFORMATION;

typedef struct _IO_STATUS_BLOCK {
  union {
    NTSTATUS Status;
    PVOID Pointer;
  };
  ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef struct _FILE_BASIC_INFORMATION {
  LARGE_INTEGER CreationTime;
  LARGE_INTEGER LastAccessTime;
  LARGE_INTEGER LastWriteTime;
  LARGE_INTEGER ChangeTime;
  DWORD FileAttributes;
} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;

typedef enum _FILE_INFORMATION_CLASS {
  FileDirectoryInformation = 1,
  FileFullDirectoryInformation,
  FileBothDirectoryInformation,
  FileBasicInformation,
  FileStandardInformation,
  FileInternalInformation,
  FileEaInformation,
  FileAccessInformation,
  FileNameInformation,
  FileRenameInformation,
  FileLinkInformation,
  FileNamesInformation,
  FileDispositionInformation,
  FilePositionInformation,
  FileFullEaInformation,
  FileModeInformation,
  FileAlignmentInformation,
  FileAllInformation,
  FileAllocationInformation,
  FileEndOfFileInformation,
  FileAlternateNameInformation,
  FileStreamInformation,
  FilePipeInformation,
  FilePipeLocalInformation,
  FilePipeRemoteInformation,
  FileMailslotQueryInformation,
  FileMailslotSetInformation,
  FileCompressionInformation,
  FileObjectIdInformation,
  FileCompletionInformation,
  FileMoveClusterInformation,
  FileQuotaInformation,
  FileReparsePointInformation,
  FileNetworkOpenInformation,
  FileAttributeTagInformation,
  FileTrackingInformation,
  FileIdBothDirectoryInformation,
  FileIdFullDirectoryInformation,
  FileValidDataLengthInformation,
  FileShortNameInformation,
  FileIoCompletionNotificationInformation,
  FileIoStatusBlockRangeInformation,
  FileIoPriorityHintInformation,
  FileSfioReserveInformation,
  FileSfioVolumeInformation,
  FileHardLinkInformation,
  FileProcessIdsUsingFileInformation,
  FileNormalizedNameInformation,
  FileNetworkPhysicalNameInformation,
  FileIdGlobalTxDirectoryInformation,
  FileIsRemoteDeviceInformation,
  FileAttributeCacheInformation,
  FileNumaNodeInformation,
  FileStandardLinkInformation,
  FileRemoteProtocolInformation,
  FileMaximumInformation
} FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;

extern "C"
ULONG
NTAPI
RtlNtStatusToDosError(NTSTATUS Status);

#ifndef NT_SUCCESS
# define NT_SUCCESS(status) (((NTSTATUS) (status)) >= 0)
#endif

extern "C"
NTSYSAPI 
NTSTATUS
NTAPI
NtSetInformationFile(
  IN HANDLE               FileHandle,
  OUT PIO_STATUS_BLOCK    IoStatusBlock,
  IN PVOID                FileInformation,
  IN ULONG                Length,
  IN FILE_INFORMATION_CLASS FileInformationClass );

#endif

#ifdef _WIN32
static int fs_wide_to_utf8(WCHAR* w_source_ptr,
                           DWORD w_source_len,
                           char** target_ptr,
                           uint64_t* target_len_ptr) {
  int r;
  int target_len;
  char* target;
  target_len = WideCharToMultiByte(CP_UTF8,
                                   0,
                                   w_source_ptr,
                                   w_source_len,
                                   NULL,
                                   0,
                                   NULL,
                                   NULL);

  if (target_len == 0) {
    return -1;
  }

  if (target_len_ptr != NULL) {
    *target_len_ptr = target_len;
  }

  if (target_ptr == NULL) {
    return 0;
  }

  target = (char*)malloc(target_len + 1);
  if (target == NULL) {
    SetLastError(ERROR_OUTOFMEMORY);
    return -1;
  }

  r = WideCharToMultiByte(CP_UTF8,
                          0,
                          w_source_ptr,
                          w_source_len,
                          target,
                          target_len,
                          NULL,
                          NULL);
  if (r != target_len) {
    return -1;
  }
  target[target_len] = '\0';
  *target_ptr = target;
  return 0;
}

static int fs_readlink_handle(HANDLE handle, char** target_ptr,
    uint64_t* target_len_ptr) {
  char buffer[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
  REPARSE_DATA_BUFFER* reparse_data = (REPARSE_DATA_BUFFER*) buffer;
  WCHAR* w_target;
  DWORD w_target_len;
  DWORD bytes;

  if (!DeviceIoControl(handle,
                       FSCTL_GET_REPARSE_POINT,
                       NULL,
                       0,
                       buffer,
                       sizeof buffer,
                       &bytes,
                       NULL)) {
    return -1;
  }

  if (reparse_data->ReparseTag == IO_REPARSE_TAG_SYMLINK) {
    /* Real symlink */
    w_target = reparse_data->SymbolicLinkReparseBuffer.PathBuffer +
        (reparse_data->SymbolicLinkReparseBuffer.SubstituteNameOffset /
        sizeof(WCHAR));
    w_target_len =
        reparse_data->SymbolicLinkReparseBuffer.SubstituteNameLength /
        sizeof(WCHAR);

    /* Real symlinks can contain pretty much everything, but the only thing we
     * really care about is undoing the implicit conversion to an NT namespaced
     * path that CreateSymbolicLink will perform on absolute paths. If the path
     * is win32-namespaced then the user must have explicitly made it so, and
     * we better just return the unmodified reparse data. */
    if (w_target_len >= 4 &&
        w_target[0] == L'\\' &&
        w_target[1] == L'?' &&
        w_target[2] == L'?' &&
        w_target[3] == L'\\') {
      /* Starts with \??\ */
      if (w_target_len >= 6 &&
          ((w_target[4] >= L'A' && w_target[4] <= L'Z') ||
           (w_target[4] >= L'a' && w_target[4] <= L'z')) &&
          w_target[5] == L':' &&
          (w_target_len == 6 || w_target[6] == L'\\')) {
        /* \??\<drive>:\ */
        w_target += 4;
        w_target_len -= 4;

      } else if (w_target_len >= 8 &&
                 (w_target[4] == L'U' || w_target[4] == L'u') &&
                 (w_target[5] == L'N' || w_target[5] == L'n') &&
                 (w_target[6] == L'C' || w_target[6] == L'c') &&
                 w_target[7] == L'\\') {
        /* \??\UNC\<server>\<share>\ - make sure the final path looks like
         * \\<server>\<share>\ */
        w_target += 6;
        w_target[0] = L'\\';
        w_target_len -= 6;
      }
    }

  } else if (reparse_data->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT) {
    /* Junction. */
    w_target = reparse_data->MountPointReparseBuffer.PathBuffer +
        (reparse_data->MountPointReparseBuffer.SubstituteNameOffset /
        sizeof(WCHAR));
    w_target_len = reparse_data->MountPointReparseBuffer.SubstituteNameLength /
        sizeof(WCHAR);

    /* Only treat junctions that look like \??\<drive>:\ as symlink. Junctions
     * can also be used as mount points, like \??\Volume{<guid>}, but that's
     * confusing for programs since they wouldn't be able to actually
     * understand such a path when returned by uv_readlink(). UNC paths are
     * never valid for junctions so we don't care about them. */
    if (!(w_target_len >= 6 &&
          w_target[0] == L'\\' &&
          w_target[1] == L'?' &&
          w_target[2] == L'?' &&
          w_target[3] == L'\\' &&
          ((w_target[4] >= L'A' && w_target[4] <= L'Z') ||
           (w_target[4] >= L'a' && w_target[4] <= L'z')) &&
          w_target[5] == L':' &&
          (w_target_len == 6 || w_target[6] == L'\\'))) {
      SetLastError(ERROR_SYMLINK_NOT_SUPPORTED);
      return -1;
    }

    /* Remove leading \??\ */
    w_target += 4;
    w_target_len -= 4;

  } else {
    /* Reparse tag does not indicate a symlink. */
    SetLastError(ERROR_SYMLINK_NOT_SUPPORTED);
    return -1;
  }

  return fs_wide_to_utf8(w_target, w_target_len, target_ptr, target_len_ptr);
}

static void fs_create_junction(const WCHAR* path, const WCHAR* new_path) {
  HANDLE handle = INVALID_HANDLE_VALUE;
  REPARSE_DATA_BUFFER *buffer = NULL;
  int created = 0;
  int target_len;
  int is_absolute, is_long_path;
  int needed_buf_size, used_buf_size, used_data_size, path_buf_len;
  int start, len, i;
  int add_slash;
  DWORD bytes;
  WCHAR* path_buf;

  target_len = (int)wcslen(path);
  is_long_path = wcsncmp(path, LONG_PATH_PREFIX, LONG_PATH_PREFIX_LEN) == 0;

  if (is_long_path) {
    is_absolute = 1;
  } else {
    is_absolute = target_len >= 3 && IS_LETTER(path[0]) &&
      path[1] == L':' && IS_SLASH(path[2]);
  }

  if (!is_absolute) {
    /* Not supporting relative paths */
    internal::throwError(L"Not supporting relative paths.");
  }

  /* Do a pessimistic calculation of the required buffer size */
  needed_buf_size =
      FIELD_OFFSET(REPARSE_DATA_BUFFER, MountPointReparseBuffer.PathBuffer) +
      JUNCTION_PREFIX_LEN * sizeof(WCHAR) +
      2 * (target_len + 2) * sizeof(WCHAR);

  /* Allocate the buffer */
  buffer = (REPARSE_DATA_BUFFER*)malloc(needed_buf_size);
  if (!buffer) {
    internal::throwError(L"Out of memory.");
  }

  /* Grab a pointer to the part of the buffer where filenames go */
  path_buf = (WCHAR*)&(buffer->MountPointReparseBuffer.PathBuffer);
  path_buf_len = 0;

  /* Copy the substitute (internal) target path */
  start = path_buf_len;

  wcsncpy((WCHAR*)&path_buf[path_buf_len], JUNCTION_PREFIX,
    JUNCTION_PREFIX_LEN);
  path_buf_len += JUNCTION_PREFIX_LEN;

  add_slash = 0;
  for (i = is_long_path ? LONG_PATH_PREFIX_LEN : 0; path[i] != L'\0'; i++) {
    if (IS_SLASH(path[i])) {
      add_slash = 1;
      continue;
    }

    if (add_slash) {
      path_buf[path_buf_len++] = L'\\';
      add_slash = 0;
    }

    path_buf[path_buf_len++] = path[i];
  }
  path_buf[path_buf_len++] = L'\\';
  len = path_buf_len - start;

  /* Set the info about the substitute name */
  buffer->MountPointReparseBuffer.SubstituteNameOffset = (USHORT)(start * sizeof(WCHAR));
  buffer->MountPointReparseBuffer.SubstituteNameLength = (USHORT)(len * sizeof(WCHAR));

  /* Insert null terminator */
  path_buf[path_buf_len++] = L'\0';

  /* Copy the print name of the target path */
  start = path_buf_len;
  add_slash = 0;
  for (i = is_long_path ? LONG_PATH_PREFIX_LEN : 0; path[i] != L'\0'; i++) {
    if (IS_SLASH(path[i])) {
      add_slash = 1;
      continue;
    }

    if (add_slash) {
      path_buf[path_buf_len++] = L'\\';
      add_slash = 0;
    }

    path_buf[path_buf_len++] = path[i];
  }
  len = path_buf_len - start;
  if (len == 2) {
    path_buf[path_buf_len++] = L'\\';
    len++;
  }

  /* Set the info about the print name */
  buffer->MountPointReparseBuffer.PrintNameOffset = (USHORT)(start * sizeof(WCHAR));
  buffer->MountPointReparseBuffer.PrintNameLength = (USHORT)(len * sizeof(WCHAR));

  /* Insert another null terminator */
  path_buf[path_buf_len++] = L'\0';

  /* Calculate how much buffer space was actually used */
  used_buf_size = FIELD_OFFSET(REPARSE_DATA_BUFFER, MountPointReparseBuffer.PathBuffer) +
    path_buf_len * sizeof(WCHAR);
  used_data_size = used_buf_size -
    FIELD_OFFSET(REPARSE_DATA_BUFFER, MountPointReparseBuffer);

  /* Put general info in the data buffer */
  buffer->ReparseTag = IO_REPARSE_TAG_MOUNT_POINT;
  buffer->ReparseDataLength = used_data_size;
  buffer->Reserved = 0;

  /* Create a new directory */
  if (!CreateDirectoryW(new_path, NULL)) {
    goto error;
  }
  created = 1;

  /* Open the directory */
  handle = CreateFileW(new_path,
                       GENERIC_WRITE,
                       0,
                       NULL,
                       OPEN_EXISTING,
                       FILE_FLAG_BACKUP_SEMANTICS |
                         FILE_FLAG_OPEN_REPARSE_POINT,
                       NULL);
  if (handle == INVALID_HANDLE_VALUE) {
    goto error;
  }

  /* Create the actual reparse point */
  if (!DeviceIoControl(handle,
                       FSCTL_SET_REPARSE_POINT,
                       buffer,
                       used_buf_size,
                       NULL,
                       0,
                       &bytes,
                       NULL)) {
    goto error;
  }

  /* Clean up */
  CloseHandle(handle);
  free(buffer);

  return;

error:
  free(buffer);

  if (handle != INVALID_HANDLE_VALUE) {
    CloseHandle(handle);
  }

  if (created) {
    RemoveDirectoryW(new_path);
  }

  internal::throwError(internal::getWinErrorMessage(GetLastError()));
}
#endif

}

namespace {
  int internalAccess(const String& p, int mode) {
    String path = path::normalize(p);
#ifdef _WIN32

    DWORD attr = GetFileAttributesW(path.data());

    if (attr == INVALID_FILE_ATTRIBUTES) {
      return GetLastError();
    }

    /*
    * Access is possible if
    * - write access wasn't requested,
    * - or the file isn't read-only,
    * - or it's a directory.
    * (Directories cannot be read-only on Windows.)
    */
    if (!(mode & fs::AccessType::AT_WOK) ||
        !(attr & FILE_ATTRIBUTE_READONLY) ||
        (attr & FILE_ATTRIBUTE_DIRECTORY)) {
      return 0;
    } else {
      return EPERM;
    }
#else
    if (::access(path.str().c_str(), mode) != 0) {
      return errno;
    }
#endif
    return 0;
  }
}

void access(const String& p, int mode) {
  int r = internalAccess(p, mode);
#ifdef _WIN32
  if (r != 0) {
    internal::throwError(internal::getWinErrorMessage(r) + L", access \"" + p + L"\"");
  }
#else
  if (r != 0) {
    internal::throwError(String(strerror(r)) + L", access \"" + p + L"\"");
  }
#endif
}

void chmod(const String& p, int mode) {
  String path = path::normalize(p);
#ifdef _WIN32
  if (::_wchmod(path.data(), mode) != 0) {
    internal::throwError(String(strerror(errno)) + L", chmod \"" + p + L"\"");
  }
#else
  if (::chmod(path.str().c_str(), mode) != 0) {
    internal::throwError(String(strerror(errno)) + L", chmod \"" + p + L"\"");
  }
#endif
}

bool exists(const String& p) {
// #if 0
//   try {
//     fs::access(p, fs::AccessType::AT_FOK);
//     return true;
//   } catch (const Error&) {
//     try {
//       fs::lstat(p);
//       return true;
//     } catch (const Error&) {
//       return false;
//     }
//   }
// #else
  if (internalAccess(p, fs::AccessType::AT_FOK) == 0) {
    return true;
  }
  fs::Stats stats;
  if (fs::Stats::createNoThrow(stats, p, false) == 0) {
    return true;
  }
  return false;
// #endif
}

void mkdir(const String& p, int mode) {
  int code = 0;
  String path = path::normalize(p);
#ifdef _WIN32
  code = _wmkdir(path.data());
#else
  code = ::mkdir(path.str().c_str(), mode);
#endif
  if (code != 0) {
    internal::throwError(String(strerror(errno)) + L", mkdir \"" + p + L"\"");
  }
}

void mkdirs(const String& p, int mode) {
  if (fs::exists(p)) {
    if (fs::lstat(p).isDirectory()) {
      return;
    } else {
      internal::throwError(String(strerror(EEXIST)) + L", mkdir \"" + p + L"\"");
    }
  }

  String dir = path::dirname(p);

  if (!fs::exists(dir)) {
    fs::mkdirs(dir);
  }

  if (fs::lstat(dir).isDirectory()) {
    fs::mkdir(p, mode);
  } else {
    internal::throwError(String(strerror(ENOENT)) + L", mkdir \"" + p + L"\"");
  }
}

void unlink(const String& p) {
  int code = 0;
  String path = path::normalize(p);
#ifdef _WIN32
  // code = _wunlink(toyo::charset::a2w(path).c_str());

  const WCHAR* pathw = path.data();
  HANDLE handle;
  BY_HANDLE_FILE_INFORMATION info;
  FILE_DISPOSITION_INFORMATION disposition;
  IO_STATUS_BLOCK iosb;
  NTSTATUS status;

  handle = CreateFileW(pathw,
                       FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | DELETE,
                       FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                       NULL,
                       OPEN_EXISTING,
                       FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
                       NULL);

  if (handle == INVALID_HANDLE_VALUE) {
    internal::throwError(internal::getWinErrorMessage(GetLastError()) + L", unlink \"" + p + L"\"" );
  }

  if (!GetFileInformationByHandle(handle, &info)) {
    CloseHandle(handle);
    internal::throwError(internal::getWinErrorMessage(GetLastError()) + L", unlink \"" + p + L"\"" );
  }

  if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
    /* Do not allow deletion of directories, unless it is a symlink. When the
     * path refers to a non-symlink directory, report EPERM as mandated by
     * POSIX.1. */

    /* Check if it is a reparse point. If it's not, it's a normal directory. */
    if (!(info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
      SetLastError(ERROR_ACCESS_DENIED);
      CloseHandle(handle);
      internal::throwError(internal::getWinErrorMessage(GetLastError()) + L", unlink \"" + p + L"\"" );
    }

    /* Read the reparse point and check if it is a valid symlink. If not, don't
     * unlink. */
    if (fs_readlink_handle(handle, NULL, NULL) < 0) {
      DWORD error = GetLastError();
      if (error == ERROR_SYMLINK_NOT_SUPPORTED)
        error = ERROR_ACCESS_DENIED;
      SetLastError(error);
      CloseHandle(handle);
      internal::throwError(internal::getWinErrorMessage(GetLastError()) + L", unlink \"" + p + L"\"" );
    }
  }

  if (info.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
    /* Remove read-only attribute */
    FILE_BASIC_INFORMATION basic = { 0 };

    basic.FileAttributes = (info.dwFileAttributes & ~FILE_ATTRIBUTE_READONLY) |
                           FILE_ATTRIBUTE_ARCHIVE;

    status = NtSetInformationFile(handle,
                                   &iosb,
                                   &basic,
                                   sizeof basic,
                                   FileBasicInformation);
    if (!NT_SUCCESS(status)) {
      SetLastError(RtlNtStatusToDosError(status));
      CloseHandle(handle);
      internal::throwError(internal::getWinErrorMessage(GetLastError()) + L", unlink \"" + p + L"\"" );
    }
  }

  /* Try to set the delete flag. */
  disposition.DeleteFile = TRUE;
  status = NtSetInformationFile(handle,
                                 &iosb,
                                 &disposition,
                                 sizeof disposition,
                                 FileDispositionInformation);
  if (NT_SUCCESS(status)) {
    CloseHandle(handle);
    return;
  } else {
    CloseHandle(handle);
    SetLastError(RtlNtStatusToDosError(status));
    internal::throwError(internal::getWinErrorMessage(GetLastError()) + L", unlink \"" + p + L"\"" );
  }
#else
  code = ::unlink(path.str().c_str());
#endif
  if (code != 0) {
    internal::throwError(String(strerror(errno)) + L", unlink \"" + p + L"\"");
  }
}

void rmdir(const String& p) {
  int code = 0;
  String path = path::normalize(p);
#ifdef _WIN32
  code = _wrmdir(path.data());
#else
  code = ::rmdir(path.str().c_str());
#endif
  if (code != 0) {
    internal::throwError(String(strerror(errno)) + L", rmdir \"" + p + L"\"");
  }
}

void rename(const String& s, const String& d) {
  int code = 0;
  String source = path::normalize(s);
  String dest = path::normalize(d);
#ifdef _WIN32
  code = _wrename(source.data(), dest.data());
#else
  code = ::rename(source.str().c_str(), dest.str().c_str());
#endif
  if (code != 0) {
    internal::throwError(String(strerror(errno)) + L", rename \"" + s + L"\" -> \"" + d + L"\"");
  }
}

void remove(const String& p) {
  if (!fs::exists(p)) {
    return;
  }

  fs::Stats stat = fs::lstat(p);
  if (stat.isDirectory()) {
    std::vector<String> items = fs::readdir(p);
    if (items.size() != 0) {
      for (size_t i = 0; i < items.size(); i++) {
        const String& item = items[i];
        fs::remove(path::join(p, item));
      }
      fs::rmdir(p);
    } else {
      fs::rmdir(p);
    }
  } else {
    fs::unlink(p);
  }
}

void symlink(const String& o, const String& n) {
  String oldpath = path::normalize(o);
  String newpath = path::normalize(n);
#ifdef _WIN32
  if (!fs::exists(oldpath)) {
    fs::symlink(o, n, ST_FILE);
  } else if (fs::lstat(oldpath).isDirectory()) {
    fs::symlink(o, n, ST_DIRECTORY);
  } else {
    fs::symlink(o, n, ST_FILE);
  }
#else
  int code = ::symlink(oldpath.str().c_str(), newpath.str().c_str());
  if (code != 0) {
    internal::throwError(String(strerror(errno)) + L", symlink \"" + o + L"\" -> \"" + n + L"\"");
  }
#endif
}

void symlink(const String& o, const String& n, SymlinkType type) {
  String oldpath = path::normalize(o);
  String newpath = path::normalize(n);
#ifdef _WIN32
  int flags = 0;
  if (type == ST_DIRECTORY) {
    flags = file_symlink_usermode_flag | SYMBOLIC_LINK_FLAG_DIRECTORY;
    if (!CreateSymbolicLinkW(newpath.data(), oldpath.data(), flags)) {
      int err = GetLastError();
      if (err == ERROR_INVALID_PARAMETER && (flags & SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE))  {
        file_symlink_usermode_flag = 0;
        fs::symlink(o, n, type);
      } else {
        internal::throwError(String(internal::getWinErrorMessage(GetLastError())) + L", symlink \"" + o + L"\" -> \"" + n + L"\"");
      }
    }
  } else if (type == ST_FILE) {
    flags = file_symlink_usermode_flag;
    if (!CreateSymbolicLinkW(newpath.data(), oldpath.data(), flags)) {
      int err = GetLastError();
      if (err == ERROR_INVALID_PARAMETER && (flags & SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE))  {
        file_symlink_usermode_flag = 0;
        fs::symlink(o, n, type);
      } else {
        internal::throwError(String(internal::getWinErrorMessage(GetLastError())) + L", symlink \"" + o + L"\" -> \"" + n + L"\"");
      }
    }
  } else if (type == ST_JUNCTION) {
    oldpath = path::resolve(oldpath);
    fs_create_junction(oldpath.data(), newpath.data());
  } else {
    internal::throwError(String(L"Error symlink_type,") + L", symlink \"" + o + L"\" -> \"" + n + L"\"");
  }
#else
  int code = ::symlink(oldpath.str().c_str(), newpath.str().c_str());
  if (code != 0) {
    internal::throwError(String(strerror(errno)) + L", symlink \"" + o + L"\" -> \"" + n + L"\"");
  }
#endif
}

String realpath(const String& p) {
  String path = path::normalize(p);
#ifdef _WIN32
  HANDLE handle;

  handle = CreateFileW(path.data(),
                       0,
                       0,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS,
                       NULL);
  if (handle == INVALID_HANDLE_VALUE) {
    internal::throwError(String(internal::getWinErrorMessage(GetLastError())) + L", realpath \"" + p + L"\"");
  }

  DWORD w_realpath_len;
  WCHAR* w_realpath_ptr = NULL;
  WCHAR* w_realpath_buf;

  w_realpath_len = GetFinalPathNameByHandleW(handle, NULL, 0, VOLUME_NAME_DOS);
  if (w_realpath_len == 0) {
    internal::throwError(String(internal::getWinErrorMessage(GetLastError())) + L", realpath \"" + p + L"\"");
  }

  w_realpath_buf = (WCHAR*)malloc((w_realpath_len + 1) * sizeof(WCHAR));
  if (w_realpath_buf == NULL) {
    SetLastError(ERROR_OUTOFMEMORY);
    internal::throwError(String(internal::getWinErrorMessage(GetLastError())) + L", realpath \"" + p + L"\"");
  }
  w_realpath_ptr = w_realpath_buf;

  if (GetFinalPathNameByHandleW(
          handle, w_realpath_ptr, w_realpath_len, VOLUME_NAME_DOS) == 0) {
    free(w_realpath_buf);
    SetLastError(ERROR_INVALID_HANDLE);
    internal::throwError(String(internal::getWinErrorMessage(GetLastError())) + L", realpath \"" + p + L"\"");
  }

  /* convert UNC path to long path */
  if (wcsncmp(w_realpath_ptr,
              L"\\\\?\\UNC\\",
              8) == 0) {
    w_realpath_ptr += 6;
    *w_realpath_ptr = L'\\';
    w_realpath_len -= 6;
  } else if (wcsncmp(w_realpath_ptr,
                      LONG_PATH_PREFIX,
                      LONG_PATH_PREFIX_LEN) == 0) {
    w_realpath_ptr += 4;
    w_realpath_len -= 4;
  } else {
    free(w_realpath_buf);
    SetLastError(ERROR_INVALID_HANDLE);
    internal::throwError(String(internal::getWinErrorMessage(GetLastError())) + L", realpath \"" + p + L"\"");
  }

  String res(w_realpath_ptr);
  free(w_realpath_buf);
  CloseHandle(handle);
  return res;
#else
  char* buf = nullptr;

#if defined(_POSIX_VERSION) && _POSIX_VERSION >= 200809L
  buf = ::realpath(path.str().c_str(), nullptr);
  if (buf == nullptr)
    internal::throwError(String(strerror(errno)) + L", realpath \"" + p + L"\"");
  return buf;
#else
  ssize_t len;

  ssize_t pathmax;

  pathmax = pathconf(path.str().c_str(), _PC_PATH_MAX);

  if (pathmax == -1)
    pathmax = JSCPP__PATH_MAX;

  len =  pathmax;

  buf = (char*)malloc(len + 1);

  if (buf == nullptr) {
    errno = ENOMEM;
    internal::throwError(String(strerror(errno)) + L", realpath \"" + p + L"\"");
  }

  if (::realpath(path.str().c_str(), buf) == NULL) {
    free(buf);
    internal::throwError(String(strerror(errno)) + L", realpath \"" + p + L"\"");
  }
  String res = buf;
  free(buf);
  return res;
#endif
#endif
}

void copyFile(const String& s, const String& d, bool failIfExists) {
  String source = path::resolve(s);
  String dest = path::resolve(d);
  String errmessage = L"copy \"" + s + L"\" -> \"" + d + L"\"";

  if (source == dest) {
    return;
  }

#ifdef _WIN32
  if (!CopyFileW(source.data(), dest.data(), failIfExists)) {
    internal::throwError(String(internal::getWinErrorMessage(GetLastError())) + L", copy \"" + s + L"\" -> \"" + d + L"\"");
  }
#else

  if (failIfExists && fs::exists(dest)) {
    internal::throwError(String(strerror(EEXIST)) + L", " + errmessage);
  }

  int mode;
  fs::Stats stats;
  int r = fs::Stats::createNoThrow(stats, source, true);
  if (r != 0) {
    internal::throwError(String(strerror(r)) + L", " + errmessage);
  }
  mode = stats.mode;

  FILE* sf = ::fopen(source.str().c_str(), "rb+");
  if (!sf) {
    internal::throwError(String(strerror(errno)) + L", " + errmessage);
  }
  FILE* df = ::fopen(dest.str().c_str(), "wb+");
  if (!df) {
    ::fclose(sf);
    internal::throwError(String(strerror(errno)) + L", " + errmessage);
  }
  uint8_t buf[JSCPP_FS_BUFFER_SIZE];
  size_t read;
  while ((read = ::fread(buf, sizeof(uint8_t), JSCPP_FS_BUFFER_SIZE, sf)) > 0) {
    ::fwrite(buf, sizeof(uint8_t), read, df);
    ::fflush(df);
  }
  ::fclose(sf);
  ::fclose(df);

  fs::chmod(dest, mode);
#endif

}

void copy(const String& s, const String& d, bool failIfExists) {
  String source = path::resolve(s);
  String dest = path::resolve(d);

  if (source == dest) {
    return;
  }

  Stats stat = fs::lstat(source);

  if (stat.isDirectory()) {
    if (path::relative(s, d).indexOf(L"..") != 0) {
      internal::throwError(String(L"Cannot copy a directory into itself.") + L" copy \"" + s + L"\" -> \"" + d + L"\"");
    }
    fs::mkdirs(dest);
    std::vector<String> items = fs::readdir(source);
    for (size_t i = 0; i < items.size(); i++) {
      fs::copy(path::join(source, items[i]), path::join(dest, items[i]), failIfExists);
    }
  } else {
    fs::copyFile(source, dest, failIfExists);
  }
}

void move(const String& s, const String& d) {
  String source = path::resolve(s);
  String dest = path::resolve(d);

  if (source == dest) {
    return;
  }

  fs::copy(source, dest);
  fs::remove(source);
}

std::vector<uint8_t> readFile(const String& p) {
  String path = path::normalize(p);
  // if (!fs::exists(path)) {
  //   throw cerror(ENOENT, "open \"" + p + "\"");
  // }

  // fs::Stats stat = fs::statSync(path);
  // if (stat.isDirectory()) {
  //   throw std::runtime_error((String("EISDIR: illegal operation on a directory, read \"") + path + "\"").toCString());
  // }

  fs::Stats stat;
  int r = fs::Stats::createNoThrow(stat, path, false);
  if (r != 0) {
    internal::throwError(String(strerror(r)) + L", open \"" + p + L"\"");
  }

  if (stat.isDirectory()) {
    internal::throwError(String(strerror(EISDIR)) + L", read \"" + p + L"\"");
  }

  long size = stat.size;

#ifdef _WIN32
  FILE* fp = ::_wfopen(path.data(), L"rb");
#else
  FILE* fp = ::fopen(path.str().c_str(), "rb");
#endif
  if (!fp) {
    internal::throwError(String(strerror(errno)) + L", open \"" + p + L"\"");
  }

  std::vector<uint8_t> buf(size, 0);
  size_t read = ::fread(&buf[0], sizeof(uint8_t), size, fp);
  ::fclose(fp);
  if (read != size) {
    internal::throwError(String(strerror(errno)) + L", read \"" + p + L"\"");
  }
  return buf;
}

String readFileAsString(const String& p) {
  std::vector<uint8_t> buf = fs::readFile(p);
  return std::string(buf.begin(), buf.end());
}

namespace {
  void internalWriteFile(const String& p, const uint8_t* buf, size_t size, String mode) {
    if (fs::exists(p)) {
      if (fs::lstat(p).isDirectory()) {
        internal::throwError(String(strerror(errno)) + L", open \"" + p + L"\"");
      }
    }

    String path = path::normalize(p);

  #ifdef _WIN32
    FILE* fp = ::_wfopen(path.data(), mode.data());
  #else
    FILE* fp = ::fopen(path.str().c_str(), mode.str().c_str());
  #endif

    if (!fp) {
      internal::throwError(String(strerror(errno)) + L", open \"" + p + L"\"");
    }

    ::fwrite(buf, sizeof(uint8_t), size, fp);
    ::fflush(fp);
    ::fclose(fp);
  }
}

void writeFile(const String& p, const std::vector<uint8_t>& buf) {
  internalWriteFile(p, buf.data(), buf.size(), L"wb");
}

void writeFile(const String& p, const String& str) {
  std::string utf8 = str.str();
  internalWriteFile(p, (const uint8_t*)(utf8.data()), utf8.size(), L"wb");
}

void appendFile(const String& p, const std::vector<uint8_t>& buf) {
  internalWriteFile(p, buf.data(), buf.size(), L"ab");
}

void appendFile(const String& p, const String& str) {
  std::string utf8 = str.str();
  internalWriteFile(p, (const uint8_t*)(utf8.data()), utf8.size(), L"ab");
}

}
}
