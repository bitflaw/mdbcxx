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
MARIADB_TAG=lts-ubi10

# docker rm -f ${DBNAME} 2>/dev/null

die()
{
  echo -e "[${ERR}ERROR${RESET}] $1"
  (return 0 2>/dev/null) && return 1 || exit 1
}

pull()
{
  echo -e "[${INFO}INFO${RESET}] Pulling database container..."
  docker pull mariadb:${MARIADB_TAG} || die "Failed to pull database container!"
}

run()
{
  docker run \
    -d \
    --name ${DBNAME} \
    -e MARIADB_ROOT_PASSWORD=${DBPASS} \
    -e MARIADB_DATABASE=${DBNAME} \
    -e MARIADB_ROOT_HOST='%' \
    -p ${DBPORT}:3306 \
    mariadb:${MARIADB_TAG} || die "Failed to start database container! Aborting..."
}

init_conn ()
{
  echo -ne "[${INFO}INFO${RESET}] Initializing"
  until docker exec ${DBNAME} mariadb-admin ping -h localhost -p${DBPASS} --silent; do
    echo -ne "."
    sleep 1s
  done
  echo -e "[${OK}OK${RESET}]\n"

  # Wait for the TCP port to actually be ready
  echo -ne "[${INFO}INFO${RESET}] Waiting for TCP connection"
  until docker exec ${DBNAME} mariadb -uroot -p${DBPASS} -e "SELECT 1" &>/dev/null; do
    echo -ne "."
    sleep 1s
  done
  echo -e "[${OK}OK${RESET}]\n"
}

docker image inspect mariadb:${MARIADB_TAG} &> /dev/null || pull

if ! docker ps --format '{{.Names}}' | grep -q "^${DBNAME}$"; then
  if docker ps -a --format '{{.Names}}' | grep -q "^${DBNAME}$"; then
    docker start "${DBNAME}" &>/dev/null
  else
    run
  fi
  init_conn
fi

echo -e "[${INFO}INFO${RESET}] Exporting variables..."

export TDB_PASS=${DBPASS}
export TDB_USR='root'
export TDB_PORT=${DBPORT}
export TDB_NAME=${DBNAME}
export TDB_HOST='127.0.0.1'

echo -e "[${INFO}INFO${RESET}] Setup complete!"
