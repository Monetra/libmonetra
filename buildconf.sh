#!/bin/sh

autoreconf -iv --no-recursive && \
[ -d "thirdparty/mstdlib" ] && (cd thirdparty/mstdlib && ./buildconf.sh)
