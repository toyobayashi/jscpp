#include "jscpp/env_paths.hpp"
#include "jscpp/Process.hpp"
#include "jscpp/os.hpp"
#include "jscpp/path.hpp"

namespace js {

namespace env_paths {

namespace {
  Paths internalCreate(const String& name) {
    const std::map<String, String>& env = process.env;
    String homedir = os::homedir();
    String tmpdir = os::tmpdir();
    Paths paths;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    String appdata;
    String localAppdata;
    if (env.find(L"APPDATA") != env.end() && !(env.at(L"APPDATA") == L"")) {
      appdata = env.at(L"APPDATA");
    } else {
      appdata = path::win32::join(homedir, L"AppData", L"Roaming");
    }

    if (env.find(L"LOCALAPPDATA") != env.end() && !(env.at(L"LOCALAPPDATA") == L"")) {
      localAppdata = env.at(L"LOCALAPPDATA");
    } else {
      localAppdata = path::win32::join(homedir, L"AppData", L"Local");
    }

    paths.data = path::win32::join(localAppdata, name, L"Data");
    paths.config = path::win32::join(appdata, name, L"Config");
    paths.cache = path::win32::join(localAppdata, name, L"Cache");
    paths.log = path::win32::join(localAppdata, name, L"Log");
    paths.temp = path::win32::join(tmpdir, name);
#elif defined(__APPLE__)
    String library = path::posix::join(homedir, L"Library");

    paths.data = path::posix::join(library, L"Application Support", name);
    paths.config = path::posix::join(library, L"Preferences", name);
    paths.cache = path::posix::join(library, L"Caches", name);
    paths.log = path::posix::join(library, L"Logs", name);
    paths.temp = path::posix::join(tmpdir, name);
#elif defined(__linux__) || defined(__linux) || defined(__EMSCRIPTEN__)
    String username = path::posix::basename(homedir);

    paths.data = path::posix::join(
      (env.find(L"XDG_DATA_HOME") != env.end() && !(env.at(L"XDG_DATA_HOME") == L"")) ? env.at(L"XDG_DATA_HOME") : path::posix::join(homedir, L".local/share"),
      name);
    paths.config = path::posix::join(
      (env.find(L"XDG_CONFIG_HOME") != env.end() && !(env.at(L"XDG_CONFIG_HOME") == L"")) ? env.at(L"XDG_CONFIG_HOME") : path::posix::join(homedir, L".config"),
      name);
    paths.cache = path::posix::join(
      (env.find(L"XDG_CACHE_HOME") != env.end() && !(env.at(L"XDG_CACHE_HOME") == L"")) ? env.at(L"XDG_CACHE_HOME") : path::posix::join(homedir, L".cache"),
      name);
    paths.log = path::posix::join(
      (env.find(L"XDG_STATE_HOME") != env.end() && !(env.at(L"XDG_STATE_HOME") == L"")) ? env.at(L"XDG_STATE_HOME") : path::posix::join(homedir, L".local/state"),
      name);
    paths.temp = path::posix::join(tmpdir, username, name);
#endif
    return paths;
  }
}

Paths create(const String& name) {
  return internalCreate(name);
}

Paths create(const String& name, const Options& options) {
  String name_ = name;
  if (options.suffix.length() > 0) {
    name_ += (L"-" + options.suffix);
  }
  return internalCreate(name_);
}

}
}
