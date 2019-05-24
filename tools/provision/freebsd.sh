#!/usr/bin/env bash

#  Copyright (c) 2014-present, Facebook, Inc.
#  All rights reserved.
#
#  This source code is licensed under both the Apache 2.0 license (found in the
#  LICENSE file in the root directory of this source tree) and the GPLv2 (found
#  in the COPYING file in the root directory of this source tree).
#  You may select, at your option, one of the above-listed licenses.

function distro_main() {
  do_sudo pkg update
  do_sudo pkg upgrade -y

  # Use it with caution -- unless the dependency is newly added
  # and the pre-built package is not ready yet.
  # do_sudo portsnap --interactive fetch update

  # Build requirements.
  package gmake
  package cmake
  package git
  package python
  package py27-pip

  # Core development requirements.
  package glog
  package thrift
  package thrift-cpp
  package boost-libs
  package magic
  package rocksdb-lite
  package rapidjson
  package zstd
  package linenoise-ng

  # Non-optional features.
  package augeas
  package ssdeep

  # Optional features.
  package sleuthkit
  package yara
  package aws-sdk-cpp
  package lldpd

  # For testing
  package valgrind
}
