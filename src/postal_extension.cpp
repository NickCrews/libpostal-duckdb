#define DUCKDB_EXTENSION_MAIN

#include "postal_extension.hpp"
#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/main/extension_util.hpp"
#include <duckdb/parser/parsed_data/create_scalar_function_info.hpp>

#include <libpostal/libpostal.h>

namespace duckdb
{

    inline void PostalScalarFun(DataChunk &args, ExpressionState &state, Vector &result)
    {
        auto &name_vector = args.data[0];
        UnaryExecutor::Execute<string_t, string_t>(
            name_vector, result, args.size(),
            [&](string_t name)
            {
                return StringVector::AddString(result, "Postal " + name.GetString() + " 🐥");
            });
    }

    inline void ParseAddress(DataChunk &args, ExpressionState &state, Vector &result)
    {
        libpostal_address_parser_options_t options = libpostal_get_address_parser_default_options();
        auto &text_vector = args.data[0];
        UnaryExecutor::Execute<string_t, string_t>(
            text_vector, result, args.size(),
            [&](string_t oneline)
            {
                libpostal_address_parser_response_t *parsed = libpostal_parse_address((char *)oneline.GetString().c_str(), options);
                auto ret = StringVector::AddString(result, parsed->labels[0]);
                libpostal_address_parser_response_destroy(parsed);
                return ret;
            });
    }

    static void LoadInternal(DatabaseInstance &instance)
    {
        if (!libpostal_setup() || !libpostal_setup_parser())
        {
            exit(EXIT_FAILURE);
        }

        auto postal = ScalarFunction("postal", {LogicalType::VARCHAR}, LogicalType::VARCHAR, PostalScalarFun);
        ExtensionUtil::RegisterFunction(instance, postal);

        auto parse_address = ScalarFunction("parse_address", {LogicalType::VARCHAR}, LogicalType::VARCHAR, ParseAddress);
        ExtensionUtil::RegisterFunction(instance, parse_address);

        // Teardown (only called once at the end of your program)
        // libpostal_teardown();
        // libpostal_teardown_parser();
    }

    void PostalExtension::Load(DuckDB &db)
    {
        LoadInternal(*db.instance);
    }
    std::string PostalExtension::Name()
    {
        return "postal";
    }

} // namespace duckdb

extern "C"
{

    DUCKDB_EXTENSION_API void postal_init(duckdb::DatabaseInstance &db)
    {
        duckdb::DuckDB db_wrapper(db);
        db_wrapper.LoadExtension<duckdb::PostalExtension>();
    }

    DUCKDB_EXTENSION_API const char *postal_version()
    {
        return duckdb::DuckDB::LibraryVersion();
    }
}

#ifndef DUCKDB_EXTENSION_MAIN
#error DUCKDB_EXTENSION_MAIN not defined
#endif
