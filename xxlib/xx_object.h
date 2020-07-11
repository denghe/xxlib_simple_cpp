#pragma once

#include "xx_data_rw.h"
#include "xx_string.h"

namespace xx {
    // 在 DataFuncs 对原生数据类型的支持的基础上，继续扩展对 std::shared_ptr  std::weak_ptr 的 Object 基类 序列化支持
    // 读写器 继承并扩展 针对智能指针的读写函数
    // 示例在本代码文件最下方, 可供手写代码复制小改

    /************************************************************************************/
    // Data 序列化 / 反序列化 基础适配模板 for Object, std::shared_ptr, std::weak_ptr

    struct DataReaderEx;
    struct DataWriterEx;

    template<typename T, typename ENABLED = void>
    struct DataFuncsEx {
        static inline void Write(DataWriterEx& dw, T const& in) {
            DataFuncs<T>::Write(dw, in);
        }
        static inline int Read(DataReaderEx& dr, T& out) {
            return DataFuncs<T>::Read(dr, out);
        }
    };

    /************************************************************************************/
    // Object 主要用于满足 无脑智能指针堆业务逻辑 的建模与序列化需求

    struct Object {
        bool toStringFlag = false;

        Object() = default;

        virtual ~Object() = default;

        // 反序列化时的创建函数 类型
        typedef std::shared_ptr<Object>(*CreateFunc)();

        // 创建函数数组 类型
        typedef std::array<CreateFunc, 1u << (sizeof(uint16_t) * 8)> CreateFuncs;

        // 序列化时获取 typeId( 如果 Object 以值类型方式使用则无需获取 typeId )
        inline virtual uint16_t GetTypeId() const { return 0; };

        // 序列化正文
        inline virtual void Serialize(DataWriterEx &) const { };

        // 反序列化( 填充正文 )
        inline virtual int Deserialize(DataReaderEx &) { return 0; };

        // 输出 json 长相时用于输出外包围 {  } 部分
        inline virtual void ToString(std::string &s) const { };

        // 输出 json 长相时用于输出花括号内部的成员拼接
        inline virtual void ToStringCore(std::string &s) const { };
    };

    // 反序列化需要这个用以提供相应的 typeId 的创建函数
    struct ObjectCreators {
        ObjectCreators() = default;
        ObjectCreators(ObjectCreators const&) = default;
        ObjectCreators& operator=(ObjectCreators const&) = default;

        // 创建函数容器
        typename Object::CreateFuncs createFuncs {};

        // 创建相应 typeId 的创建函数( 通常调用该函数的函数体位于序列化生成物 )
        template<typename T, typename ENABLED = std::enable_if_t<std::is_base_of_v<Object, T>>>
        void Register(uint16_t const &typeId = TypeId_v<T>) {
            createFuncs[typeId] = []() -> std::shared_ptr<Object> {
                try {
                    return std::make_shared<T>();
                }
                catch (...) {
                    return std::shared_ptr<T>();
                }
            };
        }
    };

    struct DataWriterEx : DataWriter {
        using DataWriter::DataWriter;

        // WriteOnce 时备份 len 值，用于计算相对 offset
        size_t lenBak = 0;

        // key: pointer   value: offset
        std::unordered_map<void *, size_t> ptrs;

        // 支持同时写入多个值. 覆盖基类的实现，以确保走 DataFuncsEx 适配模板
        template<typename ...TS>
        void Write(TS const& ...vs) {
            std::initializer_list<int> n{ (DataFuncsEx<TS>::Write(*this, vs), 0)... };
            (void)n;
        }

        // 一次完整的写入。会记录从什么地方
        template<typename T>
        void WriteOnce(T const &v) {
            lenBak = data.len;
            ptrs.clear();
            Write(v);
        }

        // 写入 typeId + offset + data
        template<typename T>
        void WritePtr(std::shared_ptr<T> const &v) {
            // 写入 typeId. 空值 typeId 为 0
            auto &&typeId = v ? v->GetTypeId() : (uint16_t)0;
            Write(typeId);
            if (!typeId) return;

            // 计算 相对偏移量. 第一次出现则 计算相对偏移量. 否则用 从字典找到的
            auto &&iter = ptrs.find((void *) &*v);
            size_t offs;
            if (iter == ptrs.end()) {
                offs = data.len - lenBak;
                ptrs[(void *) &*v] = offs;
            } else {
                offs = iter->second;
            }

            // 写入 相对偏移量
            Write(offs);

            // 第一次出现则 继续写入 内容序列化数据
            if (iter == ptrs.end()) {
                v->Serialize(*this);
            }
        }
    };

    struct DataReaderEx : DataReader {
        // ReadOnce 时备份 offset 值，用于计算相对 offset
        size_t offsetBak = 0;

        // key: offset   value: pointer
        std::unordered_map<size_t, std::shared_ptr<Object>> objIdxs;

        // 创建函数集. 创建 Reader 之后需设置.
        typename Object::CreateFuncs *creators;

        DataReaderEx(char const* const& buf, size_t const& len, ObjectCreators& oc)
            : DataReader(buf, len)
            , creators(&oc.createFuncs) {
        }
        DataReaderEx(Data const& d, ObjectCreators& oc)
                : DataReader(d.buf, d.len)
                , creators(&oc.createFuncs) {
        }

        // 读出并填充到变量. 可同时填充多个. 返回非 0 则读取失败
        template<typename ...TS>
        int Read(TS&...vs) {
            return ReadCore(vs...);
        }

    protected:
        template<typename T, typename ...TS>
        int ReadCore(T& v, TS&...vs) {
            if (auto r = DataFuncsEx<T>::Read(*this, v)) return r;
            return ReadCore(vs...);
        }
        template<typename T>
        int ReadCore(T& v) {
            return DataFuncsEx<T>::Read(*this, v);
        }
    public:

        template<typename T>
        int ReadOnce(T &v) {
            offsetBak = offset;
            objIdxs.clear();
            return Read(v);
        }

        template<typename T>
        int ReadPtr(std::shared_ptr<T> &v) {
            // 防御性清理数据内容
            v.reset();

            // 该函数依赖 creators 被赋值. 通常程序会有一段注册有相关类型构造的函数数组
            if (!creators) return __LINE__;

            // 先读 typeId. 如果为 0 则 清空 入参 并退出
            uint16_t typeId;
            if (auto &&r = Read(typeId)) return r;
            if (!typeId) {
                return 0;
            }

            // 简单检查 创建函数 的 有效性( 防止忘记注册 )
            if (!(*creators)[typeId]) return __LINE__;

            // 计算 当前相对偏移量
            auto currOffs = offset - offsetBak;

            // 读出 相对偏移量
            size_t offs;
            if (auto r = Read(offs)) return r;

            // 如果等于 当前相对偏移量，则表明是第一次写入，后面会跟随 内容序列化数据
            if (offs == currOffs) {

                // 用创建函数创建出目标实例. 如果创建失败则退出( 防止注册到无效的创建函数或创建时内存不足构造失败啥的 )
                auto &&o = (*creators)[typeId]();
                if (!o) return __LINE__;

                // 将 o 写入 v. 利用动态转换检查数据类型兼容性。如果转换失败则退出( 防止数据与类型对应不上 )
                v = std::dynamic_pointer_cast<T>(o);
                if (!v) return __LINE__;

                // 建立 相对偏移量 到 智能指针 的映射, 以便下次遇到这个 相对偏移量 从而直接读出
                objIdxs[offs] = o;

                // 继续读出类实例内容。如果失败则退出
                if (auto &&r = v->Deserialize(*this)) return r;
            } else {

                // 从字典查找 读出的 相对偏移量 对应的 智能指针。找不到 或 类型对应不上则退出
                auto iter = objIdxs.find(offs);
                if (iter == objIdxs.end()) return __LINE__;
                if (iter->second->GetTypeId() != typeId) return __LINE__;

                // 填充 入参。如果类型转换失败则退出( 防止数据与类型对应不上 )
                v = std::dynamic_pointer_cast<T>(iter->second);
                if (!v) return __LINE__;
            }
            // 返回成功
            return 0;
        }
    };

    // 适配 std::shared_ptr<T>
    template<typename T>
    struct DataFuncsEx<std::shared_ptr<T>, std::enable_if_t<std::is_base_of_v<Object, T>>> {
        static inline void Write(DataWriterEx &dw, std::shared_ptr<T> const &in) {
            dw.WritePtr(in);
        }

        static inline int Read(DataReaderEx &dr, std::shared_ptr<T> &out) {
            return dr.ReadPtr(out);
        }
    };
    template<typename T>
    struct StringFuncs<std::shared_ptr<T>, std::enable_if_t<std::is_base_of_v<Object, T>>> {
        static inline void Append(std::string& s, std::shared_ptr<T> const& in) noexcept {
            if (in) {
                in->ToString(s);
            }
            else {
                s.append("null");
            }
        }
    };

    // 适配 std::weak_ptr<T>
    template<typename T>
    struct DataFuncsEx<std::weak_ptr<T>, std::enable_if_t<std::is_base_of_v<Object, T>>> {
        static inline void Write(DataWriterEx &dw, std::weak_ptr<T> const &in) {
            dw.WritePtr(in.lock());
        }

        static inline int Read(DataReaderEx &dr, std::weak_ptr<T> &out) {
            std::shared_ptr<T> ptr;
            if (int r = dr.ReadPtr(ptr)) return r;
            out = ptr;
            return 0;
        }
    };
    template<typename T>
    struct StringFuncs<std::weak_ptr<T>, std::enable_if_t<std::is_base_of_v<Object, T>>> {
        static inline void Append(std::string& s, std::weak_ptr<T> const& in) noexcept {
            if (auto&& p = in.lock()) {
                p->ToString(s);
            }
            else {
                s.append("null");
            }
        }
    };

    // 适配 Object
    template<typename T>
    struct DataFuncsEx<T, std::enable_if_t<std::is_base_of_v<Object, T>>> {
        static inline void Write(DataWriterEx &dw, T const &in) {
            in.Serialize(dw);
        }

        static inline int Read(DataReaderEx &dr, T &out) {
            return out.Deserialize(dr);
        }
    };

    template<typename T>
    struct StringFuncs<T, std::enable_if_t<std::is_base_of_v<Object, T>>> {
        static inline void Append(std::string &s, T const &in) {
            in.ToString(s);
        }
    };



    // 适配 std::vector
    template<typename T>
    struct DataFuncsEx<std::vector<T>, void> {
        static inline void Write(DataWriterEx &dw, std::vector<T> const &in) {
            auto buf = in.data();
            auto len = in.size();
            auto&& d = dw.data;
            d.Reserve(d.len + 5 + len * sizeof(T));
            d.WriteVarIntger(len);
            if (!len) return;
            if constexpr (sizeof(T) == 1 || std::is_same_v<float, T>) {
                ::memcpy(d.buf + d.len, buf, len * sizeof(T));
                d.len += len * sizeof(T);
            }
            else {
                for (size_t i = 0; i < len; ++i) {
                    dw.Write(buf[i]);
                }
            }
        }

        static inline int Read(DataReaderEx &dr, std::vector<T> &out) {
            size_t siz = 0;
            if (auto rtv = dr.Read(siz)) return rtv;
            if (dr.offset + siz > dr.len) return __LINE__;
            out.resize(siz);
            if (siz == 0) return 0;
            auto buf = out.data();
            if constexpr (sizeof(T) == 1 || std::is_same_v<float, T>) {
                ::memcpy(buf, dr.buf + dr.offset, siz * sizeof(T));
                dr.offset += siz * sizeof(T);
            }
            else {
                for (size_t i = 0; i < siz; ++i) {
                    if (int r = dr.Read(buf[i])) return r;
                }
            }
            return 0;
        }
    };
}


#define XX_OBJECT_OVERRIDES_H \
uint16_t GetTypeId() const override; \
void Serialize(xx::DataWriterEx &dw) const override; \
int Deserialize(xx::DataReaderEx &dr) override; \
void ToString(std::string &s) const override; \
void ToStringCore(std::string &s) const override;

#define XX_OBJECT_COPYASSIGN_H(T) \
T() = default; \
T(T const&) = default; \
T& operator=(T const&) = default; \
T(T&& o); \
T& operator=(T&& o);

#define XX_GENCODE_OBJECT_H(T, BT) \
using BaseType = BT; \
T() = default; \
T(T const&) = default; \
T& operator=(T const&) = default; \
T(T&& o) noexcept; \
T& operator=(T&& o) noexcept; \
uint16_t GetTypeId() const override; \
void Serialize(xx::DataWriterEx &dw) const override; \
int Deserialize(xx::DataReaderEx &dr) override; \
void ToString(std::string &s) const override; \
void ToStringCore(std::string &s) const override;

#define XX_GENCODE_STRUCT_H(T) \
T() = default; \
T(T const&) = default; \
T& operator=(T const&) = default; \
T(T&& o) noexcept; \
T& operator=(T&& o) noexcept;

/*

struct Foo : xx::Object {
    using BaseType = xx::Object;

    XX_OBJECT_OVERRIDES_H

    int i = 1;
    std::string name = "asdf";
};

struct Bar : Foo {
    using BaseType = Foo;

    XX_OBJECT_OVERRIDES_H

    float x = 1.2, y = 3.4;
};

namespace xx {
    template<>
    struct TypeId<Foo> {
        static const uint16_t value = 1;
    };
    template<>
    struct TypeId<Bar> {
        static const uint16_t value = 2;
    };
}

uint16_t Foo::GetTypeId() const { return xx::TypeId_v<Foo>; }

void Foo::Serialize(xx::DataWriterEx &dw) const {
    this->BaseType::Serialize(dw);
    dw.Write(i, name);
}

int Foo::Deserialize(xx::DataReaderEx &dr) {
    if (int r = this->BaseType::Deserialize(dr)) return r;
    return dr.Read(i, name);
}

void Foo::ToString(std::string &s) const {
    xx::Append(s, "{\"typeId\":", GetTypeId());
    this->BaseType::ToStringCore(s);
    ToStringCore(s);
    s.push_back('}');
}

bool Foo::ToStringCore(std::string &s) const {
    ::xx::Append(s, ",\"i\":", i);
    ::xx::Append(s, ",\"name\":", name);
    return true;
}

uint16_t Bar::GetTypeId() const { return xx::TypeId_v<Bar>; }

void Bar::Serialize(xx::DataWriterEx &dw) const {
    this->BaseType::Serialize(dw);
    dw.Write(x, y);
}

int Bar::Deserialize(xx::DataReaderEx &dr) {
    if (int r = this->BaseType::Deserialize(dr)) return r;
    return dr.Read(x, y);
}

void Bar::ToString(std::string &s) const {
    xx::Append(s, "{\"typeId\":", GetTypeId());
    this->BaseType::ToStringCore(s);
    ToStringCore(s);
    s.push_back('}');
}

bool Bar::ToStringCore(std::string &s) const {
    ::xx::Append(s, ",\"x\":", x);
    ::xx::Append(s, ",\"y\":", y);
    return true;
}

*/
