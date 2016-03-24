#!/bin/bash

find . -type f -name '*.h' -exec sed --in-place 's/[[:space:]]\+$//' {} \+
find . -type f -name '*.cpp' -exec sed --in-place 's/[[:space:]]\+$//' {} \+
