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
    // const auto RETURN_TYPE = LogicalType::LIST(LogicalType::STRUCT({{
    //     {"token", LogicalType::VARCHAR},
    //     {"label", LogicalType::VARCHAR},
    // }}));

    inline void ParseAddress(DataChunk &args, ExpressionState &state, Vector &result)
    {
        auto options = libpostal_get_address_parser_default_options();
        auto &text_vector = args.data[0];
        UnaryExecutor::Execute<string_t, string_t>(
            text_vector, result, args.size(),
            [&](string_t oneline)
            {
                auto *parsed = libpostal_parse_address((char *)oneline.GetString().c_str(), options);
                // Something like:
                // ret = list<struct> of length parsed->num_components
                // for (size_t i = 0; i < parsed->num_components; i++)
                // {
                //     ret[i].token = parsed->components[i];
                //     ret[i].label = parsed->labels[i];
                // }
                // return ret;
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

        auto parse_address = ScalarFunction("parse_address", {LogicalType::VARCHAR}, LogicalType::VARCHAR, ParseAddress);
        // auto parse_address = ScalarFunction("parse_address", {LogicalType::VARCHAR}, RETURN_TYPE, ParseAddress);
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
