// ============================================================================
// path.h: Operations on file paths.
// ============================================================================

#ifndef SPARK_SUPPORT_PATH_H
#define SPARK_SUPPORT_PATH_H 1

#ifndef SPARK_COLLECTIONS_STRINGREF_H
  #include "spark/collections/stringref.h"
#endif

#if SPARK_HAVE_DIRENT_H
  #include <dirent.h>
#endif

#if SPARK_HAVE_SYS_STAT_H
  #include <sys/stat.h>
#endif

namespace spark {
namespace support {
using spark::collections::StringRef;

class PathIterator;

/** Filesystem path. */
class Path {
  friend class PathIterator;
public:
  enum Status {
    ST_UNSET,   // No stat call has been done
    ST_OK,      // Stat is OK
    ST_NOENT,   // No such file
    ST_ERROR    // Other error
  };
public:
  Path(const StringRef& path) : _path(path.begin(), path.size()), _status(ST_UNSET) {}
  Path(const Path& src) : _path(src._path), _mode(src._mode), _status(src._status) {}
  Path(const Path&& src) : _path(std::move(src._path)), _mode(src._mode), _status(src._status) {}
  Path(const Path& parent, const StringRef& name)
    : _path(parent._path)
    , _mode(0)
    , _status(ST_UNSET)
  {
    _path.push_back('/');
    _path.append(name.begin(), name.end());
  }
  Path() : _status(ST_UNSET) {}
  
  Path& operator=(const Path& src) {
    _path = src._path;
    _mode = src._mode;
    _status = src._status;
    return *this;
  }
  
  Path& operator=(const Path&& src) {
    _path = std::move(src._path);
    _mode = src._mode;
    _status = src._status;
    return *this;
  }
  
  Path& operator=(const StringRef& str) {
    _path.assign(str.begin(), str.end());
    _mode = 0;
    _status = ST_UNSET;
    return *this;
  }
  
  bool exists();
  bool isDir();
  bool isFile();
  
  /** The value of this path as a StringRef. */
  StringRef str() const { return _path; }

  /** The value of this path as a null-terminated string. */
  const char* c_str() const { return _path.c_str(); }

  /** Return the length of this path. */
  size_t size() const { return _path.size(); }
  
  /** Return a substring of this path. */
  StringRef substr(int start, int end = 0x7fff) const {
    return StringRef(_path).substr(start, end);
  }

  /** Iterate over all entries in this directory. */
  PathIterator iterate() const;

  /** Make this path relative to a base path. Returns true if succeeded, false if this path
      was not a subdir of base. */
  bool makeRelative(const Path& base);
  
  /** Update index and size with the offset and length of the next path component. To access the
      first component, set index and size to 0 before calling this method. */
  bool nextComponent(size_t& index, size_t& size);

  /** Update index and size with the offset and length of the previous path component. To access
      the last component, set size to zero and index to the length of this path. */
  bool prevComponent(size_t& index, size_t& size);
private:
  void ensureStat();
  
  std::string _path;
  ::mode_t _mode;
  Status _status;
};

class PathIterator {
  friend class Path;
public:
  ~PathIterator();
  bool next(StringRef& path);
private:
  PathIterator(const Path& path);

  ::DIR* _dir;
};

// How to print a token type.
inline ::std::ostream& operator<<(::std::ostream& os, const Path& path) {
  StringRef str = path.str();
  os.write(str.begin(), str.size());
  return os;
}

}}

#endif
