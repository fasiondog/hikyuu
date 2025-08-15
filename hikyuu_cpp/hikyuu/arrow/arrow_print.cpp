/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-16
 *      Author: fasiondog
 */

#include <iostream>
#include <vector>
#include <iomanip>
#include "hikyuu/utilities/datetime/Datetime.h"
#include "arrow_print.h"

namespace hku {

// 打印 Arrow Table 的辅助函数
void printArrowTable(const std::shared_ptr<arrow::Table>& table, int max_rows) {
    if (!table) {
        std::cout << "Table is null" << std::endl;
        return;
    }

    int num_columns = table->num_columns();
    int num_rows = std::min(static_cast<int>(table->num_rows()), max_rows);

    // 获取每列的名称和最大宽度
    std::vector<std::string> column_names;
    std::vector<int> column_widths;

    // 从 schema 中获取列名
    auto schema = table->schema();
    for (int i = 0; i < num_columns; ++i) {
        std::string name = schema->field(i)->name();  // 修正：从 schema 获取列名
        column_names.push_back(name);
        column_widths.push_back(name.length() + 2);  // 至少为列名长度加2
    }

    // 计算每列需要的宽度
    for (int col_idx = 0; col_idx < num_columns; ++col_idx) {
        auto col = table->column(col_idx);
        // 处理所有 chunk，而不仅仅是第一个
        for (const auto& chunk : col->chunks()) {
            int64_t rows_in_chunk = chunk->length();
            for (int64_t row_idx = 0; row_idx < rows_in_chunk && row_idx < num_rows; ++row_idx) {
                if (chunk->IsValid(row_idx)) {
                    std::string value;
                    if (chunk->type()->id() == arrow::Type::INT64) {
                        auto int_array = std::static_pointer_cast<arrow::Int64Array>(chunk);
                        value = std::to_string(int_array->Value(row_idx));
                    } else if (chunk->type()->id() == arrow::Type::DOUBLE) {
                        auto double_array = std::static_pointer_cast<arrow::DoubleArray>(chunk);
                        value = std::to_string(double_array->Value(row_idx));
                    } else if (chunk->type()->id() == arrow::Type::STRING) {
                        auto string_array = std::static_pointer_cast<arrow::StringArray>(chunk);
                        value = string_array->GetString(row_idx);
                    } else if (chunk->type()->id() == arrow::Type::BOOL) {
                        auto bool_array = std::static_pointer_cast<arrow::BooleanArray>(chunk);
                        value = bool_array->Value(row_idx) ? "true" : "false";
                    } else {
                        value = "[Unsupported type]";
                    }

                    if (static_cast<int>(value.length()) + 2 > column_widths[col_idx]) {
                        column_widths[col_idx] = value.length() + 2;
                    }
                }
            }
        }
    }

    // 打印分隔线
    auto print_separator = [&]() {
        for (int i = 0; i < num_columns; ++i) {
            std::cout << "+" << std::string(column_widths[i], '-');
        }
        std::cout << "+" << std::endl;
    };

    // 打印列名
    print_separator();
    for (int i = 0; i < num_columns; ++i) {
        std::cout << "|" << std::setw(column_widths[i]) << std::left
                  << (" " + column_names[i] + " ");
    }
    std::cout << "|" << std::endl;
    print_separator();

    // 打印数据行
    for (int row_idx = 0; row_idx < num_rows; ++row_idx) {
        for (int col_idx = 0; col_idx < num_columns; ++col_idx) {
            auto col = table->column(col_idx);
            std::string value = "null";

            // 找到包含当前行的 chunk
            int64_t current_offset = 0;
            for (const auto& chunk : col->chunks()) {
                if (row_idx < current_offset + chunk->length()) {
                    int64_t chunk_row_idx = row_idx - current_offset;
                    if (chunk->IsValid(chunk_row_idx)) {
                        if (chunk->type()->id() == arrow::Type::INT64) {
                            auto int_array = std::static_pointer_cast<arrow::Int64Array>(chunk);
                            value = std::to_string(int_array->Value(chunk_row_idx));
                        } else if (chunk->type()->id() == arrow::Type::DOUBLE) {
                            auto double_array = std::static_pointer_cast<arrow::DoubleArray>(chunk);
                            value = std::to_string(double_array->Value(chunk_row_idx));
                        } else if (chunk->type()->id() == arrow::Type::STRING) {
                            auto string_array = std::static_pointer_cast<arrow::StringArray>(chunk);
                            value = string_array->GetString(chunk_row_idx);
                        } else if (chunk->type()->id() == arrow::Type::BOOL) {
                            auto bool_array = std::static_pointer_cast<arrow::BooleanArray>(chunk);
                            value = bool_array->Value(chunk_row_idx) ? "true" : "false";
                        } else if (chunk->type()->id() == arrow::Type::DATE64) {
                            auto array = std::static_pointer_cast<arrow::Date64Array>(chunk);
                            value = std::to_string(
                              Datetime::fromTimestamp(array->Value(chunk_row_idx) * 1000LL)
                                .ymdhms());
                        } else {
                            value = "[Unsupported]";
                        }
                    }
                    break;
                }
                current_offset += chunk->length();
            }

            std::cout << "|" << std::setw(column_widths[col_idx]) << std::left
                      << (" " + value + " ");
        }
        std::cout << "|" << std::endl;
    }
    print_separator();

    // 打印表信息
    std::cout << "Showing " << num_rows << " of " << table->num_rows() << " rows. " << num_columns
              << " columns total." << std::endl;
}

}  // namespace hku