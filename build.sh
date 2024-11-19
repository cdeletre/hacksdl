#!/usr/bin/env bash
BUILD_DATE=`date +"%Y%m%d_%H%M%S"`
GIT_HASH=`git rev-parse HEAD`
GIT_BRANCH=`git branch --show-current`
gcc hacksdl.c configuration.c debug.c  /usr/lib/aarch64-linux-gnu/libconfig.a -o hacksdl.aarch64.so -fPIC -shared -lSDL2 -D_GNU_SOURCE -Wl,--defsym,BUILD_DATE_${BUILD_DATE}=0 -Wl,--defsym,GIT_HASH_${GIT_HASH}=0 -Wl,--defsym,GIT_BRANCH_${GIT_BRANCH}=0