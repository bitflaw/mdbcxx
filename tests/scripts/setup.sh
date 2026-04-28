#!/usr/bin/env bash

set -e

ERR='\033[0;31m'
OK='\033[0;32m'
INFO='\033[0;34m'
WARN='\033[0;33m'
RESET='\033[0m'

DBPORT=8000
DBPASS='tester'
DBNAME='testmdb'

docker rm -f ${DBNAME} 2>/dev/null

echo -e "[${INFO}INFO${RESET}] Pulling database container..."

if ! docker pull mariadb:latest; then
  echo -e "[${ERR}ERROR${RESET}] Failed to pull database container!"
  echo -e "[${INFO}INFO${RESET}] Cleaning up..."
  docker rmi mariadb:latest
  exit
fi

if ! docker run \
  -d \
  --name ${DBNAME} \
  -e MARIADB_ROOT_PASSWORD=${DBPASS} \
  -e MARIADB_DATABASE=${DBNAME} \
  -e MARIADB_ROOT_HOST='%' \
  -p ${DBPORT}:3306 \
  mariadb:latest\
  ; then
  echo -e "[${ERR}ERROR${RESET}] Failed to start database container! Aborting..."
  echo -e "[${INFO}INFO${RESET}] Cleaning up..."
  docker rm -f ${DBNAME}
  exit
fi

echo -ne "[${INFO}INFO${RESET}] Waiting for database to initialize"
until docker exec ${DBNAME} mariadb-admin ping -h localhost -p${DBPASS} --silent; do
  echo -ne "."
  sleep 1
done
echo -e "[${OK}OK${RESET}]\n"

# Wait for the TCP port to actually be ready
echo -ne "[${INFO}INFO${RESET}] Waiting for TCP connection"
until docker exec ${DBNAME} mariadb -uroot -p${DBPASS} -e "SELECT 1" &>/dev/null; do
  echo -ne "."
  sleep 1
done
echo -e "[${OK}OK${RESET}]\n"

export TDB_PASS=${DBPASS}
export TDB_USR='root'
export TDB_PORT=${DBPORT}
export TDB_NAME=${DBNAME}
export TDB_HOST='127.0.0.1'

echo -e "[${INFO}INFO${RESET}] Setup complete!"
