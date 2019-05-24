/*
*  Copyright (c) 2014-present, Facebook, Inc.
*  All rights reserved.
*
*  This source code is licensed under the BSD-style license found in the
*  LICENSE file in the root directory of this source tree. An additional grant
*  of patent rights can be found in the PATENTS file in the same directory.
*
*/

#include <fstream>
#include <string>

#include <osquery/core/conversions.h>
#include <osquery/filesystem.h>
#include <osquery/logger.h>
#include <osquery/system.h>
#include <osquery/tables.h>

namespace osquery {
namespace tables {

const size_t kNumFields = 2;
const std::set<std::string> kPythonPath = {
    "/usr/local/lib/python2.7/dist-packages/",
    "/usr/local/lib/python2.7/site-packages/",
    "/usr/lib/python2.7/dist-packages/",
    "/usr/lib/python2.7/site-packages/",
};

void genPackage(const std::string& path, Row& r) {
  std::string content;

  if (!readFile(path, content).ok()) {
    TLOG << "Cannot find info file: " << path;
    return;
  }

  auto lines = split(content, "\n");

  for (const auto& line : lines) {
    auto fields = split(line, ":");

    if (fields.size() != kNumFields) {
      continue;
    }

    if (fields[0] == "Name") {
      r["name"] = fields[1];
    } else if (fields[0] == "Version") {
      r["version"] = fields[1];
    } else if (fields[0] == "Summary") {
      r["summary"] = fields[1];
    } else if (fields[0] == "Author") {
      r["author"] = fields[1];
    } else if (fields[0] == "License") {
      r["license"] = fields[1];
      break;
    }
  }
}

QueryData genPythonPackages(QueryContext& context) {
  QueryData results;

  for (const auto& key : kPythonPath) {
    std::vector<std::string> directories;
    if (!listDirectoriesInDirectory(key, directories, true).ok()) {
      continue;
    }

    for (const auto& directory : directories) {
      if (!isDirectory(directory).ok()) {
        continue;
      }

      Row r;
      if (directory.find(".dist-info") != std::string::npos) {
        auto path = directory + "/METADATA";
        genPackage(path, r);
      } else if (directory.find(".egg-info") != std::string::npos) {
        auto path = directory + "/PKG-INFO";
        genPackage(path, r);
      } else {
        continue;
      }
      r["path"] = directory;
      results.push_back(r);
    }
  }

  return results;
}
}
}
