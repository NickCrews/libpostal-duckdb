#define DUCKDB_EXTENSION_MAIN

#include "libpostal_extension.hpp"
#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/main/extension_util.hpp"
#include <duckdb/parser/parsed_data/create_scalar_function_info.hpp>

#include <libpostal/libpostal.h>

namespace duckdb {

inline void LibpostalScalarFun(DataChunk &args, ExpressionState &state, Vector &result) {
    auto &name_vector = args.data[0];
    UnaryExecutor::Execute<string_t, string_t>(
	    name_vector, result, args.size(),
	    [&](string_t name) {
			return StringVector::AddString(result, "Libpostal "+name.GetString()+" 🐥");;
        });
}

inline void ParseAddress(DataChunk &args, ExpressionState &state, Vector &result) {
    libpostal_address_parser_options_t options = libpostal_get_address_parser_default_options();
    auto &text_vector = args.data[0];
    UnaryExecutor::Execute<string_t, string_t>(
	    text_vector, result, args.size(),
	    [&](string_t name) {
            libpostal_address_parser_response_t *parsed = libpostal_parse_address((char *)"781 Franklin Ave Crown Heights Brooklyn NYC NY 11216 USA", options);
			auto ret = StringVector::AddString(result, parsed->components[0]);;
            libpostal_address_parser_response_destroy(parsed);
            return ret;
        });




}

static void LoadInternal(DatabaseInstance &instance) {
    if (!libpostal_setup() || !libpostal_setup_parser()) {
        exit(EXIT_FAILURE);
    }

    auto libpostal_scalar_function = ScalarFunction("libpostal", {LogicalType::VARCHAR}, LogicalType::VARCHAR, LibpostalScalarFun);
    ExtensionUtil::RegisterFunction(instance, libpostal_scalar_function);

    auto libpostal_parse_address = ScalarFunction("libpostal_parse_address", {LogicalType::VARCHAR}, LogicalType::VARCHAR, ParseAddress);
    ExtensionUtil::RegisterFunction(instance, libpostal_parse_address);

    // Teardown (only called once at the end of your program)
    // libpostal_teardown();
    // libpostal_teardown_parser();
}

void LibpostalExtension::Load(DuckDB &db) {
	LoadInternal(*db.instance);
}
std::string LibpostalExtension::Name() {
	return "libpostal";
}

} // namespace duckdb

extern "C" {

DUCKDB_EXTENSION_API void libpostal_init(duckdb::DatabaseInstance &db) {
    duckdb::DuckDB db_wrapper(db);
    db_wrapper.LoadExtension<duckdb::LibpostalExtension>();
}

DUCKDB_EXTENSION_API const char *libpostal_version() {
	return duckdb::DuckDB::LibraryVersion();
}
}

#ifndef DUCKDB_EXTENSION_MAIN
#error DUCKDB_EXTENSION_MAIN not defined
#endif
