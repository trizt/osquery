/*
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <gtest/gtest.h>

#include <osquery/sql.h>

#include "osquery/carver/carver.h"
#include "osquery/core/json.h"
#include "osquery/filesystem/fileops.h"
#include "osquery/tests/test_util.h"

namespace osquery {

namespace fs = boost::filesystem;
namespace pt = boost::property_tree;

/// Prefix used for posix tar archive.
const std::string kTestCarveNamePrefix = "carve_";

std::string genGuid() {
  return boost::uuids::to_string(boost::uuids::random_generator()());
};

class CarverTests : public testing::Test {
 public:
  CarverTests() {
    fs::create_directories(kFakeDirectory + "/files_to_carve/");
    writeTextFile(kFakeDirectory + "/files_to_carve/secrets.txt",
                  "This is a message I'd rather no one saw.");
    writeTextFile(kFakeDirectory + "/files_to_carve/evil.exe",
                  "MZP\x00\x02\x00\x00\x00\x04\x00\x0f\x00\xff\xff");

    auto paths = platformGlob(kFakeDirectory + "/files_to_carve/*");
    for (const auto& p : paths) {
      carvePaths.insert(p);
    }
  };

  std::set<std::string>& getCarvePaths() {
    return carvePaths;
  }

 protected:
  void SetUp() override {
    createMockFileStructure();
  }

  void TearDown() override {
    tearDownMockFileStructure();
  }

 private:
  std::set<std::string> carvePaths;
};

TEST_F(CarverTests, test_carve_files_locally) {
  auto guid_ = genGuid();
  auto paths_ = getCarvePaths();
  std::string requestId = "";
  Carver carve(getCarvePaths(), guid_, requestId);

  Status s;
  for (const auto& p : paths_) {
    s = carve.carve(fs::path(p));
    EXPECT_TRUE(s.ok());
  }

  std::string carveFSPath = carve.getCarveDir().string();
  auto paths = platformGlob(carveFSPath + "/*");
  std::set<fs::path> carves;
  for (const auto& p : paths) {
    carves.insert(fs::path(p));
  }

  EXPECT_EQ(carves.size(), 2U);
  s = carve.compress(carves);
  EXPECT_TRUE(s.ok());

  auto tarPath = carveFSPath + "/" + kTestCarveNamePrefix + guid_ + ".tgz";
  PlatformFile tar(tarPath, PF_OPEN_EXISTING | PF_READ);
  EXPECT_TRUE(tar.isValid());
  EXPECT_GT(tar.size(), 0U);
}
}
