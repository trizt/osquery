
/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
 */

// Sanity check integration test for docker_containers
// Spec file: specs/posix/docker_containers.table

#include <osquery/tests/integration/tables/helper.h>

namespace osquery {

class dockerContainers : public IntegrationTableTest {};

TEST_F(dockerContainers, test_sanity) {
  // 1. Query data
  // QueryData data = execute_query("select * from docker_containers");
  // 2. Check size before validation
  // ASSERT_GE(data.size(), 0ul);
  // ASSERT_EQ(data.size(), 1ul);
  // ASSERT_EQ(data.size(), 0ul);
  // 3. Build validation map
  // See IntegrationTableTest.cpp for avaialbe flags
  // Or use custom DataCheck object
  // ValidatatioMap row_map = {
  //      {"id", NormalType}
  //      {"name", NormalType}
  //      {"image", NormalType}
  //      {"image_id", NormalType}
  //      {"command", NormalType}
  //      {"created", IntType}
  //      {"state", NormalType}
  //      {"status", NormalType}
  //      {"pid", IntType}
  //      {"path", NormalType}
  //      {"config_entrypoint", NormalType}
  //      {"started_at", NormalType}
  //      {"finished_at", NormalType}
  //      {"privileged", IntType}
  //      {"security_options", NormalType}
  //      {"env_variables", NormalType}
  //      {"readonly_rootfs", IntType}
  //      {"cgroup_namespace", NormalType}
  //      {"ipc_namespace", NormalType}
  //      {"mnt_namespace", NormalType}
  //      {"net_namespace", NormalType}
  //      {"pid_namespace", NormalType}
  //      {"user_namespace", NormalType}
  //      {"uts_namespace", NormalType}
  //}
  // 4. Perform validation
  // EXPECT_TRUE(validate_rows(data, row_map));
}

} // namespace osquery
