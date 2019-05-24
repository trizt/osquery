/*
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

// clang-format off
// This must be here to prevent a WinSock.h exists error
#include "osquery/remote/transports/tls.h"
// clang-format on

#include <boost/filesystem.hpp>

#include <osquery/core.h>
#include <osquery/filesystem.h>

namespace fs = boost::filesystem;
namespace http = boost::network::http;

namespace osquery {

const std::string kTLSCiphers =
    "ECDH+AESGCM:DH+AESGCM:ECDH+AES256:DH+AES256:ECDH+AES128:DH+AES:ECDH+3DES:"
    "DH+3DES:RSA+AESGCM:RSA+AES:RSA+3DES:!aNULL:!MD5";
const std::string kTLSUserAgentBase = "osquery/";

/// TLS server hostname.
CLI_FLAG(string,
         tls_hostname,
         "",
         "TLS/HTTPS hostname for Config, Logger, and Enroll plugins");

/// Path to optional TLS server/CA certificate(s), used for pinning.
CLI_FLAG(string,
         tls_server_certs,
         OSQUERY_CERTS_HOME "certs.pem",
         "Optional path to a TLS server PEM certificate(s) bundle");

/// Path to optional TLS client certificate, used for enrollment/requests.
CLI_FLAG(string,
         tls_client_cert,
         "",
         "Optional path to a TLS client-auth PEM certificate");

/// Path to optional TLS client secret key, used for enrollment/requests.
CLI_FLAG(string,
         tls_client_key,
         "",
         "Optional path to a TLS client-auth PEM private key");

#if defined(DEBUG)
HIDDEN_FLAG(bool,
            tls_allow_unsafe,
            false,
            "Allow TLS server certificate trust failures");
#endif

HIDDEN_FLAG(bool, tls_dump, false, "Print remote requests and responses");

/// Undocumented feature to override TLS endpoints.
HIDDEN_FLAG(bool, tls_node_api, false, "Use node key as TLS endpoints");

DECLARE_bool(verbose);

TLSTransport::TLSTransport() : verify_peer_(true) {
  if (FLAGS_tls_server_certs.size() > 0) {
    server_certificate_file_ = FLAGS_tls_server_certs;
  }

  if (FLAGS_tls_client_cert.size() > 0 && FLAGS_tls_client_key.size() > 0) {
    client_certificate_file_ = FLAGS_tls_client_cert;
    client_private_key_file_ = FLAGS_tls_client_key;
  }
}

void TLSTransport::decorateRequest(http::client::request& r) {
  r << boost::network::header("Connection", "close");
  r << boost::network::header("Content-Type", serializer_->getContentType());
  r << boost::network::header("Accept", serializer_->getContentType());
  r << boost::network::header("Host", FLAGS_tls_hostname);
  r << boost::network::header("User-Agent", kTLSUserAgentBase + kVersion);
}

http::client TLSTransport::getClient() {
  http::client::options options;
  options.follow_redirects(true).always_verify_peer(verify_peer_).timeout(16);

  std::string ciphers = kTLSCiphers;
  if (!isPlatform(PlatformType::TYPE_OSX)) {
    // Otherwise we prefer GCM and SHA256+
    ciphers += ":!CBC:!SHA";
  }

#if defined(DEBUG)
  // Configuration may allow unsafe TLS testing if compiled as a debug target.
  if (FLAGS_tls_allow_unsafe) {
    options.always_verify_peer(false);
  }
#endif

  options.openssl_ciphers(ciphers);
  options.openssl_options(SSL_OP_NO_SSLv3 | SSL_OP_NO_SSLv2 | SSL_OP_ALL);

  if (server_certificate_file_.size() > 0) {
    if (!osquery::isReadable(server_certificate_file_).ok()) {
      LOG(WARNING) << "Cannot read TLS server certificate(s): "
                   << server_certificate_file_;
    } else {
      // There is a non-default server certificate set.
      boost::system::error_code ec;

      auto status = fs::status(server_certificate_file_, ec);
      options.openssl_verify_path(server_certificate_file_);

      // On Windows, we cannot set openssl_certificate to a directory
      if (isPlatform(PlatformType::TYPE_WINDOWS) &&
          status.type() != fs::regular_file) {
        LOG(WARNING) << "Cannot set a non-regular file as a certificate: "
                     << server_certificate_file_;
      } else {
        options.openssl_certificate(server_certificate_file_);
      }
    }
  }

  if (client_certificate_file_.size() > 0) {
    if (!osquery::isReadable(client_certificate_file_).ok()) {
      LOG(WARNING) << "Cannot read TLS client certificate: "
                   << client_certificate_file_;
    } else if (!osquery::isReadable(client_private_key_file_).ok()) {
      LOG(WARNING) << "Cannot read TLS client private key: "
                   << client_private_key_file_;
    } else {
      options.openssl_certificate_file(client_certificate_file_);
      options.openssl_private_key_file(client_private_key_file_);
    }
  }

  // 'Optionally', though all TLS plugins should set a hostname, supply an SNI
  // hostname. This will reveal the requested domain.
  if (options_.count("hostname")) {
    // Boost cpp-netlib will only support SNI in versions >= 0.12
    options.openssl_sni_hostname(options_.get<std::string>("hostname"));
  }

  http::client client(options);
  return client;
}

inline bool tlsFailure(const std::string& what) {
  if (what.find("Error") == 0 || what.find("refused") != std::string::npos) {
    return false;
  }
  return true;
}

Status TLSTransport::sendRequest() {
  if (destination_.find("https://") == std::string::npos) {
    return Status(1, "Cannot create TLS request for non-HTTPS protocol URI");
  }

  auto client = getClient();
  http::client::request r(destination_);
  decorateRequest(r);

  VLOG(1) << "TLS/HTTPS GET request to URI: " << destination_;
  try {
    response_ = client.get(r);
    const auto& response_body = body(response_);
    if (FLAGS_verbose && FLAGS_tls_dump) {
      fprintf(stdout, "%s\n", std::string(response_body).c_str());
    }
    response_status_ =
        serializer_->deserialize(response_body, response_params_);
  } catch (const std::exception& e) {
    return Status((tlsFailure(e.what())) ? 2 : 1,
                  std::string("Request error: ") + e.what());
  }
  return response_status_;
}

Status TLSTransport::sendRequest(const std::string& params, bool compress) {
  if (destination_.find("https://") == std::string::npos) {
    return Status(1, "Cannot create TLS request for non-HTTPS protocol URI");
  }

  auto client = getClient();
  http::client::request r(destination_);
  decorateRequest(r);
  if (compress) {
    // Later, when posting/putting, the data will be optionally compressed.
    r << boost::network::header("Content-Encoding", "gzip");
  }

  // Allow request calls to override the default HTTP POST verb.
  HTTPVerb verb = HTTP_POST;
  if (options_.count("_verb") > 0) {
    verb = (HTTPVerb)options_.get<int>("_verb", HTTP_POST);
  }

  VLOG(1) << "TLS/HTTPS " << ((verb == HTTP_POST) ? "POST" : "PUT")
          << " request to URI: " << destination_;
  if (FLAGS_verbose && FLAGS_tls_dump) {
    fprintf(stdout, "%s\n", params.c_str());
  }

  try {
    if (verb == HTTP_POST) {
      response_ = client.post(r, (compress) ? compressString(params) : params);
    } else {
      response_ = client.put(r, (compress) ? compressString(params) : params);
    }

    const auto& response_body = body(response_);
    if (FLAGS_verbose && FLAGS_tls_dump) {
      fprintf(stdout, "%s\n", std::string(response_body).c_str());
    }
    response_status_ =
        serializer_->deserialize(response_body, response_params_);
  } catch (const std::exception& e) {
    return Status((tlsFailure(e.what())) ? 2 : 1,
                  std::string("Request error: ") + e.what());
  }
  return response_status_;
}
}
