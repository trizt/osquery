/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
 */

#pragma once

#include <vector>

namespace osquery {
namespace tables {

#define INTEL_ME_LINUX_IOCTL 0xC0104801
#define INTEL_ME_WINDOWS_IOCTL 0x8000E004

/**
 * @brief Intel HECI GUID for the update application.
 *
 * This GUID can be used on every OS over the HECI protocol to inspect the
 * FW version.
 */
const std::vector<uint8_t> kMEIUpdateGUID{
    232, 205, 157, 48, 177, 204, 98, 64, 143, 120, 96, 1, 21, 163, 67, 39,
};

struct mei_response {
  uint32_t maxlen;
  uint8_t version;
};

struct mei_version {
  uint32_t important_details[7];
  uint16_t major;
  uint16_t minor;
  uint16_t hotfix;
  uint16_t build;
  uint16_t r_major;
  uint16_t r_minor;
  uint16_t r_hotfix;
  uint16_t r_build;
  uint16_t codes[6];
#ifdef WIN32
  uint16_t codes2[256];
#endif
};
}
}
