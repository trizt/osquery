require File.expand_path("../Abstract/abstract-osquery-formula", __FILE__)

class Boost < AbstractOsqueryFormula
  desc "Collection of portable C++ source libraries"
  homepage "https://www.boost.org/"
  url "https://downloads.sourceforge.net/project/boost/boost/1.63.0/boost_1_63_0.tar.bz2"
  sha256 "beae2529f759f6b3bf3f4969a19c2e9d6f0c503edcb2de4a61d1428519fcb3b0"
  head "https://github.com/boostorg/boost.git"
  revision 101

  bottle do
    root_url "https://osquery-packages.s3.amazonaws.com/bottles"
    cellar :any_skip_relocation
    sha256 "4c0c142976220ce6c5aafe451e5a92905e68468ab0bfd937c1396aa200efea40" => :sierra
    sha256 "cbf4d81d465ab3453f9e0bb1d13babf74cf1a625782a5d1e4629a65864df42e0" => :x86_64_linux
  end

  env :userpaths

  option :universal

  # Keep this option, but force C++11.
  option :cxx11
  needs :cxx11

  depends_on "bzip2" unless OS.mac?

  def install
    ENV.cxx11
    ENV.universal_binary if build.universal?

    # Force boost to compile with the desired compiler
    open("user-config.jam", "a") do |file|
      if OS.mac?
        file.write "using darwin : : #{ENV.cxx} ;\n"
      else
        file.write "using gcc : : #{ENV.cxx} ;\n"
      end
    end

    # libdir should be set by --prefix but isn't
    bootstrap_args = [
      "--prefix=#{prefix}",
      "--libdir=#{lib}",
    ]

    # layout should be synchronized with boost-python
    args = [
      "--prefix=#{prefix}",
      "--libdir=#{lib}",
      "-d2",
      "-j#{ENV.make_jobs}",
      "--layout=tagged",
      "--ignore-site-config",
      "--user-config=user-config.jam",
      "--disable-icu",
      "--with-filesystem",
      "--with-regex",
      "--with-system",
      "--with-thread",
      "--with-coroutine2",
      "--with-context",
      "threading=multi",
      "link=static",
      "optimization=space",
      "variant=release",
    ]

    # Trunk starts using "clang++ -x c" to select C compiler which breaks C++11
    # handling using ENV.cxx11. Using "cxxflags" and "linkflags" still works.
    if build.cxx11? or true
      args << "cxxflags=-std=c++11 -fpic"
      #if ENV.compiler == :clang and OS.mac?
      #  #args << "cxxflags=-stdlib=libc++" << "linkflags=-stdlib=libc++"
      #end
    end

    # Fix error: bzlib.h: No such file or directory
    # and /usr/bin/ld: cannot find -lbz2
    args += [
      "include=#{HOMEBREW_PREFIX}/include",
      "linkflags=-L#{HOMEBREW_PREFIX}/lib"] unless OS.mac?

    system "./bootstrap.sh", *bootstrap_args
    system "./b2", "headers"
    system "./b2", "install", *args
  end
end
