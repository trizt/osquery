
/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
 */

// Sanity check integration test for logon_sessions
// Spec file: specs/windows/logon_sessions.table

#include <osquery/tests/integration/tables/helper.h>

namespace osquery {

class logonSessions : public IntegrationTableTest {};

TEST_F(logonSessions, test_sanity) {
  // 1. Query data
  // QueryData data = execute_query("select * from logon_sessions");
  // 2. Check size before validation
  // ASSERT_GE(data.size(), 0ul);
  // ASSERT_EQ(data.size(), 1ul);
  // ASSERT_EQ(data.size(), 0ul);
  // 3. Build validation map
  // See IntegrationTableTest.cpp for avaialbe flags
  // Or use custom DataCheck object
  // ValidatatioMap row_map = {
  //      {"logon_id", IntType}
  //      {"user", NormalType}
  //      {"logon_domain", NormalType}
  //      {"authentication_package", NormalType}
  //      {"logon_type", NormalType}
  //      {"session_id", IntType}
  //      {"logon_sid", NormalType}
  //      {"logon_time", IntType}
  //      {"logon_server", NormalType}
  //      {"dns_domain_name", NormalType}
  //      {"upn", NormalType}
  //      {"logon_script", NormalType}
  //      {"profile_path", NormalType}
  //      {"home_directory", NormalType}
  //      {"home_directory_drive", NormalType}
  //}
  // 4. Perform validation
  // validate_rows(data, row_map);
}

} // namespace osquery
