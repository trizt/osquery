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

#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/optional/optional.hpp>

// clang-format off
#ifdef WIN32
#pragma warning(push, 3)

/*
 * Suppressing warning C4005:
 * 'ASIO_ERROR_CATEGORY_NOEXCEPT': macro redefinition
 */
#pragma warning(disable: 4005)

/*
 * Suppressing warning C4244:
 * 'argument': conversion from '__int64' to 'long', possible loss of data
 */
#pragma warning(disable: 4244)
#endif

#include <boost/network/uri.hpp>
#include <boost/network/uri/uri_io.hpp>

#ifdef WIN32
#pragma warning(pop)

/// We need to reinclude this to re-enable boost's warning suppression
#include <boost/config/compiler/visualc.hpp>
#endif
// clang-format on

#include <openssl/crypto.h>
#include <openssl/ssl.h>

#ifndef OPENSSL_NO_SSL2
#define OPENSSL_NO_SSL2 1
#endif

#ifndef OPENSSL_NO_SSL3
#define OPENSSL_NO_SSL3 1
#endif

#define OPENSSL_NO_MD5 1
#define OPENSSL_NO_DEPRECATED 1

/// Newer versions of LibreSSL will lack SSL methods.
extern "C" {
#if defined(NO_SSL_TXT_SSLV3)
SSL_METHOD* SSLv3_server_method(void);
SSL_METHOD* SSLv3_client_method(void);
SSL_METHOD* SSLv3_method(void);
#endif
void ERR_remove_state(unsigned long);
}

namespace boost_system = boost::system;
namespace boost_asio = boost::asio;
namespace beast_http = boost::beast::http;

namespace osquery {
namespace http {

typedef boost_asio::ssl::stream<boost_asio::ip::tcp::socket&> ssl_stream;
typedef beast_http::request<beast_http::string_body> beast_http_request;
typedef beast_http::response<beast_http::string_body> beast_http_response;
typedef beast_http::response_parser<beast_http::string_body>
    beast_http_response_parser;
typedef beast_http::request_serializer<beast_http::string_body>
    beast_http_request_serializer;

template <typename T>
class HTTP_Request;
template <typename T>
class HTTP_Response;
typedef HTTP_Request<beast_http_request> Request;
typedef HTTP_Response<beast_http_response> Response;

/**
 * @brief a simple http client class based upon boost.beast.
 *
 * Implements put, post, get, head and delete methods.
 * These methods take request as reference and  return response by value.
 */
class Client {
 public:
  /**
   * @brief Client options class.
   *
   * Behavior of the Client class is driven by its options.
   * E.g. secured client or non-secured client.
   * Or if the client will talk to server via proxy.
   */
  class Options {
   public:
    Options()
        : ssl_options_(0),
          timeout_(0),
          always_verify_peer_(false),
          follow_redirects_(false) {}

    Options& follow_redirects(bool fr) {
      follow_redirects_ = fr;
      return *this;
    }

    Options& always_verify_peer(bool avp) {
      always_verify_peer_ = avp;
      return *this;
    }

    Options& timeout(int to) {
      timeout_ = to;
      return *this;
    }

    Options& openssl_ciphers(std::string const& ciphers) {
      ciphers_ = ciphers;
      return *this;
    }

    Options& openssl_options(long so) {
      ssl_options_ = so;
      return *this;
    }

    Options& openssl_verify_path(std::string const& vp) {
      verify_path_ = vp;
      return *this;
    }

    Options& openssl_certificate(std::string const& scf) {
      server_certificate_ = scf;
      return *this;
    }

    Options& openssl_certificate_file(std::string const& ccf) {
      client_certificate_file_ = ccf;
      return *this;
    }

    Options& openssl_private_key_file(std::string const& cpkf) {
      client_private_key_file_ = cpkf;
      return *this;
    }

    Options& openssl_sni_hostname(std::string const& sni_h) {
      sni_hostname_ = sni_h;
      return *this;
    }

    Options& proxy_hostname(std::string const& prxy_h) {
      proxy_hostname_ = prxy_h;
      return *this;
    }

    Options& remote_hostname(std::string const& remote_h) {
      remote_hostname_ = remote_h;
      return *this;
    }

    Options& remote_port(std::string const& remote_p) {
      remote_port_ = remote_p;
      return *this;
    }

   private:
    boost::optional<std::string> server_certificate_;
    boost::optional<std::string> verify_path_;
    boost::optional<std::string> client_certificate_file_;
    boost::optional<std::string> client_private_key_file_;
    boost::optional<std::string> ciphers_;
    boost::optional<std::string> sni_hostname_;
    boost::optional<std::string> proxy_hostname_;
    boost::optional<std::string> remote_hostname_;
    boost::optional<std::string> remote_port_;
    long ssl_options_;
    int timeout_;
    bool always_verify_peer_;
    bool follow_redirects_;
    friend class Client;
  };

 public:
  Client(Options const& opts = Options())
      : client_options_(opts), r_(ios_), sock_(ios_), timer_(ios_) {}

  /// HTTP put request method
  Response put(Request& req,
               std::string const& body,
               std::string const& content_type = std::string());

  /// HTTP put request method with rvalue reference to body param
  Response put(Request& req,
               std::string&& body,
               std::string const& content_type = std::string());

  /// HTTP post request method
  Response post(Request& req,
                std::string const& body,
                std::string const& content_type = std::string());

  /// HTTP post request method with rvalue reference to body param
  Response post(Request& req,
                std::string&& body,
                std::string const& content_type = std::string());

  /// HTTP get request method
  Response get(Request& req);

  /// HTTP head request method
  Response head(Request& req);

  /// HTTP delete_ request method
  Response delete_(Request& req);

  ~Client() {
    closeSocket();
  }

 private:
  /// Create Connection to server, if proxy option is set, connect to proxy
  void createConnection();

  /// Convert plain socket to ssl socket
  void encryptConnection();

  template <typename STREAM_TYPE>
  void sendRequest(STREAM_TYPE& stream,
                   Request& req,
                   beast_http_response_parser& resp);

  Response sendHTTPRequest(Request& req);

  /// Handles HTTP request timeout
  void timeoutHandler(boost_system::error_code const& ec);

  /// Handles respone code, if requests completes or aborted due to timeout
  void postResponseHandler(boost_system::error_code const& ec);

  void closeSocket();

 private:
  Options client_options_;
  boost_asio::io_service ios_;
  boost_asio::ip::tcp::resolver r_;
  boost_asio::ip::tcp::socket sock_;
  boost_asio::deadline_timer timer_;
  std::shared_ptr<ssl_stream> ssl_sock_;
  boost_system::error_code ec_;
  bool ssl_connection = false;
  static const long SHORT_READ_ERROR = 0x140000dbL;
  /// Setting the default port to squid proxy default port
  static const int PROXY_DEFAUT_PORT = 3128;
  static const int HTTP_DEFAULT_PORT = 80;
  static const int HTTPS_DEFAULT_PORT = 443;
};

/**
 * @brief HTTP request class
 *
 * This class is inherited from implemention(boost.beast) request class.
 * It extends the functionality via providing uri parsing.
 *
 */
template <typename T>
class HTTP_Request : public T {
 public:
  /**
   * @brief HTTP request header helper class
   *
   * constructor of this class takes (name, value)
   * pair, which is used to set the request header
   * of a http request with the help of overloaded
   * function 'operator<<' of HTTP_Request class.
   */
  struct Header {
    Header(const std::string& name, const std::string& value)
        : name_(name), value_(value) {}

   private:
    std::string name_;
    std::string value_;
    friend class HTTP_Request<T>;
  };

 public:
  HTTP_Request(const std::string& url = std::string()) : uri_(url) {}

  /// Returns the host part of a uri.
  boost::optional<std::string> remoteHost() {
    return uri_.host().size() ? uri_.host() : boost::optional<std::string>();
  }

  /// Returns the port part of a uri.
  boost::optional<std::string> remotePort() {
    return uri_.port().size() ? uri_.port() : boost::optional<std::string>();
  }

  /// Returns the path part of a uri.
  boost::optional<std::string> remotePath() {
    return uri_.path().size() ? uri_.path() : boost::optional<std::string>();
  }

  /// Returns the protocol part of a uri. E.g. 'http' or 'https'
  boost::optional<std::string> protocol() {
    return uri_.scheme().size() ? uri_.scheme()
                                : boost::optional<std::string>();
  }

  /// overloaded operator to set header of an http request
  HTTP_Request& operator<<(const Header& h) {
    this->T::set(h.name_, h.value_);
    return *this;
  }

  /// uri can also be set via this method, useful for redirected request.
  void uri(const std::string& url) {
    uri_ = url;
  }

 private:
  boost::network::uri::uri uri_;
};

/**
 * @brief HTTP response class
 *
 * This class is inherited from implementation(boost.beast) http response class.
 * This class gives convenient access to some functionality of implemention
 * specific http response class.
 *
 */
template <typename T>
class HTTP_Response : public T {
 public:
  HTTP_Response() = default;

  HTTP_Response(T&& resp) : T(std::move(resp)) {}

  template <typename ITER>
  class Iterator;
  class Headers;

  /// status of an http response
  unsigned status() {
    return this->T::result_int();
  }

  /// body of an http response
  const std::string& body() {
    return this->T::body;
  }

  /// All headers of an http response.
  /// Headers can be accesed via HTTP_Response<T>::Iterator class
  Headers headers() {
    return Headers(this);
  }
};

/**
 * @brief HTTP response headers iterator class
 *
 */
template <typename T>
template <typename ITER>
class HTTP_Response<T>::Iterator {
 public:
  Iterator(ITER iter) : iter_(iter) {}

  Iterator operator++() {
    ++iter_;
    return *this;
  }

  Iterator operator++(int) {
    Iterator tmp = *this;
    ++*this;
    return tmp;
  }

  bool operator!=(const Iterator& it) const {
    return (iter_ != it.iter_);
  }

  auto operator-> () {
    return std::make_shared<std::pair<std::string, std::string>>(
        std::string(iter_->name_string()), std::string(iter_->value()));
  }

  auto operator*() {
    return std::make_pair(std::string(iter_->name_string()),
                          std::string(iter_->value()));
  }

 private:
  ITER iter_;
};

/**
 * @brief HTTP response headers helper class
 *
 * This class gives convenient access to all the
 * headers of http response.
 *
 * e.g. -
 * for (const auto& header : resp.headers()) {
 *   header.first;
 *   header.second;
 * }
 */
template <typename T>
class HTTP_Response<T>::Headers {
 public:
  Headers(T* resp) : resp_(resp) {}

  std::string operator[](const std::string& name) {
    return std::string(resp_->T::operator[](name));
  }

  auto begin() {
    return Iterator<decltype(resp_->T::begin())>(resp_->T::begin());
  }

  auto end() {
    return Iterator<decltype(resp_->T::end())>(resp_->T::end());
  }

 private:
  T* resp_;
};
}
}
