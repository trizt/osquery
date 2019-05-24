require File.expand_path("../Abstract/abstract-osquery-formula", __FILE__)

class Curl < AbstractOsqueryFormula
  desc "Get a file from an HTTP, HTTPS or FTP server"
  homepage "https://curl.haxx.se/"
  url "https://curl.haxx.se/download/curl-7.48.0.tar.bz2"
  sha256 "864e7819210b586d42c674a1fdd577ce75a78b3dda64c63565abe5aefd72c753"
  revision 101

  bottle do
    root_url "https://osquery-packages.s3.amazonaws.com/bottles"
    cellar :any_skip_relocation
    sha256 "531eff9d3feb701a403322de8e0b1497e0acecad0356874bb8a7101a9f430048" => :x86_64_linux
  end

  keg_only :provided_by_osx

  option "with-libidn", "Build with support for Internationalized Domain Names"
  option "with-rtmpdump", "Build with RTMP support"
  option "with-libssh2", "Build with scp and sftp support"
  option "with-c-ares", "Build with C-Ares async DNS support"
  option "with-gssapi", "Build with GSSAPI/Kerberos authentication support."
  option "with-libmetalink", "Build with libmetalink support."
  option "with-libressl", "Build with LibreSSL instead of Secure Transport or OpenSSL"
  option "with-nghttp2", "Build with HTTP/2 support (requires OpenSSL or LibreSSL)"

  deprecated_option "with-idn" => "with-libidn"
  deprecated_option "with-rtmp" => "with-rtmpdump"
  deprecated_option "with-ssh" => "with-libssh2"
  deprecated_option "with-ares" => "with-c-ares"

  # HTTP/2 support requires OpenSSL 1.0.2+ or LibreSSL 2.1.3+ for ALPN Support
  # which is currently not supported by Secure Transport (DarwinSSL).
  if MacOS.version < :mountain_lion || (build.with?("nghttp2") && build.without?("libressl"))
    depends_on "openssl"
  else
    option "with-openssl", "Build with OpenSSL instead of Secure Transport"
    depends_on "openssl" => :optional
  end

  depends_on "pkg-config" => :build
  depends_on "libidn" => :optional
  depends_on "rtmpdump" => :optional
  depends_on "libssh2" => :optional
  depends_on "c-ares" => :optional
  depends_on "libmetalink" => :optional
  depends_on "libressl" => :optional
  depends_on "nghttp2" => :optional
  depends_on "homebrew/dupes/krb5" if build.with?("gssapi") && !OS.mac?
  depends_on "homebrew/dupes/openldap" => :optional unless OS.mac?

  # This patch fixes compile against LibreSSL. From:
  # https://github.com/curl/curl/commit/240cd84b494e0ff
  # https://github.com/curl/curl/commit/23ab4816443e2b9
  # Can be removed on the next release.
  patch :DATA

  def install
    # Throw an error if someone actually tries to rock both SSL choices.
    # Long-term, make this singular-ssl-option-only a requirement.
    if build.with?("libressl") && build.with?("openssl")
      ohai <<-EOS.undent
      --with-openssl and --with-libressl are both specified and
      curl can only use one at a time; proceeding with libressl.
      EOS
    end

    args = %W[
      --disable-debug
      --disable-dependency-tracking
      --disable-silent-rules
      --prefix=#{prefix}
    ]

    # cURL has a new firm desire to find ssl with PKG_CONFIG_PATH instead of using
    # "--with-ssl" any more. "when possible, set the PKG_CONFIG_PATH environment
    # variable instead of using this option". Multi-SSL choice breaks w/o using it.
    if build.with? "libressl"
      ENV.prepend_path "PKG_CONFIG_PATH", "#{Formula["libressl"].opt_prefix}/lib/pkgconfig"
      args << "--with-ssl=#{Formula["libressl"].opt_prefix}"
      args << "--with-ca-bundle=#{etc}/libressl/cert.pem"
    elsif MacOS.version < :mountain_lion || build.with?("openssl") || build.with?("nghttp2")
      ENV.prepend_path "PKG_CONFIG_PATH", "#{Formula["openssl"].opt_prefix}/lib/pkgconfig"
      args << "--with-ssl=#{Formula["openssl"].opt_prefix}"
      args << "--with-ca-bundle=#{etc}/openssl/cert.pem"
    else
      args << "--with-darwinssl"
    end

    args << (build.with?("libssh2") ? "--with-libssh2" : "--without-libssh2")
    args << (build.with?("libidn") ? "--with-libidn" : "--without-libidn")
    args << (build.with?("libmetalink") ? "--with-libmetalink" : "--without-libmetalink")
    args << (build.with?("gssapi") ? "--with-gssapi" : "--without-gssapi")
    args << (build.with?("rtmpdump") ? "--with-librtmp" : "--without-librtmp")

    if build.with? "c-ares"
      args << "--enable-ares=#{Formula["c-ares"].opt_prefix}"
    else
      args << "--disable-ares"
    end
    args << "--disable-ldap" if build.without? "openldap"

    ENV["PKG_CONFIG"] = "pkg-config --static"
    ENV.append "LDFLAGS", "-static"
    system "./configure", *args
    system "make", "curl_LDFLAGS=-all-static"
    system "make", "install"
  end

  test do
    # Fetch the curl tarball and see that the checksum matches.
    # This requires a network connection, but so does Homebrew in general.
    filename = (testpath/"test.tar.gz")
    system "#{bin}/curl", "-L", stable.url, "-o", filename
    filename.verify_checksum stable.checksum
  end
end

__END__
diff --git a/lib/vtls/openssl.c b/lib/vtls/openssl.c
index cbf2d21..f8ccb23 100644
--- a/lib/vtls/openssl.c
+++ b/lib/vtls/openssl.c
@@ -95,7 +95,8 @@

 #if (OPENSSL_VERSION_NUMBER >= 0x10000000L)
 #define HAVE_ERR_REMOVE_THREAD_STATE 1
-#if (OPENSSL_VERSION_NUMBER >= 0x10100004L)
+#if (OPENSSL_VERSION_NUMBER >= 0x10100004L) && \
+  !defined(LIBRESSL_VERSION_NUMBER)
 /* OpenSSL 1.1.0-pre4 removed the argument! */
 #define HAVE_ERR_REMOVE_THREAD_STATE_NOARG 1
 #endif
