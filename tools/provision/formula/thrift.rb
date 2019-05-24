require File.expand_path("../Abstract/abstract-osquery-formula", __FILE__)

class Thrift < AbstractOsqueryFormula
  desc "Framework for scalable cross-language services development"
  homepage "https://thrift.apache.org/"
  license "Apache-2.0"
  url "https://github.com/apache/thrift/archive/0.11.0.tar.gz"
  sha256 "0e324569321a1b626381baabbb98000c8dd3a59697292dbcc71e67135af0fefd"
  revision 200

  bottle do
    root_url "https://osquery-packages.s3.amazonaws.com/bottles"
    cellar :any_skip_relocation
    sha256 "4d556e7005dadd0e0035fa929a25b3e616ab3903ec47288a2138121180c88633" => :sierra
    sha256 "cfdee5e7bd1d4ff3b0ead4b686620ad5784bfd5f6a51f86543729bd351002ea9" => :x86_64_linux
  end

  depends_on "bison" => :build
  depends_on "openssl"

  patch :DATA

  def install
    ENV.cxx11
    ENV["PY_PREFIX"] = prefix
    ENV.append "CPPFLAGS", "-DOPENSSL_NO_SSL3"

    exclusions = [
      "--without-ruby",
      "--disable-tests",
      "--without-php_extension",
      "--without-haskell",
      "--without-java",
      "--without-perl",
      "--without-php",
      "--without-erlang",
      "--without-go",
      "--without-qt",
      "--without-qt4",
      "--without-nodejs",
      "--with-python",
      "--with-cpp",
      "--with-openssl=#{Formula["osquery/osquery-local/openssl"].prefix}"
    ]

    ENV.prepend_path "PATH", Formula["bison"].bin
    system "./bootstrap.sh"
    system "./configure", "--disable-debug",
                          "--prefix=#{prefix}",
                          "--libdir=#{lib}",
                          "--disable-shared",
                          "--enable-static",
                          *exclusions
    system "make", "-j#{ENV.make_jobs}"
    ENV.delete "MACOSX_DEPLOYMENT_TARGET"
    system "make", "install"
  end
end

__END__
diff --git a/lib/cpp/src/thrift/transport/TServerSocket.cpp b/lib/cpp/src/thrift/transport/TServerSocket.cpp
index 87b6383..447c89d 100644
--- a/lib/cpp/src/thrift/transport/TServerSocket.cpp
+++ b/lib/cpp/src/thrift/transport/TServerSocket.cpp
@@ -584,6 +584,12 @@ shared_ptr<TTransport> TServerSocket::acceptImpl() {
         // a certain number
         continue;
       }
+
+      // Special case because we expect setuid syscalls in other threads.
+      if (THRIFT_GET_SOCKET_ERROR == EINTR) {
+        continue;
+      }
+
       int errno_copy = THRIFT_GET_SOCKET_ERROR;
       GlobalOutput.perror("TServerSocket::acceptImpl() THRIFT_POLL() ", errno_copy);
       throw TTransportException(TTransportException::UNKNOWN, "Unknown", errno_copy);
diff --git a/configure.ac b/configure.ac
index 6a7a1a5..8b4ddc2 100755
--- a/configure.ac
+++ b/configure.ac
@@ -307,12 +307,14 @@ AM_CONDITIONAL(WITH_TWISTED_TEST, [test "$have_trial" = "yes"])
 # It's distro specific and far from ideal but needed to cross test py2-3 at once.
 # TODO: find "python2" if it's 3.x
 have_py3="no"
+if test "$with_py3" = "yes";  then
 if python --version 2>&1 | grep -q "Python 2"; then
   AC_PATH_PROGS([PYTHON3], [python3 python3.5 python35 python3.4 python34])
   if test -n "$PYTHON3"; then
     have_py3="yes"
   fi
 fi
+fi
 AM_CONDITIONAL(WITH_PY3, [test "$have_py3" = "yes"])
 
 AX_THRIFT_LIB(perl, [Perl], yes)
