
/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
 */

// Sanity check integration test for smc_keys
// Spec file: specs/darwin/smc_keys.table

#include <osquery/tests/integration/tables/helper.h>

namespace osquery {

class smcKeys : public IntegrationTableTest {};

TEST_F(smcKeys, test_sanity) {
  // 1. Query data
  // QueryData data = execute_query("select * from smc_keys");
  // 2. Check size before validation
  // ASSERT_GE(data.size(), 0ul);
  // ASSERT_EQ(data.size(), 1ul);
  // ASSERT_EQ(data.size(), 0ul);
  // 3. Build validation map
  // See IntegrationTableTest.cpp for avaialbe flags
  // Or use custom DataCheck object
  // ValidatatioMap row_map = {
  //      {"key", NormalType}
  //      {"type", NormalType}
  //      {"size", IntType}
  //      {"value", NormalType}
  //      {"hidden", IntType}
  //}
  // 4. Perform validation
  // validate_rows(data, row_map);
}

} // namespace osquery
