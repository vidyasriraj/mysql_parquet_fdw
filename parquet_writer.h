#ifndef PARQUET_WRITER_H
#define PARQUET_WRITER_H

#include <arrow/api.h>

#include <memory>
#include <string>

// PostgreSQL includes
#ifdef __cplusplus
extern "C" {
#endif
#include <postgres.h>
#include <fmgr.h>
#include <executor/spi.h>
#include <catalog/pg_type.h>
#include <catalog/pg_attribute.h>
#include <utils/array.h>
#include <utils/builtins.h>
#ifdef __cplusplus
}
#endif

// Convert PostgreSQL type to Arrow type
std::shared_ptr<arrow::DataType> PostgresTypeToArrow(Oid type_id);

// Write an Arrow Table to a Parquet file
arrow::Status WriteToParquet(const std::vector<std::string>& column_names,
                             const std::vector<std::vector<std::string>>& data,
                             const std::string& output_path);

#endif // PARQUET_WRITER_H
