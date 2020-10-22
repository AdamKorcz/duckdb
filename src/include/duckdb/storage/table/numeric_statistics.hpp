//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/storage/table/numeric_statistics.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/storage/table/base_statistics.hpp"
#include "duckdb/common/serializer.hpp"
#include "duckdb/common/limits.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/string_util.hpp"

namespace duckdb {

template<class T>
class NumericStatistics : public BaseStatistics {
public:
	NumericStatistics() {
		min = NumericLimits<T>::Maximum();
		max = NumericLimits<T>::Minimum();
	}
	NumericStatistics(T min, T max) : min(min), max(max) {
	}

	//! The minimum value of the segment
	T min;
	//! The maximum value of the segment
	T max;

public:
	inline void Update(T value) {
		if (LessThan::Operation(value, min)) {
			min = value;
		}
		if (GreaterThan::Operation(value, max)) {
			max = value;
		}
	}
	void Merge(const BaseStatistics &other_) override {
		auto &other = (const NumericStatistics<T> &) other_;
		has_null = has_null || other.has_null;
		min = MinValue<T>(min, other.min);
		max = MaxValue<T>(max, other.max);
	}
	bool CheckZonemap(ExpressionType comparison_type, T constant) {
		switch (comparison_type) {
		case ExpressionType::COMPARE_EQUAL:
			return constant >= min && constant <= max;
		case ExpressionType::COMPARE_GREATERTHANOREQUALTO:
			return constant <= max;
		case ExpressionType::COMPARE_GREATERTHAN:
			return constant < max;
		case ExpressionType::COMPARE_LESSTHANOREQUALTO:
			return constant >= min;
		case ExpressionType::COMPARE_LESSTHAN:
			return constant > min;
		default:
			throw InternalException("Operation not implemented");
		}
	}

	unique_ptr<BaseStatistics> Copy() override {
		auto stats = make_unique<NumericStatistics<T>>(min, max);
		stats->has_null = has_null;
		return move(stats);
	}
	void Serialize(Serializer &serializer) override {
		BaseStatistics::Serialize(serializer);
		serializer.Write<T>(min);
		serializer.Write<T>(max);
	}
	static unique_ptr<BaseStatistics> Deserialize(Deserializer &source) {
		auto min = source.Read<T>();
		auto max = source.Read<T>();
		return make_unique_base<BaseStatistics, NumericStatistics<T>>(min, max);
	}

	string ToString() override {
		return StringUtil::Format("Numeric Statistics<%s> [Has Null: %s, Min: %lld, Max: %lld]",
		    TypeIdToString(GetTypeId<T>()), has_null ? "true" : "false", min, max);
	}
};

template<>
string NumericStatistics<hugeint_t>::ToString();
template<>
string NumericStatistics<float>::ToString();
template<>
string NumericStatistics<double>::ToString();

}