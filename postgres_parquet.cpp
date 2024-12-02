#include "parquet_writer.h"
#include <vector>
#include <string>

extern "C" {
// PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(write_to_parquet);

Datum write_to_parquet(PG_FUNCTION_ARGS) {
    char* query = text_to_cstring(PG_GETARG_TEXT_P(0));  // Query as TEXT
    char* output_path = text_to_cstring(PG_GETARG_TEXT_P(1));  // Output file path as TEXT

    if (SPI_connect() != SPI_OK_CONNECT) {
        elog(ERROR, "Failed to connect to SPI");
    }

    // Execute the query
    int ret = SPI_execute(query, true, 0);
    if (ret != SPI_OK_SELECT) {
        SPI_finish();
        elog(ERROR, "Failed to execute query: %s", query);
    }

    // Fetch results
    int num_rows = SPI_processed;
    int num_columns = SPI_tuptable->tupdesc->natts;

    // Prepare column names and data
    std::vector<std::string> column_names;
    std::vector<std::vector<std::string>> data;

    // Collect column names
    for (int col = 0; col < num_columns; col++) {
        column_names.push_back(SPI_fname(SPI_tuptable->tupdesc, col + 1));
    }

    // Collect rows
    for (int row = 0; row < num_rows; row++) {
        std::vector<std::string> row_data;
        for (int col = 0; col < num_columns; col++) {
            bool is_null = false;
            Datum value = SPI_getbinval(SPI_tuptable->vals[row], SPI_tuptable->tupdesc, col + 1, &is_null);
            if (is_null) {
                row_data.push_back("NULL");
            } else {
                char* val_cstr = SPI_getvalue(SPI_tuptable->vals[row], SPI_tuptable->tupdesc, col + 1);
                row_data.push_back(std::string(val_cstr));
            }
        }
        data.push_back(row_data);
    }

    SPI_finish();

    // Write data to Parquet
    auto status = WriteToParquet(column_names, data, output_path);
    if (!status.ok()) {
        elog(ERROR, "Failed to write Parquet file: %s", status.message().c_str());
    }

    PG_RETURN_VOID();
}
}
