
/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
 */

// Sanity check integration test for file
// Spec file: specs/utility/file.table

#include <fstream>

#include <osquery/tests/integration/tables/helper.h>

#include <boost/filesystem.hpp>

namespace osquery {

class FileTests : public IntegrationTableTest {
 public:
  boost::filesystem::path filepath;

  virtual void SetUp() {
    auto directory =
        boost::filesystem::temp_directory_path() /
        boost::filesystem::unique_path("test-integration-file-table.%%%%-%%%%");
    ASSERT_TRUE(boost::filesystem::create_directory(directory));
    filepath = directory / boost::filesystem::path("file-table-test.txt");
    {
      auto fout = std::ofstream(filepath.native(), std::ios::out);
      fout.open(filepath.string(), std::ios::out);
      fout << "test";
    }
  }

  virtual void TearDown() {
    boost::filesystem::remove_all(filepath.parent_path());
  }
};

TEST_F(FileTests, test_sanity) {
  QueryData data = execute_query(
      "select * from file where path like \"" +
      (filepath.parent_path() / boost::filesystem::path("%.txt")).string() +
      "\"");
  EXPECT_EQ(data.size(), 1ul);

  ValidatatioMap row_map = {{"path", FileOnDisk},
                            {"directory", DirectoryOnDisk},
                            {"filename", NonEmptyString},
                            {"inode", IntType},
                            {"uid", NonNegativeInt},
                            {"gid", NonNegativeInt},
                            {"mode", NormalType},
                            {"device", IntType},
                            {"size", NonNegativeInt},
                            {"block_size", NonNegativeInt},
                            {"atime", NonNegativeInt},
                            {"mtime", NonNegativeInt},
                            {"ctime", NonNegativeInt},
                            {"btime", NonNegativeInt},
                            {"hard_links", IntType},
                            {"symlink", IntType},
                            {"type", NonEmptyString}};
#ifdef WIN32
  row_map["attributes"] = NormalType;
  row_map["volume_serial"] = NormalType;
  row_map["file_id"] = NormalType;
#endif

  validate_rows(data, row_map);
  ASSERT_EQ(data[0]["path"], filepath.string());
  ASSERT_EQ(data[0]["directory"], filepath.parent_path().string());
  ASSERT_EQ(data[0]["filename"], filepath.filename().string());
}

} // namespace osquery
