// ============================================================================
// path.h: Operations on file paths.
// ============================================================================

#ifndef SPARK_SUPPORT_PATH_H
#define SPARK_SUPPORT_PATH_H 1

#ifndef SPARK_COLLECTIONS_STRINGREF_H
  #include "spark/collections/stringref.h"
#endif

#if SPARK_HAVE_VECTOR
  #include <vector>
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
  Path(const Path& parent, const StringRef& name);
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

  bool exists() const;
  bool isDir() const;
  bool isFile() const;

  /** Equality operator. */
  bool operator==(const Path& src) const {
    return StringRef(src._path) == StringRef(_path);
  }

  /** Equality operator (string). */
  bool operator==(const StringRef& str) const {
    return str == StringRef(_path);
  }

  /** The value of this path as a StringRef. */
  StringRef str() const { return _path; }

  /** The value of this path as a null-terminated string. */
  const char* c_str() const { return _path.c_str(); }

  /** Return the length of this path. */
  size_t size() const { return _path.size(); }

  /** Return whether this path is the empty string or not. */
  bool empty() const { return _path.empty(); }

  /** Return a substring of this path. */
  StringRef substr(int start, int end = 0x7fff) const {
    return StringRef(_path).substr(start, end);
  }

  /** Iterate over all entries in this directory. */
  PathIterator iterate() const;

  /** Return whether this is an absolute path. */
  bool isAbsolute() const;

  /** Make this path relative to a base path. Returns true if succeeded, false if this path
      was not a subdir of base. */
  bool makeRelative(const Path& base);

  /** Remove all /./ and /../ components from the path. Also removes trailing slash. */
  void normalize();

  /** Update index and size with the offset and length of the next path component. To access the
      first component, set index and size to 0 before calling this method. */
  bool nextComponent(size_t& index, size_t& size) const;

  /** Update index and size with the offset and length of the previous path component. To access
      the last component, set size to zero and index to the length of this path. */
  bool prevComponent(size_t& index, size_t& size) const;

  /** Return the last component of this path. Returns an empty string if the last component
      cannot be found. */
  StringRef name() const;

  /** Return the last component of this path without it's suffix. Returns an empty string if the
      last component cannot be found. */
  StringRef stem() const;

  /** Return a path containing all but the last component of this path. Returns an empty string if
      there is only one component. */
  Path parent() const;

  /** Return the components of this path as a list. */
  const std::vector<StringRef> parts() const;

  /** Return the suffix of this filename (including the dot). If there's more than one suffix,
      it only returns the last one. */
  StringRef suffix() const;

  /** Return a copy of this path with the suffix changed to the specified string. If there was
      no suffix originally, the new suffix will be appended. If the original path has several
      suffices, then only the last one will be changed. */
  Path withSuffix(StringRef suffix) const;

//   /** Path concatenation operator. If the path on the right is absolute, it simply returns that
//       path. Otherwise, joins the two paths with a '/' between them, and normalizes the result. */
//   friend const Path operator/(const Path& left, const Path& right);
//   friend const Path operator/(const Path& left, const StringRef& right) {
//     return left / Path(right);
//   }

  /** Returns a path representing the current directory. */
  static Path curdir();
private:
  void ensureStat() const;

  std::string _path;
  mutable ::mode_t _mode;
  mutable Status _status;
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
