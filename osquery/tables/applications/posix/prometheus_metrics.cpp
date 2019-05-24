/*
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#include <sstream>

#include <boost/network/protocol/http/client.hpp>

#include <osquery/config.h>
#include <osquery/logger.h>
#include <osquery/tables.h>

#include <osquery/core/conversions.h>

#include "osquery/config/parsers/prometheus_targets.h"
#include "osquery/tables/applications/posix/prometheus_metrics.h"

namespace http = boost::network::http;

namespace osquery {
namespace tables {

void parseScrapeResults(
    const std::map<std::string, PrometheusResponseData>& scrapeResults,
    QueryData& rows) {
  for (auto const& target : scrapeResults) {
    std::stringstream ss(target.second.content);
    std::string dest;

    while (std::getline(ss, dest)) {
      if (!dest.empty() && dest[0] != '#') {
        auto metric(osquery::split(dest, " "));

        if (metric.size() > 1) {
          Row r;
          r[kColTargetName] = target.first;
          r[kColTimeStamp] = BIGINT(target.second.timestampMS.count());
          r[kColMetric] = metric[0];
          r[kColValue] = metric[1];

          rows.push_back(r);
        }
      }
    }
  }
}

void scrapeTargets(std::map<std::string, PrometheusResponseData>& scrapeResults,
                   size_t timeoutS) {
  http::client client(
      http::client::options().follow_redirects(true).timeout(timeoutS));

  for (auto& target : scrapeResults) {
    try {
      http::client::request request(target.first);
      http::client::response response(client.get(request));

      target.second.timestampMS =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch());
      target.second.content = static_cast<std::string>(body(response));

    } catch (std::exception& e) {
      LOG(ERROR) << "Failed on scrape of target " << target.first << ": "
                 << e.what();
    }
  }
}

QueryData genPrometheusMetrics(QueryContext& context) {
  QueryData result;
  // get urls from config
  auto parser = Config::getParser("prometheus_targets");
  if (parser == nullptr || parser.get() == nullptr) {
    return result;
  }

  /* Add a specific value to the default property tree to differentiate it from
   * the scenario where the user does not provide any prometheus_targets config.
   */
  const auto& config = parser->getData().get_child(
      kConfigParserRootKey, boost::property_tree::ptree("UNEXPECTED"));
  if (config.get_value("") == "UNEXPECTED") {
    LOG(WARNING) << "Could not load prometheus_targets root key: "
                 << kConfigParserRootKey;
    return result;
  }
  if (config.count("urls") == 0) {
    /* Only warn the user if they supplied the config, but did not supply any
     * urls. */
    if (!config.empty()) {
      LOG(WARNING)
          << "Configuration for prometheus_targets is missing field: urls";
    }

    return result;
  }

  std::map<std::string, PrometheusResponseData> sr;
  /* Below should be unreachable if there were no urls child node, but we set
   * handle with default value for consistency's sake and for added robustness.
   */
  auto urls = config.get_child("urls", boost::property_tree::ptree());
  if (urls.empty()) {
    return result;
  }
  for (const auto& url :
       config.get_child("urls", boost::property_tree::ptree())) {
    if (!url.first.empty()) {
      return result;
    }

    sr[url.second.data()] = PrometheusResponseData{};
  }

  size_t timeout = config.get_child("timeout", boost::property_tree::ptree())
                       .get_value<size_t>(1);

  scrapeTargets(sr, timeout);
  parseScrapeResults(sr, result);

  return result;
}
}
}
