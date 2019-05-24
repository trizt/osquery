/*
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include <map>
#include <string>

#define _WIN32_DCOM
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <psapi.h>
#include <stdlib.h>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>

#include <osquery/core.h>
#include <osquery/filesystem.h>
#include <osquery/logger.h>
#include <osquery/tables.h>

#include "osquery/core/conversions.h"
#include "osquery/core/utils.h"
#include "osquery/core/windows/wmi.h"

namespace osquery {
int getUidFromSid(PSID sid);
int getGidFromSid(PSID sid);
namespace tables {

std::set<long> getSelectedPids(const QueryContext& context) {
  std::set<long> pidlist;
  if (context.constraints.count("pid") > 0 &&
      context.constraints.at("pid").exists(EQUALS)) {
    for (const auto& pid : context.constraints.at("pid").getAll<int>(EQUALS)) {
      if (pid > 0) {
        pidlist.insert(pid);
      }
    }
  }

  /// If there are no constraints, pidlist will be an empty set
  return pidlist;
}

void genProcess(const WmiResultItem& result, QueryData& results_data) {
  Row r;
  Status s;
  long pid;
  long lPlaceHolder;
  std::string sPlaceHolder;
  HANDLE hProcess = nullptr;

  /// Store current process pid for more efficient API use.
  auto currentPid = GetCurrentProcessId();

  s = result.GetLong("ProcessId", pid);
  r["pid"] = s.ok() ? BIGINT(pid) : BIGINT(-1);

  long uid = -1;
  long gid = -1;
  if (pid == currentPid) {
    hProcess = GetCurrentProcess();
  } else {
    hProcess =
        OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, pid);
  }

  if (GetLastError() == ERROR_ACCESS_DENIED) {
    uid = 0;
    gid = 0;
  }

  result.GetString("Name", r["name"]);
  result.GetString("ExecutablePath", r["path"]);
  result.GetString("CommandLine", r["cmdline"]);
  result.GetString("ExecutionState", r["state"]);
  result.GetLong("ParentProcessId", lPlaceHolder);
  r["parent"] = BIGINT(lPlaceHolder);
  result.GetLong("Priority", lPlaceHolder);
  r["nice"] = INTEGER(lPlaceHolder);
  r["on_disk"] = osquery::pathExists(r["path"]).toString();

  std::vector<char> fileName(MAX_PATH);
  fileName.assign(MAX_PATH + 1, '\0');
  if (pid == currentPid) {
    GetModuleFileName(nullptr, fileName.data(), MAX_PATH);
  } else {
    GetModuleFileNameEx(hProcess, nullptr, fileName.data(), MAX_PATH);
  }
  r["cwd"] = SQL_TEXT(fileName.data());
  r["root"] = r["cwd"];

  r["pgroup"] = "-1";
  r["euid"] = "-1";
  r["suid"] = "-1";
  r["egid"] = "-1";
  r["sgid"] = "-1";
  r["start_time"] = "0";

  result.GetString("UserModeTime", sPlaceHolder);
  long long llHolder;
  osquery::safeStrtoll(sPlaceHolder, 10, llHolder);
  r["user_time"] = BIGINT(llHolder / 10000000);
  result.GetString("KernelModeTime", sPlaceHolder);
  osquery::safeStrtoll(sPlaceHolder, 10, llHolder);
  r["system_time"] = BIGINT(llHolder / 10000000);
  result.GetString("PrivatePageCount", sPlaceHolder);
  r["wired_size"] = BIGINT(sPlaceHolder);
  result.GetString("WorkingSetSize", sPlaceHolder);
  r["resident_size"] = sPlaceHolder;
  result.GetString("VirtualSize", sPlaceHolder);
  r["total_size"] = BIGINT(sPlaceHolder);

  /// Get the process UID and GID from its SID
  HANDLE tok = nullptr;
  std::vector<char> tokOwner(sizeof(TOKEN_OWNER), 0x0);
  auto ret = OpenProcessToken(hProcess, TOKEN_READ, &tok);
  if (ret != 0 && tok != nullptr) {
    unsigned long tokOwnerBuffLen;
    ret = GetTokenInformation(tok, TokenOwner, nullptr, 0, &tokOwnerBuffLen);
    if (ret == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
      tokOwner.resize(tokOwnerBuffLen);
      ret = GetTokenInformation(
          tok, TokenOwner, tokOwner.data(), tokOwnerBuffLen, &tokOwnerBuffLen);
    }
  }
  if (uid != 0 && ret != 0 && !tokOwner.empty()) {
    auto sid = PTOKEN_OWNER(tokOwner.data())->Owner;
    r["uid"] = INTEGER(getUidFromSid(sid));
    r["gid"] = INTEGER(getGidFromSid(sid));
  } else {
    r["uid"] = INTEGER(uid);
    r["gid"] = INTEGER(gid);
  }

  if (hProcess != nullptr) {
    CloseHandle(hProcess);
    hProcess = nullptr;
  }
  if (tok != nullptr) {
    CloseHandle(tok);
    tok = nullptr;
  }
  results_data.push_back(r);
}

QueryData genProcesses(QueryContext& context) {
  QueryData results;

  std::string query = "SELECT * FROM Win32_Process";

  auto pidlist = getSelectedPids(context);
  if (pidlist.size() > 0) {
    std::vector<std::string> constraints;
    for (const auto& pid : pidlist) {
      constraints.push_back("ProcessId=" + std::to_string(pid));
    }
    if (constraints.size() > 0) {
      query += " WHERE " + boost::algorithm::join(constraints, " OR ");
    }
  }

  WmiRequest request(query);
  if (request.getStatus().ok()) {
    for (const auto& item : request.results()) {
      long pid = 0;
      if (item.GetLong("ProcessId", pid).ok()) {
        genProcess(item, results);
      }
    }
  }

  return results;
}
}
}
