#!/bin/bash

#  Copyright (c) 2015, Facebook, Inc.
#  All rights reserved.
#
#  This source code is licensed under the BSD-style license found in the
#  LICENSE file in the root directory of this source tree. An additional grant
#  of patent rights can be found in the PATENTS file in the same directory.

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

DARWIN_SETUP="\
if [[ ! -f /var/.osquery_build ]]; then \
touch /tmp/.com.apple.dt.CommandLineTools.installondemand.in-progress; \
PROD=\$(softwareupdate -l | grep \"\\*.*Command Line\" | \
  head -n 1 | awk -F\"*\" '{print \$2}' | sed -e 's/^ *//' | tr -d '\n' \
); \
softwareupdate -i \"\$PROD\" --verbose; \
sudo touch /var/.osquery_build; \
fi; \
"

DARWIN_BOX="macos10.12"
LINUX_BOX="ubuntu16.04"

function usage() {
  echo "${BASH_SOURCE[0]} VERSION PATH_TO_OSQUERY"
}

function main() {
  if [[ $# < 2 ]]; then
    usage
    exit 1
  fi

  CURRENT_DIR=$(pwd)
  VERSION=$1
  OSQUERY=$2

  PACKAGES_CMD="cd /build; make packages;"
  BUILD_CMD="\
    sudo install -d -o vagrant /build; cd /build; \
    git clone https://github.com/facebook/osquery . || true; \
    git checkout master && git pull; \
    git checkout $VERSION; \
    make sysprep; SKIP_TESTS=1 make -j 4; \
  "

  cd $OSQUERY
  echo "[+] Checking out version $VERSION"

  PKG_DIR="build/$VERSION"
  mkdir -p $PKG_DIR

  echo "[+] Vagrant up $LINUX_BOX"
  OSQUERY_BUILD_CPUS=4 vagrant up $LINUX_BOX
  echo "[+] Building linux packages..."
  vagrant ssh $LINUX_BOX -c "$BUILD_CMD"
  echo "[+] Running package build command for linux..."
  vagrant ssh $LINUX_BOX -c "$PACKAGES_CMD"
  echo "[+] Copying linux packages to $PKG_DIR"
  vagrant scp "$LINUX_BOX:/build/build/linux/osquery*$VERSION*" ./$PKG_DIR
  vagrant halt $LINUX_BOX

  echo "[+] Vagrant up $DARWIN_BOX"
  OSQUERY_BUILD_CPUS=4 vagrant up $DARWIN_BOX
  echo "[+] Running initial softwareupdate check..."
  vagrant ssh $DARWIN_BOX -c "$DARWIN_SETUP"
  echo "[+] Running build command for macOS..."
  vagrant ssh $DARWIN_BOX -c "$BUILD_CMD"
  vagrant ssh $DARWIN_BOX -c "/usr/local/osquery/bin/brew install rpm"
  echo "[+] Running package build command for macOS..."
  vagrant ssh $DARWIN_BOX -c "$PACKAGES_CMD"
  echo "[+] Copying macOS packages to $PKG_DIR"
  vagrant scp "$DARWIN_BOX:/build/build/darwin/osquery*$VERSION*" ./$PKG_DIR
  vagrant halt $DARWIN_BOX

  echo "[+] Packages copied to $OSQUERY ./$PKG_DIR"
  echo "[+] Finished"
  cd $CURRENT_DIR
}

main $@
