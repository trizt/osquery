
/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
 */

// Sanity check integration test for video_info
// Spec file: specs/windows/video_info.table

#include <osquery/tests/integration/tables/helper.h>

namespace osquery {

class videoInfo : public IntegrationTableTest {};

TEST_F(videoInfo, test_sanity) {
  // 1. Query data
  // QueryData data = execute_query("select * from video_info");
  // 2. Check size before validation
  // ASSERT_GE(data.size(), 0ul);
  // ASSERT_EQ(data.size(), 1ul);
  // ASSERT_EQ(data.size(), 0ul);
  // 3. Build validation map
  // See IntegrationTableTest.cpp for avaialbe flags
  // Or use custom DataCheck object
  // ValidatatioMap row_map = {
  //      {"color_depth", IntType}
  //      {"driver", NormalType}
  //      {"driver_date", NormalType}
  //      {"driver_version", NormalType}
  //      {"manufacturer", NormalType}
  //      {"model", NormalType}
  //      {"series", NormalType}
  //      {"video_mode", NormalType}
  //}
  // 4. Perform validation
  // validate_rows(data, row_map);
}

} // namespace osquery
