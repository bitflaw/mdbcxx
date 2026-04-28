#!/usr/bin/env bash

set -e

# trap './teardown.sh' EXIT

source ./setup.sh
./../../build/tests
