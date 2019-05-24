#!/usr/bin/env bash

#  Copyright (c) 2014-present, Facebook, Inc.
#  All rights reserved.
#
#  This source code is licensed under both the Apache 2.0 license (found in the
#  LICENSE file in the root directory of this source tree) and the GPLv2 (found
#  in the COPYING file in the root directory of this source tree).
#  You may select, at your option, one of the above-listed licenses.

function distro_main() {
  do_sudo apt-get -y update

  package gawk
  package autotools-dev
  package autopoint
  package g++
  package ruby
  package ruby-dev
  package curl
  package bison
  package flex
  package bsdtar
  package realpath
  package doxygen
  package valgrind

  GEM=`which gem`
  do_sudo $GEM install --no-ri --no-rdoc fpm
}
