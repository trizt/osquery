/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
 */

// Sanity check integration test for memory_map
// Spec file: specs/linux/memory_map.table

#include <osquery/core/conversions.h>

#include <osquery/tests/integration/tables/helper.h>

namespace osquery {

class MemoryMapTest : public IntegrationTableTest {};

TEST_F(MemoryMapTest, test_sanity) {
  QueryData data = execute_query("select * from memory_map");
  ASSERT_GT(data.size(), 0ul);
  ValidatatioMap row_map = {{"name", NonEmptyString},
                            {"start", NonNegativeInt},
                            {"end", NonNegativeInt}};
  validate_rows(data, row_map);

  for (const auto& row : data) {
    auto start = tryTo<unsigned long long>(row.at("start"));
    auto end = tryTo<unsigned long long>(row.at("end"));
    ASSERT_TRUE(start) << "start does not fit in unsigned long long";
    ASSERT_TRUE(end) << "end does not fit in unsigned long long";
    ASSERT_LE(*start, *end) << "start should be less than or equal to end";
  }
}

} // namespace osquery
