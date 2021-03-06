#include "duckdb/function/scalar/string_functions.hpp"

#include "duckdb/common/exception.hpp"
#include "duckdb/common/vector_operations/vector_operations.hpp"
#include "utf8proc.hpp"

using namespace std;

namespace duckdb {

// length returns the size in characters
struct StringLengthOperator {
	template <class TA, class TR> static inline TR Operation(TA input) {
		return LengthFun::Length<TA, TR>(input);
	}
};

// strlen returns the size in bytes
struct StrLenOperator {
	template <class TA, class TR> static inline TR Operation(TA input) {
		return input.GetSize();
	}
};

// bitlen returns the size in bits
struct BitLenOperator {
	template <class TA, class TR> static inline TR Operation(TA input) {
		return 8 * input.GetSize();
	}
};

void LengthFun::RegisterFunction(BuiltinFunctions &set) {
	set.AddFunction({"length", "len"},
	                ScalarFunction({LogicalType::VARCHAR}, LogicalType::BIGINT,
	                               ScalarFunction::UnaryFunction<string_t, int64_t, StringLengthOperator, true>));
	set.AddFunction(ScalarFunction("strlen", {LogicalType::VARCHAR}, LogicalType::BIGINT,
	                               ScalarFunction::UnaryFunction<string_t, int64_t, StrLenOperator, true>));
	set.AddFunction(ScalarFunction("bit_length", {LogicalType::VARCHAR}, LogicalType::BIGINT,
	                               ScalarFunction::UnaryFunction<string_t, int64_t, BitLenOperator, true>));
	// length for BLOB type
	set.AddFunction(ScalarFunction("octet_length", {LogicalType::BLOB}, LogicalType::BIGINT,
	                               ScalarFunction::UnaryFunction<string_t, int64_t, StrLenOperator, true>));
}

struct UnicodeOperator {
	template <class TA, class TR> static inline TR Operation(const TA &input) {
		const auto str = reinterpret_cast<const utf8proc_uint8_t *>(input.GetDataUnsafe());
		const auto len = input.GetSize();
		utf8proc_int32_t codepoint;
		(void)utf8proc_iterate(str, len, &codepoint);
		return codepoint;
	}
};

void UnicodeFun::RegisterFunction(BuiltinFunctions &set) {
	set.AddFunction(ScalarFunction("unicode", {LogicalType::VARCHAR}, LogicalType::INTEGER,
	                               ScalarFunction::UnaryFunction<string_t, int32_t, UnicodeOperator, true>));
}

} // namespace duckdb
