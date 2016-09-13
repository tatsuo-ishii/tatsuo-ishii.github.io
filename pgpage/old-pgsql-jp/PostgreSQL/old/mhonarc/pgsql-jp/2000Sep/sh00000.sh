#!/bin/sh
dropdb dump_err
createdb dump_err
psql dump_err <<_SQL
create table dump_err(
	id int,
	val varchar(5)
);

copy dump_err from stdin;
1	ºÙÀî¥«¥Ê
2	ºÙÀî
\.
_SQL

pg_dump -c dump_err > /tmp/dump_err.dmp
psql dump_err <<_SQL
truncate table dump_err;
_SQL
psql dump_err < /tmp/dump_err.dmp
