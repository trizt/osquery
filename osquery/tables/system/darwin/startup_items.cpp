/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
 */

#include <boost/filesystem.hpp>

#include <osquery/core.h>
#include <osquery/tables.h>
#include <osquery/filesystem.h>
#include <osquery/logger.h>

namespace fs = boost::filesystem;
namespace pt = boost::property_tree;

namespace osquery {
namespace tables {

const std::vector<std::string> kLibraryStartupItemPaths = {
    "/System/Library/StartupItems/", "/Library/StartupItems/",
};

// Path (after /Users/foo) where the login items plist will be found
const std::string kLoginItemsPlistPath =
    "Library/Preferences/com.apple.loginitems.plist";

// Key to the array within the Login Items plist containing the items
const std::vector<std::string> kLoginItemsKeyPaths = {
    "SessionItems.CustomListItems", "privilegedlist.CustomListItems",
};

void genLibraryStartupItems(const std::string& sysdir, QueryData& results) {
  try {
    fs::directory_iterator it((fs::path(sysdir))), end;
    for (; it != end; ++it) {
      if (!fs::exists(it->status()) || !fs::is_directory(it->status())) {
        continue;
      }

      Row r;
      r["name"] = it->path().string();
      r["path"] = it->path().string();
      r["type"] = "Startup Item";
      r["status"] = "enabled";
      r["source"] = sysdir;
      results.push_back(r);
    }
  } catch (const fs::filesystem_error& e) {
    VLOG(1) << "Error traversing " << sysdir << ": " << e.what();
  }
}

void genLoginItems(const fs::path& sipath, QueryData& results) {
  pt::ptree tree;
  if (!pathExists(sipath.string()).ok() || !isReadable(sipath.string()).ok()) {
    // User does not have a startup items list, or bad permissions.
    return;
  }

  if (!osquery::parsePlist(sipath.string(), tree).ok()) {
    // Could not parse the user's startup items plist.
    return;
  }

  // Enumerate Login Items if we successfully opened the plist.
  for (const auto& plist_path : kLoginItemsKeyPaths) {
    try {
      for (const auto& entry : tree.get_child(plist_path)) {
        Row r;
        r["name"] = entry.second.get<std::string>("Name", "");
        r["type"] = "Login Item";
        r["source"] = sipath.string();

        auto alias_data = entry.second.get<std::string>("Alias", "");
        std::string bin_path;
        auto s = pathFromPlistAliasData(alias_data, bin_path);
        if (!s.ok()) {
          VLOG(1) << "No valid path found for " << r["name"] << ": "
                  << s.getMessage();
        }
        r["path"] = std::move(bin_path);
        results.push_back(r);
      }
    } catch (const pt::ptree_error& e) {
      continue;
    }
  }
}

QueryData genStartupItems(QueryContext& context) {
  QueryData results;

  // Get the login items available for all users
  genLoginItems("/", results);

  // Get the login items available in System Preferences for each user.
  for (const auto& dir : getHomeDirectories()) {
    auto sipath = dir / kLoginItemsPlistPath;
    genLoginItems(sipath.string(), results);
  }

  // Find system wide startup items in Library directories.
  for (const auto& dir : kLibraryStartupItemPaths) {
    genLibraryStartupItems(dir, results);
  }

  return results;
}
}
}
