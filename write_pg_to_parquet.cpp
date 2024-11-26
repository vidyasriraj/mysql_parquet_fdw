#include <arrow/api.h>
#include <postgres.h>
#include <fmgr.h>
#include <catalog/pg_type.h>
#include <catalog/pg_attribute.h>
#include <utils/array.h>
#include <utils/builtins.h>
#include <memory>
#include <string>

extern "C" Datum writeToParquet(PG_FUNCTION_ARGS);

// Helper function to map PostgreSQL types to Arrow types
std::shared_ptr<arrow::DataType> PostgresTypeToArrow(Oid type_id) {
    switch (type_id) {
        case INT4OID:
            return arrow::int32();
        case TEXTOID:
            return arrow::utf8();
        // Add other cases as needed
        default:
            return nullptr;
    }
}

// Function to convert PostgreSQL data to Arrow Array
std::shared_ptr<arrow::Array> ConvertPGToArrow(TupleDesc tupleDesc, int attnum, int* values, bool* isnull, int64_t rowCount) {
    arrow::Int32Builder builder;

    for (int i = 0; i < rowCount; ++i) {
        if (isnull[i]) {
            builder.AppendNull();  // Handle nulls
        } else {
            builder.Append(values[i]);  // Handle non-null values
        }
    }

    std::shared_ptr<arrow::Array> array;
    if (!builder.Finish(&array).ok()) {
        elog(ERROR, "Error building Arrow array");
    }
    return array;
}

extern "C" Datum writeToParquet(PG_FUNCTION_ARGS) {
    // Define the tuple descriptor and other necessary components
    TupleDesc tupleDesc = (TupleDesc) PG_GETARG_POINTER(0);
    int attnum = PG_GETARG_INT32(1);
    Datum* values = (Datum*) PG_GETARG_POINTER(2);
    bool* isnull = (bool*) PG_GETARG_POINTER(3);
    int64_t rowCount = PG_GETARG_INT64(4);

    // Get the Arrow type corresponding to PostgreSQL type
    Oid type_id = tupleDesc->attrs[attnum].atttypid;  // Use dot notation
    std::shared_ptr<arrow::DataType> arrow_type = PostgresTypeToArrow(type_id);

    // Convert PostgreSQL data to Arrow Array
    auto arrow_array = ConvertPGToArrow(tupleDesc, attnum, (int*) values, isnull, rowCount);

    // Return result as a Datum (depending on your actual implementation)
    PG_RETURN_DATUM(0);  // Adjust this to return the correct value
}
