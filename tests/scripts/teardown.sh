#!/usr/bin/env bash

set -e
ERR='\033[0;31m'
OK='\033[0;32m'
INFO='\033[0;34m'
WARN='\033[0;33m'

echo -e "${INFO}[INFO]${INFO} Cleaning up test resources..."

docker stop ${TDB_NAME}
docker rm -f ${TDB_NAME}
docker rmi mariadb:latest

echo -e "${INFO}[INFO]${INFO} Completed clean-up! Bye"
