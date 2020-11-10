﻿#pragma once

#include "xx_ptr.h"
#include "xx_data.h"
#include "xx_typename_islambda.h"

#define XX_GENCODE_OBJECT_H(T, BT) \
using BaseType = BT; \
T() = default; \
T(T const&) = default; \
T& operator=(T const&) = default; \
T(T&& o) noexcept; \
T& operator=(T&& o) noexcept; \
void Write(xx::ObjManager& o) const override; \
int Read(xx::ObjManager& o) override; \
void ToString(xx::ObjManager& o) const override; \
void ToStringCore(xx::ObjManager& o) const override; \
void Clone1(xx::ObjManager& o, void* const& tar) const override; \
void Clone2(xx::ObjManager& o, void* const& tar) const override; \
void RecursiveReset(xx::ObjManager& o) override;

#define XX_GENCODE_STRUCT_H(T) \
T() = default; \
T(T const&) = default; \
T& operator=(T const&) = default; \
T(T&& o) noexcept; \
T& operator=(T&& o) noexcept;

namespace xx {

	struct ObjBase;
	using ObjBase_s = Shared<ObjBase>;
	struct ObjManager;

	/************************************************************************************/
	// 接口函数适配模板. 特化 以扩展类型支持
	template<typename T, typename ENABLED = void>
	struct ObjFuncs {
		static inline void Write(ObjManager& om, T const& in) {
			std::string s(xx::TypeName_v<T>);
		}
		static inline int Read(ObjManager& om, T& out) {
			return 0;
		}
		static inline void ToString(ObjManager& om, T const& in) {
			std::string s(xx::TypeName_v<T>);
		}
		static inline void ToStringCore(ObjManager& om, T const& in) {
		}
		static inline void Clone1(ObjManager& om, T const& in, T& out) {
			out = in;
		}
		static inline void Clone2(ObjManager& om, T const& in, T& out) {
		}
		static inline void RecursiveReset(ObjManager& om, T& in) {
		}
	};


	/************************************************************************************/
	// 方便复制
	/*
	inline void Write(xx::ObjManager& o) const override { }
	inline int Read(xx::ObjManager& o) override { }
	inline void ToString(xx::ObjManager& o) const override { }
	inline void ToStringCore(xx::ObjManager& o) const override { }
	inline void Clone1(xx::ObjManager& o, void* const& tar) const override { }
	inline void Clone2(xx::ObjManager& o, void* const& tar) const override { }
	inline void RecursiveReset(xx::ObjManager& o) override { }
	*/

	// ObjBase: 仅用于 Shared<> Weak<> 包裹的类型基类
	struct ObjBase {
		// 派生类都需要有默认构造。
		ObjBase() = default;

		virtual ~ObjBase() = default;

		// 序列化
		virtual void Write(ObjManager& om) const = 0;

		// 反序列化
		virtual int Read(ObjManager& om) = 0;

		// 输出 json 长相时用于输出外包围 {  } 部分
		virtual void ToString(ObjManager& om) const = 0;

		// 输出 json 长相时用于输出花括号内部的成员拼接
		virtual void ToStringCore(ObjManager& om) const = 0;

		// 克隆步骤1: 拷贝普通数据，遇到 Shared 就同型新建, 并保存映射关系
		virtual void Clone1(ObjManager& om, void* const& tar) const = 0;

		// 克隆步骤2: 只处理成员中的 Weak 类型。根据步骤 1 建立的映射关系来填充
		virtual void Clone2(ObjManager& om, void* const& tar) const = 0;

		// 向 o 传递所有 Shared<T> member 以斩断循环引用 防止内存泄露
		virtual void RecursiveReset(ObjManager& om) = 0;

		// 注意: 下面两个函数, 不可以在析构函数中使用, 构造函数中使用也需要确保构造过程顺利无异常。另外，如果指定 T, 则 unsafe, 需小心确保 this 真的能转为 T。不确定就不传参，用 .As<T>
		// 得到当前类的强指针
		template<typename T = ObjBase>
		XX_FORCEINLINE Shared<T> SharedFromThis() const {
			auto h = (PtrHeader*)this - 1;
			return (*((Weak<T>*)&h)).Lock();
		}

		// 得到当前类的弱指针
		template<typename T = ObjBase>
		XX_FORCEINLINE Weak<T> WeakFromThis() const {
			auto h = (PtrHeader*)this - 1;
			return *((Weak<T>*)&h);
		}

		// 得到当前类的 typeId
		XX_FORCEINLINE int16_t GetTypeId() const {
			auto h = (PtrHeader*)this - 1;
			return (int16_t)h->typeId;
		}
	};


	/************************************************************************************/
	// ObjBase 相关操作类. 注册 typeId 与 关联 Create 函数

	struct ObjManager {
		// 公共上下文
		std::vector<void*> ptrs;
		std::vector<void*> ptrs2;
		Data* data = nullptr;
		std::string* str = nullptr;

		// 类实例 创建函数
		typedef ObjBase_s(*FT)();

		// typeId : 类实例 创建函数 映射容器
		std::array<FT, std::numeric_limits<uint16_t>::max()> fs{};

		// 注册类型 & ptrTypeId. 将创建函数塞入容器
		template<typename T>
		void Register(uint16_t const& typeId = TypeId_v<T>) {
			static_assert(std::is_base_of_v<ObjBase, T>);
			fs[typeId] = []() -> ObjBase_s { return MakeShared<T>(); };
		}

		// 根据 typeId 来创建对象. 失败返回空
		template<typename T = ObjBase>
		XX_FORCEINLINE Shared<T> Create(uint16_t const& typeId) {
			static_assert(std::is_base_of_v<ObjBase, T>);
			if (!fs[typeId]) return nullptr;
			return fs[typeId]();
		}

		// 向 data 写入数据. 会初始化写入上下文, 并在写入结束后擦屁股( 主要入口 )
		template<typename...Args>
		XX_FORCEINLINE void WriteTo(Data& d, Args const&...args) {
			static_assert(sizeof...(args) > 0);
			data = &d;
			ptrs.clear();
			auto sg = MakeScopeGuard([this] {
				for (auto&& p : ptrs) {
					*(uint32_t*)p = 0;
				}
				});

			(Write_(args), ...);
		}

	protected:
		// 内部函数
		template<typename T>
		XX_FORCEINLINE void Write_(T const& v) {
			auto& d = *data;
			if constexpr (IsPtrShared_v<T>) {
				using U = typename T::ElementType;
				if constexpr (std::is_same_v<U, ObjBase> || TypeId_v<U> > 0) {
					auto typeId = v.typeId();
					d.WriteVarIntger(typeId);
					if (typeId == 0) return;

					auto h = ((PtrHeader*)v.pointer - 1);
					if (h->offset == 0) {
						ptrs.push_back(&h->offset);
						h->offset = (uint32_t)ptrs.size();
						d.WriteVarIntger(h->offset);
						Write_(*v.pointer);
					}
					else {
						d.WriteVarIntger(h->offset);
					}
				}
				else {
					if (v) {
						d.WriteFixed((uint8_t)1);
						Write_(*v);
					}
					else {
						d.WriteFixed((uint8_t)0);
					}
				}
			}
			else if constexpr (IsPtrWeak_v<T>) {
				if (v.h) {
					auto p = v.h + 1;
					Write_(*(Shared<typename T::ElementType>*) & p);
				}
				else {
					d.WriteFixed((uint8_t)0);
				}
			}
			else if constexpr (std::is_base_of_v<ObjBase, T>) {
				v.Write(*this);
			}
			else if constexpr (IsOptional_v<T>) {
				if (v.has_value()) {
					d.WriteFixed((uint8_t)1);
					Write_(*v);
				}
				else {
					d.WriteFixed((uint8_t)0);
				}
			}
			else if constexpr (IsVector_v<T>) {
				d.WriteVarIntger(v.size());
				if (v.empty()) return;
				if constexpr (sizeof(T) == 1 || std::is_floating_point_v<T>) {
					d.WriteBuf(v.data(), v.size() * sizeof(T));
				}
				else if constexpr (std::is_integral_v<typename T::value_type>) {
					auto cap = v.size() * (sizeof(T) + 1);
					if (d.cap < cap) {
						d.Reserve<false>(cap);
					}
					for (auto&& o : v) {
						d.WriteVarIntger<false>(o);
					}
				}
				else {
					for (auto&& o : v) {
						Write_(o);
					}
				}
			}
			else if constexpr (IsUnorderedSet_v<T>) {
				d.WriteVarIntger(v.size());
				for (auto&& o : v) {
					Write_(o);
				}
			}
			else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>) {
				d.WriteVarIntger(v.size());
				d.WriteBuf(v.data(), v.size());
			}
			else if constexpr (std::is_same_v<T, xx::Data> || std::is_same_v<T, xx::DataView>) {
				d.WriteVarIntger(v.len);
				d.WriteBuf(v.buf, v.len);
			}
			else if constexpr (std::is_integral_v<T>) {
				if constexpr (sizeof(T) == 1) {
					d.WriteFixed(v);
				}
				else {
					d.WriteVarIntger(v);
				}
			}
			else if constexpr (std::is_enum_v<T>) {
				Write_(*(std::underlying_type_t<T>*) & v);
			}
			else if constexpr (std::is_floating_point_v<T>) {
				d.WriteFixed(v);
			}
			else if constexpr (IsTuple_v<T>) {
				std::apply([&](auto const &... args) {
					(Write_(args), ...);
					}, v);
			}
			else if constexpr (IsPair_v<T>) {
				Write(v.first, v.second);
			}
			else if constexpr (IsMap_v<T> || IsUnorderedMap_v<T>) {
				d.WriteVarIntger(v.size());
				for (auto&& kv : v) {
					Write(kv.first, kv.second);
				}
			}
			else {
				ObjFuncs<T>::Write(*this, v);
			}
		}

	public:
		// 转发到 Write_
		template<typename...Args>
		XX_FORCEINLINE void Write(Args const&...args) {
			static_assert(sizeof...(args) > 0);
			(Write_(args), ...);
		}

		// 从 data 读入 / 反序列化, 填充到 v. 原则: 尽量复用, 不新建对象( 主要入口 )
		// 可传入开始读取的位置
		template<typename...Args>
		XX_FORCEINLINE int ReadFrom(Data& d, Args&...args) {
			static_assert(sizeof...(args) > 0);
			data = &d;
			ptrs.clear();
			return Read_(args...);
		}

	protected:
		template<std::size_t I = 0, typename... Tp>
		XX_FORCEINLINE std::enable_if_t<I == sizeof...(Tp) - 1, int> ReadTuple(std::tuple<Tp...>& t) {
			return Read_(std::get<I>(t));
		}

		template<std::size_t I = 0, typename... Tp>
		XX_FORCEINLINE std::enable_if_t < I < sizeof...(Tp) - 1, int> ReadTuple(std::tuple<Tp...>& t) {
			if (int r = Read_(std::get<I>(t))) return r;
			return ReadTuple<I + 1, Tp...>(t);
		}

		// 内部函数
		template<typename T>
		XX_FORCEINLINE int Read_(T& v) {
			auto& d = *data;
			if constexpr (IsPtrShared_v<T>) {
				using U = typename T::ElementType;
				if constexpr (std::is_same_v<U, ObjBase> || TypeId_v<U> > 0) {
					uint16_t typeId;
					if (int r = Read_(typeId)) return r;
					if (!typeId) {
						if (v) {
							v.Reset();
						}
						return 0;
					}

					auto len = (uint32_t)ptrs.size();
					uint32_t offs;
					if (int r = Read_(offs)) return r;
					if (!offs) return __LINE__;

					if (offs == len + 1) {
						if (!v || v.typeId() != typeId) {
							auto&& o = Create(typeId);
							// Register 时如果要求传入 BaseType 则能避免 dynamic_cast, 实现快速转换. 理论上讲两种方案可并行
							v = o.As<U>();
							if (!v) return __LINE__;
						}
						ptrs.emplace_back(v.pointer);
						if (int r = Read_(*v)) return r;
					}
					else {
						if (offs > len) return __LINE__;
						auto& o = *(ObjBase_s*)&ptrs[offs - 1];
						if (o.typeId() != typeId) return __LINE__;
						v = o.As<U>();
						if (!v) return __LINE__;
					}
				}
				else {
					uint8_t hasValue;
					if (int r = Read_(hasValue)) return r;
					if (!hasValue) {
						v.Reset();
						return 0;
					}
					if (v.Empty()) {
						v = MakeShared<U>();
					}
					return Read_(v.Value());
				}
			}
			else if constexpr (IsPtrWeak_v<T>) {
				Shared<typename T::ElementType> o;
				if (int r = Read_(o)) return r;
				v = o;
			}
			else if constexpr (std::is_base_of_v<ObjBase, T>) {
				return v.Read(*this);
			}
			else if constexpr (IsOptional_v<T>) {
				uint8_t hasValue;
				if (int r = Read_(hasValue)) return r;
				if (!hasValue) {
					v.reset();
					return 0;
				}
				if (!v.has_value()) {
					v.emplace();
				}
				return Read_(v.value());
			}
			else if constexpr (IsVector_v<T>) {
				size_t siz = 0;
				if (int r = Read_(siz)) return r;
				if (d.offset + siz > d.len) return __LINE__;
				v.resize(siz);
				if (siz == 0) return 0;
				auto buf = v.data();
				if constexpr (sizeof(T) == 1 || std::is_floating_point_v<T>) {
					::memcpy(buf, d.buf + d.offset, siz * sizeof(T));
					d.offset += siz * sizeof(T);
				}
				else {
					for (size_t i = 0; i < siz; ++i) {
						if (int r = Read_(buf[i])) return r;
					}
				}
			}
			else if constexpr (IsUnorderedSet_v<T>) {
				size_t siz = 0;
				if (int r = Read_(siz)) return r;
				if (d.offset + siz > d.len) return __LINE__;
				v.clear();
				if (siz == 0) return 0;
				for (size_t i = 0; i < siz; ++i) {
					if (int r = Read_(v.emplace())) return r;
				}
			}
			else if constexpr (std::is_same_v<T, std::string>) {
				size_t siz;
				if (int r = Read_(siz)) return r;
				if (d.offset + siz > d.len) return __LINE__;
				v.assign((char*)d.buf + d.offset, siz);
				d.offset += siz;
			}
			else if constexpr (std::is_same_v<T, xx::Data>) {
				size_t siz;
				if (int r = Read_(siz)) return r;
				if (d.offset + siz > d.len) return __LINE__;
				v.Clear();
				v.WriteBuf(d.buf + d.offset, siz);
				d.offset += siz;
			}
			else if constexpr (std::is_integral_v<T>) {
				if constexpr (sizeof(T) == 1) {
					if (int r = d.ReadFixed(v))  return __LINE__ * 1000000 + r;
				}
				else {
					if (int r = d.ReadVarInteger(v)) return __LINE__ * 1000000 + r;
				}
			}
			else if constexpr (std::is_enum_v<T>) {
				return Read_(*(std::underlying_type_t<T>*) & v);
			}
			else if constexpr (std::is_floating_point_v<T>) {
				if (int r = d.ReadFixed(v))  return __LINE__ * 1000000 + r;
			}
			else if constexpr (IsTuple_v<T>) {
				return ReadTuple(v);
			}
			else if constexpr (IsPair_v<T>) {
				return Read(v.first, v.second);
			}
			else if constexpr (IsMap_v<T> || IsUnorderedMap_v<T>) {
				size_t siz;
				if (int r = Read_(siz)) return r;
				if (siz == 0) return 0;
				if (d.offset + siz * 2 > d.len) return __LINE__;
				for (size_t i = 0; i < siz; ++i) {
					UnorderedMap_Pair_t<T> kv;
					if (int r = Read_(kv.first, kv.second)) return r;
					v.insert(std::move(kv));
				}
				return 0;
			}
			else {
				return ObjFuncs<T>::Read(*this, v);;
			}
			return 0;
		}

		template<typename T, typename ...TS>
		XX_FORCEINLINE int Read_(T& v, TS &...vs) {
			if (auto r = Read_(v)) return r;
			return Read_(vs...);
		}

	public:
		// 由 ObjBase 虚函数 或 不依赖序列化上下文的场景调用
		template<typename...Args>
		XX_FORCEINLINE int Read(Args&...args) {
			return Read_(args...);
		}


		// 向 s 写入数据. 会初始化写入上下文, 并在写入结束后擦屁股( 主要入口 )
		template<typename...Args>
		XX_FORCEINLINE void AppendTo(std::string& s, Args const&...args) {
			static_assert(sizeof...(args) > 0);
			str = &s;
			ptrs.clear();
			auto sg = MakeScopeGuard([this] {
				for (auto&& p : ptrs) {
					*(uint32_t*)p = 0;
				}
				});

			(Append_(args), ...);
		}

		// 内部函数
		template<typename T>
		XX_FORCEINLINE void Append_(T const& v) {
			auto& s = *str;
			if constexpr (IsPtrShared_v<T>) {
				using U = typename T::ElementType;
				if (v) {
					if constexpr (std::is_same_v<U, ObjBase> || TypeId_v<U> > 0) {
						auto h = ((PtrHeader*)v.pointer - 1);
						if (h->offset == 0) {
							ptrs.push_back(&h->offset);
							h->offset = (uint32_t)ptrs.size();
							Append_(*v);
						}
						else {
							s.append(std::to_string(h->offset));
						}
					}
					else {
						ObjFuncs<U>::ToString(*this, *v);
					}
				}
				else {
					s.append("null");
				}
			}
			else if constexpr (IsPtrWeak_v<T>) {
				Append_(v.Lock());
			}
			else if constexpr (std::is_base_of_v<ObjBase, T>) {
				v.ToString(*this);
			}
			else if constexpr (std::is_arithmetic_v<T>) {
				if constexpr (std::is_same_v<bool, T>) {
					s.append(v ? "true" : "false");
				}
				else if constexpr (std::is_same_v<char, T>) {
					s.push_back(v);
				}
				else if constexpr (std::is_floating_point_v<T>) {
					char buf[32];
					snprintf(buf, 32, "%.16lf", (double)v);
					s.append(buf);
				}
				else {
					s.append(std::to_string(v));
				}
			}
			else if constexpr (IsLiteral_v<T> || std::is_same_v<T, char*> || std::is_same_v<T, char const*>) {
				s.append(v);
			}
			else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>) {
				s.push_back('\"');
				s.append(v);
				s.push_back('\"');
			}
			else if constexpr (std::is_enum_v<T>) {
				Append_(*(std::underlying_type_t<T>*) & v);
			}
			else if constexpr (IsOptional_v<T>) {
				if (v.has_value()) {
					Append_(*v);
				}
				else {
					s.append("null");
				}
			}
			else if constexpr (IsVector_v<T> || IsUnorderedSet_v<T>) {
				s.push_back('[');
				if (!v.empty()) {
					for (auto&& o : v) {
						Append_(o);
						s.push_back(',');
					}
					s[s.size() - 1] = ']';
				}
				else {
					s.push_back(']');
				}
			}
			else if constexpr (IsUnorderedMap_v<T> || IsMap_v<T>) {
				s.push_back('[');
				if (!v.empty()) {
					for (auto& kv : v) {
						Append_(kv.first);
						s.push_back(',');
						Append_(kv.second);
						s.push_back(',');
					}
					s[s.size() - 1] = ']';
				}
				else {
					s.push_back(']');
				}
			}
			else if constexpr (IsPair_v<T>) {
				s.push_back('[');
				Append_(v.first);
				s.push_back(',');
				Append_(v.second);
				s.push_back(']');
			}
			else if constexpr (IsTuple_v<T>) {
				s.push_back('[');
				std::apply([&](auto const &... args) {
					Append(args..., ',');
					if constexpr (sizeof...(args) > 0) {
						s.resize(s.size() - 1);
					}
					}, v);
				s.push_back(']');
			}
			else if constexpr (IsTimePoint_v<T>) {
				auto&& t = std::chrono::system_clock::to_time_t(v);
				std::stringstream ss;
				std::tm tm{};
#ifdef _WIN32
				localtime_s(&tm, &t);
#else
				localtime_r(&t, &tm);
#endif
				ss << std::put_time(&tm, "%F %T");
				s.append(ss.str());
			}
			else if constexpr (std::is_same_v<T, Data> || std::is_same_v<T, DataView>) {
				s.push_back('[');
				if (auto inLen = v.len) {
					for (size_t i = 0; i < inLen; ++i) {
						Append_((uint8_t)v[i]);
						s.push_back(',');
					}
					s[s.size() - 1] = ']';
				}
				else {
					s.push_back(']');
				}
			}
			else if constexpr (std::is_same_v<T, std::type_info>) {
				s.push_back('\"');
				s.append(v.name());
				s.push_back('\"');
			}
			else {
				ObjFuncs<T>::ToString(*this, v);
			}
		}

		// 由 ObjBase 虚函数 或 不依赖序列化上下文的场景调用
		template<typename...Args>
		XX_FORCEINLINE void Append(Args const&...args) {
			static_assert(sizeof...(args) > 0);
			(Append_(args), ...);
		}


		// 替代 std::cout。可根据适配模板调用相应的函数
		template<typename...Args>
		XX_FORCEINLINE void Cout(Args const &...args) {
			std::string s;
			AppendTo(s, args...);
			std::cout << s;
		}

		// 在 Cout 基础上添加了换行
		template<typename...Args>
		XX_FORCEINLINE void CoutN(Args const &...args) {
			Cout(args...);
			std::cout << std::endl;
		}

		// 在 CoutN 基础上于头部添加了时间
		template<typename...Args>
		XX_FORCEINLINE void CoutTN(Args const &...args) {
			CoutN("[", std::chrono::system_clock::now(), "] ", args...);
		}


		// 向 out 深度复制 in. 会初始化 ptrs, 并在写入结束后擦屁股( 主要入口 )
		template<typename T>
		XX_FORCEINLINE void Clone(T const& in, T& out) {
			ptrs.clear();
			ptrs2.clear();
			auto sg = MakeScopeGuard([this] {
				for (auto&& p : ptrs) {
					*(uint32_t*)p = 0;
				}
				ptrs.clear();
				});
			Clone1(in, out);
			sg.f();
			Clone2(in, out);
		}

		template<class Tuple, std::size_t N>
		struct TupleForeachClone {
			XX_FORCEINLINE static void Clone1(ObjManager& self, Tuple const& in, Tuple& out) {
				self.Clone1(std::get<N - 1>(in), std::get<N - 1>(out));
				TupleForeachClone<Tuple, N - 1>::Clone1(in, out);
			}

			XX_FORCEINLINE static void Clone2(ObjManager& self, Tuple& out, Tuple const& in) {
				self.Clone1(std::get<N - 1>(in), std::get<N - 1>(out));
				TupleForeachClone<Tuple, N - 1>::Clone1(in, out);
			}
		};

		template<class Tuple>
		struct TupleForeachClone<Tuple, 1> {
			static void Clone1(ObjManager& self, Tuple const& in, Tuple& out) {}
			static void Clone2(ObjManager& self, Tuple const& in, Tuple& out) {}
		};


		template<typename T>
		XX_FORCEINLINE void Clone1(T const& in, T& out) {
			if constexpr (IsPtrShared_v<T>) {
				if (!in) {
					out.Reset();
				}
				else {
					auto h = ((PtrHeader*)in.pointer - 1);
					if (h->offset == 0) {
						ptrs.push_back(&h->offset);
						h->offset = (uint32_t)ptrs.size();

						auto inTypeId = in.typeId();
						if (out.typeId() != inTypeId) {
							out = Create(inTypeId).template As<typename T::ElementType>();
						}
						ptrs2.push_back(out.pointer);
						Clone1(*in, *out);
					}
					else {
						out = *(T*)&ptrs2[h->offset - 1];
					}
				}
			}
			else if constexpr (IsPtrWeak_v<T>) {
				out.Reset();
			}
			else if constexpr (std::is_base_of_v<ObjBase, T>) {
				in.Clone1(*this, (void*)&out);
			}
			else if constexpr (IsOptional_v<T>) {
				if (in.has_value()) {
					if (!out.has_value()) {
						out.emplace();
					}
					Clone1(*in, *out);
				}
				else {
					out.reset();
				}
			}
			else if constexpr (IsVector_v<T>) {
				auto siz = in.size();
				out.resize(siz);
				if constexpr (xx::IsPod_v<typename T::value_type>) {
					memcpy(out.data(), in.data(), siz * sizeof(typename T::value_type));
				}
				else {
					for (size_t i = 0; i < siz; ++i) {
						Clone1(in[i], out[i]);
					}
				}
			}
			else if constexpr (IsUnorderedSet_v<T>) {
				out.clear();
				for (auto&& o : in) {
					Clone1(o, out.emplace());
				}
			}
			else if constexpr (IsTuple_v<T>) {
				TupleForeachClone<T, std::tuple_size_v<T>>::Clone1(*this, in, out);
			}
			else if constexpr (IsPair_v<T>) {
				Clone1(out.first, in.first);
				Clone1(out.second, in.second);
			}
			else if constexpr (IsMap_v<T> || IsUnorderedMap_v<T>) {
				out.clear();
				for (auto&& kv : in) {
					std::pair<typename T::key_type, typename T::value_type> tar;
					Clone1(kv.first, tar.first);
					Clone1(kv.second, tar.second);
					out.insert(std::move(tar));
				}
			}
			else {
				ObjFuncs<T>::Clone1(*this, in, out);
			}
		}

		template<typename T>
		XX_FORCEINLINE void Clone2(T const& in, T& out) {
			if constexpr (IsPtrShared_v<T>) {
				if (in) {
					auto h = ((PtrHeader*)in.pointer - 1);
					if (h->offset == 0) {
						ptrs.push_back(&h->offset);
						h->offset = (uint32_t)ptrs.size();

						Clone2(*in.pointer, *out.pointer);
					}
				}
			}
			else if constexpr (IsPtrWeak_v<T>) {
				if (!in) return;
				if (in.h->offset) {
					out = *(Shared<typename T::ElementType>*) & ptrs2[in.h->offset - 1];
				}
				else {
					out = in;
				}
			}
			else if constexpr (std::is_base_of_v<ObjBase, T>) {
				in.Clone2(*this, (void*)&out);
			}
			else if constexpr (IsOptional_v<T>) {
				if (in.has_value()) {
					Clone2(*in, *out);
				}
			}
			else if constexpr (IsVector_v<T>) {
				auto siz = in.size();
				if constexpr (xx::IsPod_v<typename T::value_type>) {
				}
				else {
					for (size_t i = 0; i < siz; ++i) {
						Clone2(in[i], out[i]);
					}
				}
			}
			else if constexpr (IsUnorderedSet_v<T>) {
				static_assert(xx::IsPod_v<T>);
			}
			else if constexpr (IsTuple_v<T>) {
				TupleForeachClone<T, std::tuple_size_v<T>>::Clone2(*this, in, out);
			}
			else if constexpr (IsPair_v<T>) {
				Clone2(in.first, out.first);
				Clone2(in.second, out.second);
			}
			else if constexpr (IsMap_v<T> || IsUnorderedMap_v<T>) {
				for (auto&& kv : in) {
					auto&& iter = out.find(kv.first);
					//Clone2(kv.first, *(K*)&iter->first);	// 理论上讲 key 应该为简单类型 否则可能出问题
					Clone2(kv.second, iter->second);
				}
			}
			else {
				ObjFuncs<T>::Clone2(*this, in, out);
			}
		}


		// 斩断循环引用的 Shared 以方便顺利释放内存( 入口 )
		template<typename...Args>
		XX_FORCEINLINE void RecursiveResetRoot(Args&...args) {
			static_assert(sizeof...(args) > 0);
			ptrs.clear();
			auto sg = MakeScopeGuard([this] {
				for (auto&& p : ptrs) {
					*(uint32_t*)p = 0;
				}
				});

			(RecursiveReset_(args), ...);
		}

	protected:
		template<typename T>
		XX_FORCEINLINE void RecursiveReset_(T& v) {
			if constexpr (IsPtrShared_v<T>) {
				if (v) {
					auto h = ((PtrHeader*)v.pointer - 1);
					if (h->offset == 0) {
						h->offset = 1;
						ptrs.push_back(&h->offset);
						RecursiveReset_(*v);
					}
					else {
						--h->useCount;
						v.pointer = nullptr;
					}
				}
			}
			else if constexpr (IsPtrWeak_v<T>) {
			}
			else if constexpr (std::is_base_of_v<ObjBase, T>) {
				v.RecursiveReset(*this);
			}
			else if constexpr (IsOptional_v<T>) {
				if (v.has_value()) {
					RecursiveReset_(*v);
				}
			}
			else if constexpr (IsVector_v<T> || IsUnorderedSet_v<T>) {
				for (auto& o : v) {
					RecursiveReset_(o);
				}
			}
			else if constexpr (IsTuple_v<T>) {
				std::apply([this](auto&... args) {
					(RecursiveReset_(args), ...);
					}, v);
			}
			else if constexpr (IsPair_v<T>) {
				RecursiveResets(v.first, v.second);
			}
			else if constexpr (IsMap_v<T> || IsUnorderedMap_v<T>) {
				for (auto& kv : v) {
					RecursiveReset_(kv.second);
				}
			}
			else {
				ObjFuncs<T>::RecursiveReset(*this, v);
			}
		}
	public:

		// 供类成员函数调用
		template<typename...Args>
		XX_FORCEINLINE void RecursiveReset(Args&...args) {
			static_assert(sizeof...(args) > 0);
			(RecursiveReset_(args), ...);
		}
	};
}
