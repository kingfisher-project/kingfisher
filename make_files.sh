#!/usr/bin/env bash

# SPDX-License-Identifier: Unlicense

set -e

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <filename>"
    echo "Example: $0 widget"
    exit 1
fi

NAME="$1"

# Convert filename to a header guard
GUARD=$(printf "KINGFISHER_%s_H_" "$NAME" \
    | tr '/[:lower:]-' '_[:upper:]_')

mkdir -p \
    "include/$(dirname "$NAME")" \
    "src/$(dirname "$NAME")" \
    "tests/$(dirname "$NAME")"

cat > "include/${NAME}.h" <<EOF
// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#ifndef ${GUARD}
#define ${GUARD}

namespace kingfisher {

}

#endif // ${GUARD}
EOF

cat > "src/${NAME}.cc" <<EOF
#include "${NAME}.h"

EOF

cat > "tests/${NAME}_test.cc" <<EOF
#include <gtest/gtest.h>

#include "${NAME}.h"

EOF

echo "Generated:"
echo "  include/${NAME}.h"
echo "  src/${NAME}.cc"
echo "  tests/${NAME}_tests.cc"

