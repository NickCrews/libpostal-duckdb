#define DUCKDB_EXTENSION_MAIN

#include "libpostal_extension.hpp"
#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/main/extension_util.hpp"
#include <duckdb/parser/parsed_data/create_scalar_function_info.hpp>

namespace duckdb {

inline void LibpostalScalarFun(DataChunk &args, ExpressionState &state, Vector &result) {
    auto &name_vector = args.data[0];
    UnaryExecutor::Execute<string_t, string_t>(
	    name_vector, result, args.size(),
	    [&](string_t name) {
			return StringVector::AddString(result, "Libpostal "+name.GetString()+" 🐥");;
        });
}

static void LoadInternal(DatabaseInstance &instance) {
    auto libpostal_scalar_function = ScalarFunction("libpostal", {LogicalType::VARCHAR}, LogicalType::VARCHAR, LibpostalScalarFun);
    ExtensionUtil::RegisterFunction(instance, libpostal_scalar_function);
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
