/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-11
 *      Author: fasiondog
 */

#include <utf8proc.h>
#include <arrow/ipc/writer.h>
#include <arrow/io/memory.h>
#include "pybind_utils.h"

namespace hku {
size_t utf8_to_utf32(const std::string& utf8_str, int32_t* out, size_t out_len) noexcept {
    memset(out, 0, out_len * sizeof(int32_t));
    if (utf8_str.empty()) {
        return 0;
    }
    size_t result = utf8proc_decompose((const uint8_t*)utf8_str.data(), utf8_str.size(), out,
                                       out_len, UTF8PROC_NULLTERM);
    if (result < 0) {
        memset(out, 0, out_len * sizeof(int32_t));
        result = 0;
    }
    return result;
}

py::object to_pyarrow_table(const std::shared_ptr<arrow::Table>& table) {
    HKU_CHECK(table, "Table is null!");
    auto out = arrow::io::BufferOutputStream::Create().ValueOrDie();

    auto writer = arrow::ipc::MakeStreamWriter(out, table->schema()).ValueOrDie();
    HKU_CHECK(writer->WriteTable(*table).ok(), "Failed to write table to IPC stream!");
    HKU_CHECK(writer->Close().ok(), "Failed to close IPC writer!");

    // 获取序列化后的二进制数据
    auto buffer = out->Finish().ValueOrDie();
    py::buffer_info info(
      const_cast<uint8_t*>(buffer->data()),      // 数据指针（const_cast 安全，因仅读取）
      buffer->size(),                            // 数据大小
      py::format_descriptor<uint8_t>::format(),  // 数据格式
      1,                                         // 维度
      {buffer->size()},                          // 形状
      {1}                                        // 步长
    );
    py::array ipc_array(info);

    py::module_ pa = py::module_::import("pyarrow");
    py::module_ pa_ipc = py::module_::import("pyarrow.ipc");
    py::object reader = pa.attr("BufferReader")(ipc_array);
    py::object stream = pa_ipc.attr("open_stream")(reader);
    return stream.attr("read_all")();
}

}  // namespace hku