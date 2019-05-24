/*
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include <osquery/filesystem.h>
#include <osquery/sql.h>

#include "osquery/database/plugins/rocksdb.h"
#include "osquery/database/tests/plugin_tests.h"

namespace osquery {

class RocksDBDatabasePluginTests : public DatabasePluginTests {
 protected:
  std::string name() override {
    return "rocksdb";
  }
};

// Define the default set of database plugin operation tests.
CREATE_DATABASE_TESTS(RocksDBDatabasePluginTests);

TEST_F(RocksDBDatabasePluginTests, test_rocksdb_loglevel) {
  // Make sure a log file was created.
  EXPECT_FALSE(pathExists(path_ + "/LOG"));

  // Make sure no log file is created.
  // RocksDB logs are intercepted and forwarded to the GLog sink.
  auto details = SQL::selectAllFrom("file", "path", EQUALS, path_ + "/LOG");
  ASSERT_EQ(details.size(), 0U);
}

TEST_F(RocksDBDatabasePluginTests, test_corruption) {
  ASSERT_TRUE(pathExists(path_));
  ASSERT_FALSE(pathExists(path_ + ".backup"));

  // Mark the database as corrupted
  RocksDBDatabasePlugin::setCorrupted();
  printf("set corrupt\n");
  resetDatabase();
  printf("did reset\n");

  EXPECT_TRUE(pathExists(path_ + ".backup"));

  // Remove the backup and expect another reload to not create one.
  removePath(path_ + ".backup");
  ASSERT_FALSE(pathExists(path_ + ".backup"));

  resetDatabase();
  EXPECT_FALSE(pathExists(path_ + ".backup"));
}
}
