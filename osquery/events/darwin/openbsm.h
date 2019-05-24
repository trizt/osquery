/*
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#pragma once

#include <osquery/events.h>

namespace osquery {

struct OpenBSMSubscriptionContext : public SubscriptionContext {
  // The id of the event you want to alert on (23 for execve for example)
  int event_id;
};

struct OpenBSMEventContext : public EventContext {
  // The event_id of the OpenBSM log
  int event_id;
  // The tokens for the event to pass to the subscriber
  std::vector<tokenstr_t> tokens;
  // A smart pointer to the memory returned by OpenBSM
  std::shared_ptr<unsigned char> buffer;
};

using OpenBSMEventContextRef = std::shared_ptr<OpenBSMEventContext>;
using OpenBSMSubscriptionContextRef =
    std::shared_ptr<OpenBSMSubscriptionContext>;

/// This is a dispatched service that handles published audit replies.
class OpenBSMConsumerRunner;

class OpenBSMEventPublisher
    : public EventPublisher<OpenBSMSubscriptionContext, OpenBSMEventContext> {
  DECLARE_PUBLISHER("openbsm");

 public:
  Status setUp() override;

  void configure() override;

  void tearDown() override;

  Status run() override;

  OpenBSMEventPublisher() : EventPublisher() {}

  virtual ~OpenBSMEventPublisher() {
    tearDown();
  }

 private:
  FILE* audit_pipe_ = nullptr;
  /// Apply normal subscription to event matching logic.
  bool shouldFire(const OpenBSMSubscriptionContextRef& mc,
                  const OpenBSMEventContextRef& ec) const override;
};
} // namespace osquery
