#!/usr/bin/env bash

set -e

sudo ./setup.sh
./../../build/tests
sudo -E ./teardown.sh
