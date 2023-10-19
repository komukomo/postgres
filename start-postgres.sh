#!/bin/sh

BIN_DIR=./build/postgres/bin
POSTGRES_PORT=5432

"${BIN_DIR}"/postgres -D "${BIN_DIR}"/pgdata -p ${POSTGRES_PORT}