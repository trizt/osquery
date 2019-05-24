
/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
 */

// Sanity check integration test for virtual_memory_info
// Spec file: specs/darwin/virtual_memory_info.table

#include <osquery/tests/integration/tables/helper.h>

namespace osquery {

class virtualMemoryInfo : public IntegrationTableTest {};

TEST_F(virtualMemoryInfo, test_sanity) {
  // 1. Query data
  // QueryData data = execute_query("select * from virtual_memory_info");
  // 2. Check size before validation
  // ASSERT_GE(data.size(), 0ul);
  // ASSERT_EQ(data.size(), 1ul);
  // ASSERT_EQ(data.size(), 0ul);
  // 3. Build validation map
  // See IntegrationTableTest.cpp for avaialbe flags
  // Or use custom DataCheck object
  // ValidatatioMap row_map = {
  //      {"free", IntType}
  //      {"active", IntType}
  //      {"inactive", IntType}
  //      {"speculative", IntType}
  //      {"throttled", IntType}
  //      {"wired", IntType}
  //      {"purgeable", IntType}
  //      {"faults", IntType}
  //      {"copy", IntType}
  //      {"zero_fill", IntType}
  //      {"reactivated", IntType}
  //      {"purged", IntType}
  //      {"file_backed", IntType}
  //      {"anonymous", IntType}
  //      {"uncompressed", IntType}
  //      {"compressor", IntType}
  //      {"decompressed", IntType}
  //      {"compressed", IntType}
  //      {"page_ins", IntType}
  //      {"page_outs", IntType}
  //      {"swap_ins", IntType}
  //      {"swap_outs", IntType}
  //}
  // 4. Perform validation
  // validate_rows(data, row_map);
}

} // namespace osquery
