#!/usr/bin/env bash
# Builds fitsToKdb.so against conda-forge cfitsio and installs it with init.q.
set -euo pipefail

cd "${SRC_DIR:-$(cd "$(dirname "$0")/.." && pwd)}"

# cfitsio comes from the host env; pkg-config resolves it under $PREFIX.
CFLAGS_CFITSIO="$(pkg-config --cflags cfitsio)"
LIBS_CFITSIO="$(pkg-config --libs cfitsio)"

${CC:-cc} -O2 -fPIC -shared \
  ${CFLAGS_CFITSIO} \
  -I./include \
  fitsToKdb.c \
  -o fitsToKdb.so \
  ${LIBS_CFITSIO}

# Installed under share/fd.cfitsiokdb; loaded in q with use`fd.cfitsiokdb`.
INSTALL_DIR="${PREFIX}/share/fd.cfitsiokdb"
mkdir -p "${INSTALL_DIR}"
install -m 0755 fitsToKdb.so "${INSTALL_DIR}/fitsToKdb.so"
install -m 0644 init.q "${INSTALL_DIR}/init.q"
