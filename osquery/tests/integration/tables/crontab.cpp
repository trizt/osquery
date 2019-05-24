
/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
 */

// Sanity check integration test for crontab
// Spec file: specs/posix/crontab.table

#include <osquery/tests/integration/tables/helper.h>

namespace osquery {

class Crontab : public IntegrationTableTest {};

TEST_F(Crontab, test_sanity) {
  QueryData data = execute_query("select * from crontab");
  std::unordered_set<std::string> month_list = {"jan",
                                                "feb",
                                                "mar",
                                                "apr",
                                                "may",
                                                "jun",
                                                "jul",
                                                "aug",
                                                "sep",
                                                "oct",
                                                "nov",
                                                "dec"};
  std::unordered_set<std::string> days_list = {
      "mon", "tue", "wed", "thu", "fri", "sat", "sun"};
  ValidatatioMap row_map = {{"event", NormalType},
                            {"minute", CronValuesCheck(0, 59)},
                            {"hour", CronValuesCheck(0, 23)},
                            {"day_of_month", CronValuesCheck(1, 31)},
                            {"month", CronValuesCheck(1, 31, month_list)},
                            {"day_of_week", CronValuesCheck(0, 6, days_list)},
                            {"command", NonEmptyString},
                            {"path", FileOnDisk}};
  EXPECT_TRUE(validate_rows(data, row_map));
}

} // namespace osquery
