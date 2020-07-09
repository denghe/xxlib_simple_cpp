#pragma once

#include "xx_data_rw.h"
#include "xx_string.h"

namespace xx {
    // 在 DataReader 对原生数据类型的支持的基础上，继续扩展对 std::shared_ptr  std::weak_ptr 的类实例序列化支持

    struct ObjectWriter;
    struct ObjectReader;

    struct Object {
        Object() = default;

        virtual ~Object() = default;

        // 序列化相关
        // 获取 type id
        virtual uint16_t GetTypeId() const = 0;

        // 序列化
        virtual void Serialize(ObjectWriter &) const = 0;

        // 反序列化
        virtual int Deserialize(ObjectReader &) = 0;

        // 字串输出相关
        // 输出外壳
        virtual void ToString(std::string &s) const = 0;

        // 输出成员
        virtual void ToStringCore(std::string &s) const = 0;
    };

    // Object 序列化 基础适配模板. 默认转发到 DataFuncs 模板. 针对 std::shared/weak_ptr 特化
    template<typename T, typename ENABLED = void>
    struct ObjectFuncs {
        static void Write(ObjectWriter &writer, T const &in);
        static int Read(ObjectReader &reader, T &out);
    };

    struct ObjectWriter : DataWriter {
        using DataWriter::DataWriter;
        // offset值写入修正
        size_t offsetRoot = 0;
        // key: pointer   value: offset
        std::unordered_map<void*, size_t> ptrs;

        template<typename T>
        void WritePtr(std::shared_ptr<T> const&);
    };

    struct ObjectReader : DataReader {
        using DataReader::DataReader;
        // key: offset   value: pointer
        std::unordered_map<size_t, std::shared_ptr<Object>> objIdxs;

        template<typename T>
        void ReadPtr(std::shared_ptr<T>&);
    };

    // 适配基本数据类型，转发到 DataFuncs 模板
    template<typename T, typename ENABLED>
    inline void ObjectFuncs<T, ENABLED>::Write(ObjectWriter &writer, T const &in) {
        DataFuncs<T>::Write(writer, in);
    }

    template<typename T, typename ENABLED>
    inline int ObjectFuncs<T, ENABLED>::Read(ObjectReader &reader, T &out) {
        return DataFuncs<T>::Read(reader, out);
    }

    // 适配 std::shared_ptr<T>
    template<typename T>
    struct ObjectFuncs<std::shared_ptr<T>, std::enable_if_t<
            std::is_base_of_v<Object, T> || std::is_same_v<std::string, T>>> {
        static inline void Write(ObjectWriter &writer, std::shared_ptr<T> const &in) noexcept {
            writer.WritePtr(in);
        }

        static inline int Read(ObjectReader &reader, std::shared_ptr<T> &out) noexcept {
            return reader.ReadPtr(out);
        }
    };

    // 适配 std::weak_ptr<T>
    template<typename T>
    struct ObjectFuncs<std::weak_ptr<T>, std::enable_if_t<
            std::is_base_of_v<Object, T> || std::is_same_v<std::string, T>>> {
        static inline void Write(ObjectWriter &writer, std::weak_ptr<T> const &in) noexcept {
            if (auto ptr = in.lock()) {
                writer.WritePtr(ptr);
            } else {
                writer.WriteVarIntger((uint16_t) 0);
            }
        }

        static inline int Read(ObjectReader &reader, std::weak_ptr<T> &out) noexcept {
            std::shared_ptr<T> ptr;
            if (int r = reader.ReadPtr(ptr)) return r;
            out = ptr;
            return 0;
        }
    };
}
