#!/usr/bin/env bash

set -e
ERR='\033[0;31m'
OK='\033[0;32m'
INFO='\033[0;34m'
WARN='\033[0;33m'
RESET='\033[0m'

echo -e "[${INFO}INFO${RESET}] Cleaning up test resources..."

docker stop ${TDB_NAME}
docker rm -f ${TDB_NAME}
# docker rmi mariadb:latest

echo -e "[${INFO}INFO${RESET}] Completed clean-up! Bye"
