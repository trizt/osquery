require File.expand_path("../Abstract/abstract-osquery-formula", __FILE__)

class UtilLinux < AbstractOsqueryFormula
  desc "Collection of Linux utilities"
  homepage "https://github.com/karelzak/util-linux"
  url "https://www.kernel.org/pub/linux/utils/util-linux/v2.27/util-linux-2.27.1.tar.xz"
  sha256 "0a818fcdede99aec43ffe6ca5b5388bff80d162f2f7bd4541dca94fecb87a290"
  head "https://github.com/karelzak/util-linux.git"
  revision 101

  bottle do
    root_url "https://osquery-packages.s3.amazonaws.com/bottles"
    cellar :any_skip_relocation
    sha256 "44b55c155b23c6f95e21b318a5a6ff59317ba3ce23e881647bc6736703d4c396" => :x86_64_linux
  end

  def install
    system "./autogen.sh"
    system "./configure",
      "--disable-debug",
      "--disable-dependency-tracking",
      "--disable-silent-rules",
      "--prefix=#{prefix}",
      # Fix chgrp: changing group of 'wall': Operation not permitted
      "--disable-use-tty-group",
      # Conflicts with coreutils.
      "--disable-kill",
      "--disable-shared",
      "--enable-static"
    system "make", "install"
  end

  test do
    assert_match "January", shell_output("#{bin}/cal 1 2016")
  end
end
