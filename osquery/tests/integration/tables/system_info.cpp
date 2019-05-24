
/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
 */

// Sanity check integration test for system_info
// Spec file: specs/system_info.table

#include <osquery/tests/integration/tables/helper.h>

namespace osquery {

class SystemInfo : public IntegrationTableTest {};

TEST_F(SystemInfo, test_sanity) {
  QueryData data = execute_query("select * from system_info");
  ASSERT_EQ(data.size(), 1ul);
  ValidatatioMap row_map = {{"hostname", NormalType},
                            {"uuid", ValidUUID},
                            {"cpu_type", NonEmptyString},
                            {"cpu_subtype", NormalType},
                            {"cpu_brand", NormalType},
                            {"cpu_physical_cores", NonNegativeInt},
                            {"cpu_logical_cores", NonNegativeInt},
                            {"cpu_microcode", NormalType},
                            {"physical_memory", NonNegativeInt},
                            {"hardware_vendor", NormalType},
                            {"hardware_model", NormalType},
                            {"hardware_version", NormalType},
                            {"hardware_serial", NonEmptyString},
                            {"computer_name", NormalType},
                            {"local_hostname", NonEmptyString}};
  EXPECT_TRUE(validate_rows(data, row_map));
}

} // namespace osquery
