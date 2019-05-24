/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
 */

#include <unordered_map>

#include <boost/format.hpp>

#include <osquery/dispatcher.h>
#include <osquery/flags.h>
#include <osquery/logger.h>
#include <osquery/numeric_monitoring.h>
#include <osquery/numeric_monitoring/plugin_interface.h>
#include <osquery/numeric_monitoring/pre_aggregation_cache.h>
#include <osquery/registry_factory.h>

namespace osquery {

FLAG(bool,
     enable_numeric_monitoring,
     false,
     "Enable numeric monitoring system");
FLAG(string,
     numeric_monitoring_plugins,
     "filesystem",
     "Coma separated numeric monitoring plugins names");
FLAG(uint64,
     numeric_monitoring_pre_aggregation_time,
     60,
     "Time period in seconds for numeric monitoring pre-aggreagation buffer.");

namespace {
using monitoring::PreAggregationType;

template <typename KeyType, typename ValueType>
inline auto reverseMap(const std::unordered_map<KeyType, ValueType>& straight) {
  auto reversed = std::unordered_map<ValueType, KeyType>{};
  for (const auto& item : straight) {
    reversed.emplace(item.second, item.first);
  }
  return reversed;
}

const auto& getAggregationTypeToStringTable() {
  const auto static table = std::unordered_map<PreAggregationType, std::string>{
      {PreAggregationType::None, "none"},
      {PreAggregationType::Sum, "sum"},
      {PreAggregationType::Min, "min"},
      {PreAggregationType::Max, "max"},
  };
  return table;
}

const auto& getStringToAggregationTypeTable() {
  const auto static table = reverseMap(getAggregationTypeToStringTable());
  return table;
}

} // namespace

template <>
std::string to<std::string>(const monitoring::PreAggregationType& from) {
  auto it = getAggregationTypeToStringTable().find(from);
  if (it == getAggregationTypeToStringTable().end()) {
    LOG(ERROR) << "Unknown PreAggregationType "
               << static_cast<std::underlying_type<PreAggregationType>::type>(
                      from)
               << " could not be converted to the string";
    return "";
  }
  return it->second;
}

template <>
Expected<monitoring::PreAggregationType, ConversionError>
tryTo<monitoring::PreAggregationType>(const std::string& from) {
  auto it = getStringToAggregationTypeTable().find(from);
  if (it == getStringToAggregationTypeTable().end()) {
    return createError(
        ConversionError::InvalidArgument,
        boost::str(
            boost::format(
                "Wrong string representation of `PreAggregationType`: \"%s\"") %
            from));
  }
  return it->second;
}

namespace monitoring {

namespace {

class FlusherIsScheduled {};
FlusherIsScheduled schedule();

class PreAggregationBuffer final {
 public:
  static PreAggregationBuffer& get() {
    static PreAggregationBuffer instance{};
    static auto const flusher_is_scheduled = schedule();
    boost::ignore_unused(flusher_is_scheduled);
    return instance;
  }

  void record(const std::string& path,
              const ValueType& value,
              const PreAggregationType& pre_aggregation,
              const TimePoint& time_point) {
    if (0 == FLAGS_numeric_monitoring_pre_aggregation_time) {
      dispatchOne(path, value, pre_aggregation, time_point);
    } else {
      std::lock_guard<std::mutex> lock(mutex_);
      cache_.addPoint(Point(path, value, pre_aggregation, time_point));
    }
  }

  void flush() {
    auto points = takeCachedPoints();
    for (const auto& pt : points) {
      dispatchOne(
          pt.path_, pt.value_, pt.pre_aggregation_type_, pt.time_point_);
    }
  }

 private:
  std::vector<Point> takeCachedPoints() {
    std::lock_guard<std::mutex> lock(mutex_);
    auto points = cache_.takePoints();
    return points;
  }

  void dispatchOne(const std::string& path,
                   const ValueType& value,
                   const PreAggregationType& pre_aggregation,
                   const TimePoint& time_point) {
    auto status = Registry::call(
        registryName(),
        FLAGS_numeric_monitoring_plugins,
        {
            {recordKeys().path, path},
            {recordKeys().value, std::to_string(value)},
            {recordKeys().pre_aggregation, to<std::string>(pre_aggregation)},
            {recordKeys().timestamp,
             std::to_string(time_point.time_since_epoch().count())},
        });
    if (!status.ok()) {
      LOG(ERROR) << "Data loss. Numeric monitoring point dispatch failed: "
                 << status.what();
    }
  }

 private:
  PreAggregationCache cache_;
  std::mutex mutex_;
};

class PreAggregationFlusher : public InternalRunnable {
 public:
  explicit PreAggregationFlusher()
      : InternalRunnable("numeric_monitoring_pre_aggregation_buffer_flusher") {}

  void start() override {
    while (!interrupted() &&
           0 != FLAGS_numeric_monitoring_pre_aggregation_time) {
      pauseMilli(
          std::chrono::seconds(FLAGS_numeric_monitoring_pre_aggregation_time));
      PreAggregationBuffer::get().flush();
    }
  }
};

FlusherIsScheduled schedule() {
  Dispatcher::addService(std::make_shared<PreAggregationFlusher>());
  return FlusherIsScheduled{};
}

} // namespace

void flushForTests() {
  PreAggregationBuffer::get().flush();
}

void record(const std::string& path,
            ValueType value,
            PreAggregationType pre_aggregation,
            TimePoint time_point) {
  if (!FLAGS_enable_numeric_monitoring) {
    return;
  }
  PreAggregationBuffer::get().record(
      path, value, pre_aggregation, std::move(time_point));
}

} // namespace monitoring
} // namespace osquery
