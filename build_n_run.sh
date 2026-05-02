#!/usr/bin/bash
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd "${SCRIPT_DIR}"

mkdir -p .build
cd .build

cmake ..
if [[ $? -ne 0 ]]; then
    echo "cmake fail"
    exit 1
fi

make -j$(nproc)
if [[ $? -ne 0 ]]; then
    echo "build fail"
    exit 1
fi

./"cmake_cpp_project_template" "$@"
