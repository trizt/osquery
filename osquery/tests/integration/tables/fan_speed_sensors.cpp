
/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
 */

// Sanity check integration test for fan_speed_sensors
// Spec file: specs/darwin/fan_speed_sensors.table

#include <osquery/tests/integration/tables/helper.h>

namespace osquery {

class fanSpeedSensors : public IntegrationTableTest {};

TEST_F(fanSpeedSensors, test_sanity) {
  // 1. Query data
  // QueryData data = execute_query("select * from fan_speed_sensors");
  // 2. Check size before validation
  // ASSERT_GE(data.size(), 0ul);
  // ASSERT_EQ(data.size(), 1ul);
  // ASSERT_EQ(data.size(), 0ul);
  // 3. Build validation map
  // See IntegrationTableTest.cpp for avaialbe flags
  // Or use custom DataCheck object
  // ValidatatioMap row_map = {
  //      {"fan", NormalType}
  //      {"name", NormalType}
  //      {"actual", IntType}
  //      {"min", IntType}
  //      {"max", IntType}
  //      {"target", IntType}
  //}
  // 4. Perform validation
  // validate_rows(data, row_map);
}

} // namespace osquery
