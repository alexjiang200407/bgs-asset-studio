#!/bin/sh

if [ "$#" -lt 1 ]; then
    echo "usage: $0 [module1] [module2] ..."
    exit 1
fi

for module in "$@"; do

    mkdir -p "$module/cmake/"

    echo "set(SOURCES" > "$module/cmake/sourcelist.cmake"
    find "./$module" -type f -name '*.cpp' -printf '%P\n' | sed 's/^/\t/g' >> "$module/cmake/sourcelist.cmake"
    echo ")" >> "$module/cmake/sourcelist.cmake"

    echo "set(HEADERS" > "$module/cmake/headerlist.cmake"
    find "./$module" -type f -name '*.h' -printf '%P\n' | sed 's/^/\t/g' >> "$module/cmake/headerlist.cmake"
    echo ")" >> "$module/cmake/headerlist.cmake"

done