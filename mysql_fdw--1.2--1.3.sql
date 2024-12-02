CREATE OR REPLACE FUNCTION write_to_parquet(query TEXT, output_path TEXT)
RETURNS void
AS 'MODULE_PATHNAME', 'write_to_parquet'
LANGUAGE C STRICT;
