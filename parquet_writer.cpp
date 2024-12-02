#include <arrow/api.h>
#include <arrow/io/file.h>
#include <parquet/arrow/writer.h>
#include <parquet/types.h>
#include <vector>
#include <string>
extern "C" {
#include <postgres.h>  
#include <catalog/pg_type.h>  // Add this line to include PostgreSQL type OIDs
}

// Convert PostgreSQL types to Arrow types
std::shared_ptr<arrow::DataType> PostgresTypeToArrow(Oid type_id) {
    switch (type_id) {
        case INT4OID:
            return arrow::int32();
        case TEXTOID:
            return arrow::utf8();
        default:
            elog(ERROR, "Unsupported PostgreSQL type for Arrow conversion.");
            return nullptr;
    }
}

std::shared_ptr<arrow::Table> CreateArrowTable(
    const std::vector<std::string>& column_names,
    const std::vector<std::vector<std::string>>& data) {
    std::vector<std::shared_ptr<arrow::Field>> fields;
    for (const auto& column_name : column_names) {
        fields.push_back(arrow::field(column_name, arrow::utf8()));  // Assuming string data
    }
    auto schema = arrow::schema(fields);

    std::vector<std::shared_ptr<arrow::Array>> arrays;
    for (size_t col = 0; col < column_names.size(); ++col) {
        arrow::StringBuilder builder;
        for (const auto& row : data) {
            auto status = builder.Append(row[col]);
            if (!status.ok()) {
                elog(ERROR, "Error appending data to Arrow array: %s", status.message().c_str());
                return nullptr;
            }
        }
        arrays.push_back(builder.Finish().ValueOrDie());
    }

    return arrow::Table::Make(schema, arrays);
}


// Write Arrow Table to Parquet
arrow::Status WriteToParquet(const std::vector<std::string>& column_names,
                             const std::vector<std::vector<std::string>>& data,
                             const std::string& output_path) {
    // Convert data to Arrow Table
    auto table = CreateArrowTable(column_names, data);

    // Create a file output stream
    std::shared_ptr<arrow::io::FileOutputStream> outfile;
    ARROW_ASSIGN_OR_RAISE(outfile, arrow::io::FileOutputStream::Open(output_path));
    
    // Create a Parquet writer
    std::unique_ptr<parquet::arrow::FileWriter> writer;
    PARQUET_ASSIGN_OR_THROW(writer, parquet::arrow::FileWriter::Open(*table->schema(), arrow::default_memory_pool(), outfile));

    // Write the table to the Parquet file
    ARROW_RETURN_NOT_OK(writer->WriteTable(*table, table->num_rows()));
    return writer->Close();
}

arrow::Status ReadFromParquet(const std::string& input_path, std::vector<std::vector<std::string>>& data) {
    // Create a file input stream
    std::shared_ptr<arrow::io::FileInputStream> infile;
    ARROW_ASSIGN_OR_RAISE(infile, arrow::io::FileInputStream::Open(input_path));

    // Create a Parquet reader
    std::unique_ptr<parquet::arrow::FileReader> reader;
    PARQUET_ASSIGN_OR_THROW(reader, parquet::arrow::FileReader::Open(*infile));  

    // Read the table from the Parquet file and convert it to a vector of vectors
    std::shared_ptr<arrow::Table> table;        
    PARQUET_ASSIGN_OR_THROW(table, reader->ReadTable());    
    for (size_t row = 0; row < table->num_rows(); ++row) {
        std::vector<std::string> row_data;
        for (size_t col = 0; col < table->num_columns(); ++col) {
            auto value = table->column(col)->GetView(row);
            row_data.push_back(value.ToString());
        }
        data.push_back(row_data);
    }
    return arrow::Status::OK(); 
    }