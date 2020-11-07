#pragma once

#include "xx_ptr.h"
#include "xx_data.h"
#include "xx_typename_islambda.h"

namespace xx {

	struct ObjBase;
	using ObjBase_s = Shared<ObjBase>;
	struct ObjManager;

	/************************************************************************************/
	// 接口函数适配模板. 特化 以扩展类型支持
	template<typename T, typename ENABLED = void>
	struct ObjFuncs {
		static inline void Write(ObjManager& om, T const& in) {
		}
		static inline int Read(ObjManager& om, T& out) {
			return 0;
		}
		static inline void ToString(ObjManager& om, T const& in) {
		}
		static inline void ToStringCore(ObjManager& om, T const& in) {
		}
		static inline void Clone1(ObjManager& om, T const& in, T& out) {
		}
		static inline void Clone2(ObjManager& om, T const& in, T& out) {
		}
	};


	/************************************************************************************/
	// ObjBase: 仅用于 Shared<> Weak<> 包裹的类型基类
	// 方便复制
	/*
	inline void Write(xx::ObjManager& o) const override { }
	inline int Read(xx::ObjManager& o) override { }
	inline void ToString(xx::ObjManager& o) const override { }
	inline void ToStringCore(xx::ObjManager& o) const override { }
	inline void Clone1(xx::ObjManager& o, xx::ObjBase_s const& tar) const override { }
	inline void Clone2(xx::ObjManager& o, xx::ObjBase_s const& tar) const override { }
	*/

	struct ObjBase {
		ObjBase() = default;

		virtual ~ObjBase() = default;

		// 序列化
		virtual void Write(ObjManager& o) const = 0;

		// 反序列化
		virtual int Read(ObjManager& o) = 0;

		// 输出 json 长相时用于输出外包围 {  } 部分
		virtual void ToString(ObjManager& o) const = 0;

		// 输出 json 长相时用于输出花括号内部的成员拼接
		virtual void ToStringCore(ObjManager& o) const = 0;

		// 克隆步骤1: 拷贝普通数据，遇到 shared_ptr 就同型新建, 并保存映射关系
		virtual void Clone1(ObjManager& o, ObjBase_s const& tar) const = 0;

		// 克隆步骤2: 只处理成员中的 weak_ptr 类型。根据步骤 1 建立的映射关系来填充
		virtual void Clone2(ObjManager& o, ObjBase_s const& tar) const = 0;
	};


	/************************************************************************************/
	// ObjBase 相关操作类. 注册 typeId 与 关联 Create 函数

	struct ObjManager {
		// 公共上下文
		std::vector<void*> ptrs;
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
		XX_FORCEINLINE void WriteTo(Data& d, Args&&...args) {
			static_assert(sizeof...(args) > 0);
			data = &d;
			ptrs.clear();
			auto sg = MakeScopeGuard([this] {
				for (auto&& p : ptrs) {
					*(uint32_t*)p = 0;
				}
				});

			(Write_(std::forward<Args>(args)), ...);
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

					auto&& h = PtrHeader::Get(v.pointer);
					if (h.offset == 0) {
						ptrs.push_back(&h.offset);
						h.offset = (uint32_t)ptrs.size();
						d.WriteVarIntger(h.offset);
						Write_(*v.pointer);
					}
					else {
						d.WriteVarIntger(h.offset);
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
				Write_(v.Lock());
			}
			else if constexpr (std::is_base_of_v<ObjBase, T>) {
				v.Write(*this);
			}
			else if constexpr (IsOptional<T>::value) {
				if (v.has_value()) {
					d.WriteFixed((uint8_t)1);
					Write_(*v);
				}
				else {
					d.WriteFixed((uint8_t)0);
				}
			}
			else if constexpr (IsVector<T>::value) {
				d.WriteVarIntger(v.size());
				if (v.empty()) return;
				if constexpr (sizeof(T) == 1 || std::is_floating_point_v<T>) {
					d.WriteBuf(v.data(), v.size() * sizeof(T));
				}
				else if constexpr (std::is_integral_v<typename T::value_type>) {
					for (auto&& o : v) {
						d.WriteVarIntger(o);
					}
				}
				else {
					for (auto&& o : v) {
						Write_(o);
					}
				}
			}
			else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>) {
				d.WriteVarIntger(v.size());
				d.WriteBuf(v.data(), v.size());
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
			// else map? pair? ...
			else {
				ObjFuncs<T>::Write(*this, v);
			}
		}

	public:
		// 转发到 Write_
		template<typename...Args>
		XX_FORCEINLINE void Write(Args&&...args) {
			static_assert(sizeof...(args) > 0);
			(Write_(std::forward<Args>(args)), ...);
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
					if (!typeId) return 0;

					auto len = (uint32_t)ptrs.size();
					uint32_t offs;
					if (int r = Read_(offs)) return r;

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
						if (offs > len)return __LINE__;
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
			else if constexpr (IsOptional<T>::value) {
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
			else if constexpr (IsVector<T>::value) {
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
			else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>) {
				size_t siz;
				if (int r = Read_(siz)) return r;
				if (d.offset + siz > d.len) return __LINE__;
				v.assign((char*)d.buf + d.offset, siz);
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
			// else map? pair? ...
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
		XX_FORCEINLINE void AppendTo(std::string& s, Args&&...args) {
			static_assert(sizeof...(args) > 0);
			str = &s;
			ptrs.clear();
			auto sg = MakeScopeGuard([this] {
				for (auto&& p : ptrs) {
					*(uint32_t*)p = 0;
				}
				});

			(Append_(std::forward<Args>(args)), ...);
		}

		// 内部函数
		template<typename T>
		XX_FORCEINLINE void Append_(T const& v) {
			auto& s = *str;
			if constexpr (IsPtrShared_v<T>) {
				using U = typename T::ElementType;
				if (v) {
					if constexpr (std::is_same_v<U, ObjBase> || TypeId_v<U> > 0) {
						v->ToString(*this);
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
			else if constexpr (std::is_same_v<T, char*> || std::is_same_v<T, char const*> || IsLiteral_v<T>) {
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
			else if constexpr (std::is_same_v<std::decay<T>, Data> || std::is_same_v<std::decay<T>, DataView>) {
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
		XX_FORCEINLINE void Append(Args&&...args) {
			static_assert(sizeof...(args) > 0);
			(Append_(std::forward<Args>(args)), ...);
		}


		// 替代 std::cout。可根据适配模板调用相应的函数
		template<typename...Args>
		inline void Cout(Args const &...args) {
			std::string s;
			AppendTo(s, args...);
			std::cout << s;
		}

		// 在 Cout 基础上添加了换行
		template<typename...Args>
		inline void CoutN(Args const &...args) {
			Cout(args...);
			std::cout << std::endl;
		}

		// 在 CoutN 基础上于头部添加了时间
		template<typename...Args>
		inline void CoutTN(Args const &...args) {
			CoutN("[", std::chrono::system_clock::now(), "] ", args...);
		}

	};
}
