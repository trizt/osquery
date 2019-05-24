require File.expand_path("../Abstract/abstract-osquery-formula", __FILE__)

class Openssl < AbstractOsqueryFormula
  desc "SSL/TLS cryptography library"
  homepage "https://openssl.org/"
  url "https://www.openssl.org/source/openssl-1.0.2k.tar.gz"
  mirror "https://dl.bintray.com/homebrew/mirror/openssl-1.0.2k.tar.gz"
  mirror "https://www.mirrorservice.org/sites/ftp.openssl.org/source/openssl-1.0.2k.tar.gz"
  sha256 "6b3977c61f2aedf0f96367dcfb5c6e578cf37e7b8d913b4ecb6643c3cb88d8c0"
  revision 100

  bottle do
    root_url "https://osquery-packages.s3.amazonaws.com/bottles"
    cellar :any_skip_relocation
    sha256 "967beaed429296f369d02fdc93dcef4af2cdc5dfa44e18173cde349f4f8e87bd" => :sierra
    sha256 "e37e82ba5a5e96cfc7f902aa3a2b1a52b96c5579397e6aabd960666b03be0b95" => :x86_64_linux
  end

  resource "cacert" do
    # Update post_install when you update this resource.
    # homepage "http://curl.haxx.se/docs/caextract.html"
    url "https://curl.haxx.se/ca/cacert-2017-01-18.pem"
    sha256 "e62a07e61e5870effa81b430e1900778943c228bd7da1259dd6a955ee2262b47"
  end

  option "without-test", "Skip build-time tests (not recommended)"

  deprecated_option "without-check" => "without-test"

  depends_on "makedepend" => :build
  depends_on "zlib" unless OS.mac?
  depends_on :perl => ["5.0", :build] unless OS.mac?

  def arch_args
    if OS.linux?
      %w[linux-x86_64]
    else
      %w[darwin64-x86_64-cc enable-ec_nistp_64_gcc_128]
    end
  end

  def configure_args
    args = [
      "--prefix=#{prefix}",
      "--openssldir=#{openssldir}",
      "no-ssl2",
      "no-ssl3",
      "no-asm",
      "no-shared",
      "no-weak-ssl-ciphers",
      "zlib-dynamic",
      "enable-cms",
    ]
    if OS.linux?
      args += [
        ENV.cppflags,
        ENV.cflags,
        ENV.ldflags,
      ]
    end
    return args
  end

  def install
    # Load zlib from an explicit path instead of relying on dyld's fallback
    # path, which is empty in a SIP context. This patch will be unnecessary
    # when we begin building openssl with no-comp to disable TLS compression.
    # https://langui.sh/2015/11/27/sip-and-dlopen
    inreplace "crypto/comp/c_zlib.c",
              'zlib_dso = DSO_load(NULL, "z", NULL, 0);',
              'zlib_dso = DSO_load(NULL, "/usr/lib/libz.dylib", NULL, DSO_FLAG_NO_NAME_TRANSLATION);' if OS.mac?

    ENV.deparallelize
    system "perl", "./Configure", *(configure_args + arch_args)
    system "make", "depend"
    system "make"
    system "make", "test" if build.with?("test")
    system "make", "install", "MANDIR=#{man}", "MANSUFFIX=ssl"
  end

  def openssldir
    etc/"openssl"
  end

  def post_install
    ENV.delete "LIBRARY_PATH"
    (etc/"openssl").install resource("cacert").files("cacert-2017-01-18.pem" => "cert.pem")
  end

  def caveats; <<-EOS.undent
    A CA file has been bootstrapped using certificates from the system
    keychain. To add additional certificates, place .pem files in
      #{openssldir}/certs

    and run
      #{opt_bin}/c_rehash
    EOS
  end

  test do
    # Make sure the necessary .cnf file exists, otherwise OpenSSL gets moody.
    assert (HOMEBREW_PREFIX/"etc/openssl/openssl.cnf").exist?,
            "OpenSSL requires the .cnf file for some functionality"

    # Check OpenSSL itself functions as expected.
    (testpath/"testfile.txt").write("This is a test file")
    expected_checksum = "e2d0fe1585a63ec6009c8016ff8dda8b17719a637405a4e23c0ff81339148249"
    system "#{bin}/openssl", "dgst", "-sha256", "-out", "checksum.txt", "testfile.txt"
    open("checksum.txt") do |f|
      checksum = f.read(100).split("=").last.strip
      assert_equal checksum, expected_checksum
    end
  end
end
