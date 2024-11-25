#!/usr/bin/env bash
BUILD_DATE=`date +"%Y%m%d_%H%M%S"`
GIT_HASH=`git rev-parse HEAD`
GIT_BRANCH=`git branch --show-current`
VERSION="${1:-TESTING}"
hwplatform=`uname -i`
[[ "${hwplatform}" == "aarch64" ]] && PATHLIB="/usr/lib/aarch64-linux-gnu" && ARCH="aarch64"
[[ "${hwplatform}" == "armv7l" ]] && PATHLIB="/usr/lib/arm-linux-gnueabihf" && ARCH="armhf"

gcc hacksdl.c configuration.c debug.c  "${PATHLIB}/libconfig.a" -o "hacksdl.${ARCH}.so" -fPIC -shared -lSDL2 -D_GNU_SOURCE -Wl,--defsym,BUILD_DATE_${BUILD_DATE}=0 -Wl,--defsym,GIT_HASH_${GIT_HASH}=0 -Wl,--defsym,GIT_BRANCH_${GIT_BRANCH}=0 -Wl,--defsym,VERSION_${VERSION}=0

cp "hacksdl.${ARCH}.so" "hacksdl.so"
strip "hacksdl.so"
zip "hacksdl-${ARCH}-${VERSION}.zip" "hacksdl.so"
rm "hacksdl.so"