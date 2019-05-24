
/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
 */

// Sanity check integration test for ec2_instance_tags
// Spec file: specs/linux/ec2_instance_tags.table

#include <osquery/tests/integration/tables/helper.h>

namespace osquery {

class ec2InstanceTags : public IntegrationTableTest {};

TEST_F(ec2InstanceTags, test_sanity) {
  // 1. Query data
  // QueryData data = execute_query("select * from ec2_instance_tags");
  // 2. Check size before validation
  // ASSERT_GE(data.size(), 0ul);
  // ASSERT_EQ(data.size(), 1ul);
  // ASSERT_EQ(data.size(), 0ul);
  // 3. Build validation map
  // See IntegrationTableTest.cpp for avaialbe flags
  // Or use custom DataCheck object
  // ValidatatioMap row_map = {
  //      {"instance_id", NormalType}
  //      {"key", NormalType}
  //      {"value", NormalType}
  //}
  // 4. Perform validation
  // validate_rows(data, row_map);
}

} // namespace osquery
