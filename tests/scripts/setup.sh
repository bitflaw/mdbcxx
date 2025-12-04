#!/usr/bin/env bash

set -e

ERR='\033[0;31m'
OK='\033[0;32m'
INFO='\033[0;34m'
WARN='\033[0;33m'

DBPORT=8000
DBPASS='tester'
DBNAME='testmdb'

echo -e "${INFO}[INFO]${INFO} Pulling database container..."

if ! sudo docker pull mariadb:latest; then
  echo -e "${ERR}[ERROR]${ERR} Failed to pull database container!"
  echo -e "${INFO}[INFO]${INFO} Cleaning up..."
  sudo docker rmi mariadb:latest
  exit
fi

if ! sudo docker run \
  -d \
  --name ${DBNAME} \
  -e MARIADB_ROOT_PASSWORD=${DBPASS} \
  -p ${DBPORT}:3306 \
  mariadb:latest\
  ; then
  echo -e "${ERR}[ERROR]${ERR} Faield to start database container! Aborting..."
  echo -e "${INFO}[INFO]${INFO} Cleaning up..."
  sudo docker rm -f ${DBNAME}
  exit
fi

export TDB_PASS=${DBPASS}
export TDB_USR='root'
export TDB_PORT=${DBPORT}
export TDB_NAME=${DBNAME}

echo -e "${INFO}[INFO]${INFO} Setup complete!"
