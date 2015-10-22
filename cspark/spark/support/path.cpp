// ============================================================================
// Fileesystem paths - implementation.
// ============================================================================

#include "spark/support/path.h"

#if SPARK_HAVE_IOSTREAM
  #include <iostream>
#endif

#if SPARK_HAVE_UNISTD_H
  #include <unistd.h>
#endif

namespace spark {
namespace support {

Path::Path(const Path& parent, const StringRef& name)
  : _path(parent._path)
  , _mode(0)
  , _status(ST_UNSET)
{
  if (!name.empty() && name[0] == '/') {
    _path.clear();
  } else if (!_path.empty() && _path.back() != '/') {
    _path.push_back('/');
  }
  _path.append(name.begin(), name.end());
  normalize();
}

bool Path::exists() const {
  ensureStat();
  return _status == ST_OK;
}

bool Path::isDir() const {
  ensureStat();
  return _status == ST_OK && S_ISDIR(_mode);
}

bool Path::isFile() const {
  ensureStat();
  return _status == ST_OK && S_ISREG(_mode);
}

// bool Path::isReadable() {
//   return _status == ST_OK && S_ISDIR(_mode);
// }

void Path::ensureStat() const {
  if (_status == ST_UNSET) {
    struct ::stat st;
    if (::stat(_path.c_str(), &st) != 0) {
      _mode = 0;
      if (errno == ENOENT) {
        _status = ST_NOENT;
      } else {
        std::cerr << _path << ": " << ::strerror(errno) << "\n";
        _status = ST_ERROR;
      }
    } else {
      _status = ST_OK;
      _mode = st.st_mode;
    }
  }
}

PathIterator Path::iterate() const {
  return PathIterator(*this);
}

bool Path::isAbsolute() const {
  return !_path.empty() && _path.front() == '/';
}

bool Path::makeRelative(const Path& base) {
  if (base._path.size() >= _path.size()) {
    return false;
  }
  size_t split = base.size();
  while (split > 0 && base._path[split - 1] == '/') {
    --split;
  }
  if (base.substr(0, split) != substr(0, split)) {
    return false;
  }
  while (split < size() && _path[split] == '/') {
    ++split;
  }
  _path.erase(0, split);
  _status = ST_UNSET;
  return true;
}

void Path::normalize() {
  size_t index = 0;
  bool startComponent = true;
  while (index < _path.size()) {
    if (startComponent) {
      size_t end = index;
      while (end < _path.size() && _path[end] != '/') {
        ++end;
      }
      size_t size = end - index;
      if (end < _path.size() && _path[end] == '/') {
        ++end;
      }
      if (size == 1 && _path[index] == '.') {
        _path.erase(_path.begin() + index, _path.begin() + end);
      } else if (size == 2 && _path[index] == '.' && _path[index + 1] == '.') {
        size_t prev = index > 0 ? index - 1 : index;
        while (prev > 0 && _path[prev - 1] != '/') {
          --prev;
        }
        if (prev < index) {
          _path.erase(_path.begin() + prev, _path.begin() + end);
          index = prev;
        } else {
          index = end;
        }
      } else {
        index = end;
      }
    } else {
      startComponent = _path[index++] == '/';
    }
  }
  if (!_path.empty() && _path.back() == '/') {
    _path.pop_back();
  }
  _status = ST_UNSET;
}

bool Path::nextComponent(size_t& index, size_t& size) const {
  index += size;
  while (index < _path.size() && _path[index] == '/') {
    ++index;
  }
  size = 0;
  while (index + size < _path.size() && _path[index + size] != '/') {
    ++size;
  }
  return index < _path.size();
}

bool Path::prevComponent(size_t& index, size_t& size) const {
  while (index > 0 && _path[index - 1] == '/') {
    --index;
  }
  size_t end = index;
  while (index > 0 && _path[index - 1] != '/') {
    --index;
  }
  size = end - index;
  return size > 0;
}

StringRef Path::name() const {
  size_t start = _path.size();
  size_t size = 0;
  if (prevComponent(start, size)) {
    if (start + size == _path.size()) { // No trailing slash.
      return substr(start, start + size);
    }
  }
  return collections::StringRef();
}

StringRef Path::stem() const {
  size_t start = _path.size();
  size_t size = 0;
  // Look for last component.
  if (prevComponent(start, size)) {
    if (start + size == _path.size()) { // No trailing slash.
      // Look for last period
      size_t dot = size;
      while (dot-- > 0) {
        if (_path[start + dot] == '.') {
          size = dot;
          break;
        }
      }
      return substr(start, start + size);
    }
  }
  return collections::StringRef();
}

Path Path::parent() const {
  size_t start = _path.size();
  size_t size = 0;
  if (prevComponent(start, size)) {
    if (start > 0 && _path[start - 1] == '/') {
      --start;
    }
    return Path(substr(0, start));
  }
  return collections::StringRef();
}

const std::vector<StringRef> Path::parts() const {
  std::vector<StringRef> result;
  size_t start = 0;
  size_t size = 0;
  while (nextComponent(start, size)) {
    result.push_back(substr(start, start + size));
  }
  return result;
}

StringRef Path::suffix() const {
  size_t pos = _path.size();
  while (pos > 0) {
    --pos;
    if (_path[pos] == '.') {
      return StringRef(_path).substr(pos, _path.size());
    } else if (_path[pos] == '/') {
      break;
    }
  }
  return StringRef();
}

Path Path::withSuffix(StringRef suffix) const {
  if (suffix.size() > 0) {
    assert(suffix[0] == '.');
  }
  size_t pos = _path.size();
  while (pos > 0) {
    --pos;
    if (_path[pos] == '.') {
      break;
    } else if (_path[pos] == '/') {
      pos = _path.size();
      break;
    }
  }
  Path result(StringRef(_path).substr(0, pos));
  result._path.append(suffix.begin(), suffix.size());
  return result;
}

Path Path::curdir() {
  char* curDir = ::getcwd(NULL, 0);
  Path result(curDir);
  ::free(curDir);
  return result;
}

// const Path operator/(const Path& left, const Path& right) {
//   if (right.isAbsolute() || left._path.empty()) {
//     return right;
//   }
//   Path result(left);
//   if (result._path.back() != '/') {
//     result._path.push_back('/');
//   }
//   result._path.append(right._path);
//   result.normalize();
//   result._status = Path::ST_UNSET;
//   return result;
// }

PathIterator::PathIterator(const Path& path) {
  _dir = ::opendir(path._path.c_str());
  if (_dir == NULL) {
    int err = errno;
    std::cerr << "Unable to list contents of directory: " << path._path << "\n";
    std::cerr << ::strerror(err) << "\n";
  }
}

PathIterator::~PathIterator() {
  if (_dir) {
    ::closedir(_dir);
  }
}

bool PathIterator::next(StringRef& path) {
  if (_dir) {
    struct dirent *dp = ::readdir(_dir);
    if (dp) {
      path = &dp->d_name[0];
      return true;
    }
  }
  return false;
}

}}
