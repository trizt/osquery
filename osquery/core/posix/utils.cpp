/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
 */

#include <climits>
#include <string.h>

#include <osquery/status.h>

#include "osquery/core/utils.h"

namespace osquery {

std::string platformAsctime(const struct tm* timeptr) {
  if (timeptr == nullptr) {
    return "";
  }

  // Manual says at least 26 characters.
  char buffer[32] = {0};
  return ::asctime_r(timeptr, buffer);
}

std::string platformStrerr(int errnum) {
  return ::strerror(errnum);
}

const std::string canonicalize_file_name(const char* name) {
#ifdef PATH_MAX
  // On supported platforms where PATH_MAX is defined we can pass null
  // as buffer, and allow libc to alloced space
  // On centos/ubuntu libc will use realloc so we will be able to resolve
  // any path
  // On darwin libc will allocate PATH_MAX buffer for us
  char* resolved = realpath(name, nullptr);
  std::string result = (resolved == nullptr) ? name : resolved;
  free(resolved);
#else
#warning PATH_MAX is undefined, please read comment below
  // PATH_MAX is not defined, very likely it's not officially supported
  // os, our best guess is _PC_PATH_MAX if available
  // In case of failure fallback to "safe" buffer of 8K

  long int path_max = pathconf(name, _PC_PATH_MAX);
  if (path_max <= 0) {
    path_max = 8 * 1024;
  }
  char* buffer = static_cast<char*>(malloc(path_max));
  char* resolved = realpath(name, buffer);
  std::string result = (resolved == nullptr) ? name : resolved;
  free(buffer);
#endif
  return result;
}

} // namespace osquery
