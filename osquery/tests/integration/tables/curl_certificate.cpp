
/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
 */

// Sanity check integration test for curl_certificate
// Spec file: specs/curl_certificate.table

#include <osquery/tests/integration/tables/helper.h>

namespace osquery {

class curlCertificate : public IntegrationTableTest {};

TEST_F(curlCertificate, test_sanity) {
  // 1. Query data
  // QueryData data = execute_query("select * from curl_certificate");
  // 2. Check size before validation
  // ASSERT_GE(data.size(), 0ul);
  // ASSERT_EQ(data.size(), 1ul);
  // ASSERT_EQ(data.size(), 0ul);
  // 3. Build validation map
  // See IntegrationTableTest.cpp for avaialbe flags
  // Or use custom DataCheck object
  // ValidatatioMap row_map = {
  //      {"hostname", NormalType}
  //      {"common_name", NormalType}
  //      {"organization", NormalType}
  //      {"organization_unit", NormalType}
  //      {"serial_number", NormalType}
  //      {"issuer_common_name", NormalType}
  //      {"issuer_organization", NormalType}
  //      {"issuer_organization_unit", NormalType}
  //      {"valid_from", NormalType}
  //      {"valid_to", NormalType}
  //      {"sha256_fingerprint", NormalType}
  //      {"sha1_fingerprint", NormalType}
  //}
  // 4. Perform validation
  // EXPECT_TRUE(validate_rows(data, row_map));
}

} // namespace osquery
