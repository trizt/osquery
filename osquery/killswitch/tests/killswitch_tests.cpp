/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
 */

#include <gtest/gtest.h>

#include <osquery/core.h>
#include <osquery/flags.h>
#include <osquery/killswitch.h>
#include <osquery/killswitch/killswitch_plugin.h>
#include <osquery/registry.h>
#include <osquery/tests/test_util.h>

namespace osquery {

DECLARE_uint32(killswitch_refresh_rate);

class KillswitchTests : public testing::Test {};

TEST_F(KillswitchTests, test_killswitch_plugin) {
  auto& rf = RegistryFactory::get();
  auto plugin = std::make_shared<KillswitchPlugin>();
  rf.registry(Killswitch::killswitch_)->add("test", plugin);
  EXPECT_TRUE(rf.setActive(Killswitch::killswitch_, "test").ok());

  {
    PluginResponse response;
    auto status = Registry::call(Killswitch::killswitch_,
                                 {{Killswitch::action_, Killswitch::isEnabled_},
                                  {Killswitch::key_, "testSwitch"}},
                                 response);
    EXPECT_FALSE(status.ok());
  }

  {
    PluginResponse response;
    auto status = Registry::call(
        Killswitch::killswitch_, {{Killswitch::key_, "testSwitch"}}, response);
    EXPECT_FALSE(status.ok());
  }

  {
    PluginResponse response;
    auto status = Registry::call(Killswitch::killswitch_,
                                 {{Killswitch::action_, "testSwitch"}},
                                 response);
    EXPECT_FALSE(status.ok());
  }

  plugin->addCacheEntry("testSwitch", true);

  {
    auto result = plugin->isEnabled("testSwitch");
    EXPECT_TRUE(result);
    EXPECT_TRUE(*result);
    EXPECT_TRUE(Killswitch::get().isNewCodeEnabled("testSwitch"));
  }
  {
    PluginResponse response;
    auto status = Registry::call(Killswitch::killswitch_,
                                 {{Killswitch::action_, Killswitch::isEnabled_},
                                  {Killswitch::key_, "testSwitch"}},
                                 response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response[0][Killswitch::isEnabled_], std::string("1"));
    auto result = Killswitch::get().isEnabled("testSwitch");
    EXPECT_TRUE(result);
    EXPECT_TRUE(*result);
    EXPECT_TRUE(Killswitch::get().isNewCodeEnabled("testSwitch"));
  }

  plugin->addCacheEntry("testSwitch", false);

  {
    auto result = plugin->isEnabled("testSwitch");
    EXPECT_TRUE(result);
    EXPECT_FALSE(*result);
    EXPECT_FALSE(Killswitch::get().isNewCodeEnabled("testSwitch"));
  }
  {
    PluginResponse response;
    auto status = Registry::call(Killswitch::killswitch_,
                                 {{Killswitch::action_, Killswitch::isEnabled_},
                                  {Killswitch::key_, "testSwitch"}},
                                 response);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response[0][Killswitch::isEnabled_], std::string("0"));
    auto result = Killswitch::get().isEnabled("testSwitch");
    EXPECT_TRUE(result);
    EXPECT_FALSE(*result);
    EXPECT_FALSE(Killswitch::get().isNewCodeEnabled("testSwitch"));
  }

  plugin->setCache(std::unordered_map<std::string, bool>());

  {
    PluginResponse response;
    auto status = Registry::call(Killswitch::killswitch_,
                                 {{Killswitch::action_, Killswitch::isEnabled_},
                                  {Killswitch::key_, "testSwitch"}},
                                 response);
    EXPECT_FALSE(status.ok());
    EXPECT_EQ(response.size(), 0);
    auto result = Killswitch::get().isEnabled("testSwitch");
    EXPECT_FALSE(result);

    EXPECT_TRUE(Killswitch::get().isNewCodeEnabled("testSwitch"));
  }

  EXPECT_FALSE(Killswitch::get().refresh());

  rf.registry(Killswitch::killswitch_)->remove("test");
}

} // namespace osquery
