/*-------------------------------------------------------------------------
 *
 * mysql_fdw--1.3.sql
 * 			Foreign-data wrapper for remote MySQL servers
 *
 * Portions Copyright (c) 2022-2024, EnterpriseDB Corporation.
 *
 * IDENTIFICATION
 * 			mysql_fdw--1.3.sql
 *
 *-------------------------------------------------------------------------
 */


CREATE FUNCTION mysql_fdw_handler()
RETURNS fdw_handler
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;

CREATE FUNCTION mysql_fdw_validator(text[], oid)
RETURNS void
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;

CREATE FOREIGN DATA WRAPPER mysql_fdw
  HANDLER mysql_fdw_handler
  VALIDATOR mysql_fdw_validator;

CREATE OR REPLACE FUNCTION mysql_fdw_version()
  RETURNS pg_catalog.int4 STRICT
  AS 'MODULE_PATHNAME' LANGUAGE C;

CREATE OR REPLACE FUNCTION mysql_fdw_display_pushdown_list(IN reload boolean DEFAULT false,
  OUT object_type text,
  OUT object_name text)
RETURNS SETOF record
  AS 'MODULE_PATHNAME', 'mysql_display_pushdown_list'
LANGUAGE C PARALLEL SAFE;


CREATE OR REPLACE FUNCTION write_to_parquet(query TEXT, output_path TEXT)
RETURNS void
AS 'MODULE_PATHNAME', 'write_to_parquet'
LANGUAGE C STRICT;
