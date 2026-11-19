#!/usr/bin/env bash
# Builds the fitsToKdb shared library against conda-forge cfitsio and installs it
# into the q module path so that `use`fd.cfitsiokdb` resolves.
set -euo pipefail

cd "${SRC_DIR:-$(cd "$(dirname "$0")/.." && pwd)}"

# cfitsio comes from the host env; pkg-config resolves it under $PREFIX.
CFLAGS_CFITSIO="$(pkg-config --cflags cfitsio)"
LIBS_CFITSIO="$(pkg-config --libs cfitsio)"

${CC:-cc} -O2 -fPIC -shared \
  ${CFLAGS_CFITSIO} \
  -I./include \
  fitsToKdb.c \
  -o clib.li64.so \
  ${LIBS_CFITSIO}

INSTALL_DIR="${PREFIX}/lib/q/mod/fd/cfitsiokdb"
mkdir -p "${INSTALL_DIR}"
install -m 0755 clib.li64.so "${INSTALL_DIR}/clib.li64.so"
install -m 0644 init.q "${INSTALL_DIR}/init.q"
