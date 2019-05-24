require File.expand_path("../Abstract/abstract-osquery-formula", __FILE__)

class Cmake < AbstractOsqueryFormula
  desc "Cross-platform make"
  homepage "https://www.cmake.org/"
  url "https://cmake.org/files/v3.9/cmake-3.9.1.tar.gz"
  sha256 "d768ee83d217f91bb597b3ca2ac663da7a8603c97e1f1a5184bc01e0ad2b12bb"
  revision 100

  head "https://cmake.org/cmake.git"

  bottle do
    root_url "https://osquery-packages.s3.amazonaws.com/bottles"
    cellar :any_skip_relocation
    sha256 "aa6d4acaf60b8a6ab32d925ae56ab5f8c886857f382ce85143542047fa3d1110" => :sierra
    sha256 "3bf8a820fabc6b9b7bbca8c65ef633c7fb7e7950bbdcb66d055c26bc1218080e" => :x86_64_linux
  end

  # The `with-qt` GUI option was removed due to circular dependencies if
  # CMake is built with Qt support and Qt is built with MySQL support as MySQL uses CMake.
  # For the GUI application please instead use `brew cask install cmake`.

  patch :DATA

  def install
    args = %W[
      --prefix=#{prefix}
      --no-system-libs
      --parallel=#{ENV.make_jobs}
      --datadir=/share/cmake
      --docdir=/share/doc/cmake
      --mandir=/share/man
      --system-zlib
      --system-bzip2
      --system-liblzma
    ]

    # Keep these for initial compiler detection
    # This initial detection does not use LDFLAGS.
    ENV.append "CXXFLAGS", "-L#{legacy_prefix}/lib"
    ENV.append "CXXFLAGS", "-L#{default_prefix}/lib"
    ENV.append "CXXFLAGS", "-lrt -lpthread" if OS.linux?
    ENV["LD_LIBRARY_PATH"] = default_prefix/"lib"

    system "./bootstrap", *args
    system "make"
    system "make", "install"

    elisp.install "Auxiliary/cmake-mode.el"
  end

  test do
    (testpath/"CMakeLists.txt").write("find_package(Ruby)")
    system bin/"cmake", "."
  end
end

__END__
diff --git a/Utilities/cmlibuv/CMakeLists.txt b/Utilities/cmlibuv/CMakeLists.txt
index 4c8e228..9605d6a 100644
--- a/Utilities/cmlibuv/CMakeLists.txt
+++ b/Utilities/cmlibuv/CMakeLists.txt
@@ -175,7 +175,7 @@ if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
 endif()
 
 if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
-  list(APPEND uv_libraries dl rt)
+  list(APPEND uv_libraries dl rt pthread)
   list(APPEND uv_headers
     include/uv-linux.h
     )
