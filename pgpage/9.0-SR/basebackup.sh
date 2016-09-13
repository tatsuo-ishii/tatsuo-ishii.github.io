#! /bin/sh
psql -c "SELECT pg_start_backup('Streaming Replication', true)" postgres
rsync -a ${PGDATA}/ localhost:/usr/local/src/pgsql/9.0-beta/standby/ --exclude postmaster.pid --exclude postgresql.conf --exclude recovery.conf --exclude pg_log
rm -f standby/pg_xlog/*
rm -f standby/pg_xlog/archive_status/*

psql -c "SELECT pg_stop_backup()" postgres
