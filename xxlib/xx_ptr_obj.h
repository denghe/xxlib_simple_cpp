#pragma once

#include "xx_ptr.h"
#include "xx_data.h"
#include <unordered_map>
#include <array>

namespace xx {

	/************************************************************************************/
	// ObjBase: 仅用于 Shared<> Weak<> 包裹的类型基类
	// 方便复制
	/*
	inline void Write(xx::ObjManager& o) const override { }
	inline void Read(xx::ObjManager& o) override { }
	inline void ToString(xx::ObjManager& o) const override { }
	inline void ToStringCore(xx::ObjManager& o) const override { }
	inline void Clone1(xx::ObjManager& o, xx::ObjBase_s const& tar) const override { }
	inline void Clone2(xx::ObjManager& o, xx::ObjBase_s const& tar) const override { }
	*/

	struct ObjBase;
	using ObjBase_s = Shared<ObjBase>;

	struct ObjManager;

	struct ObjBase {
		ObjBase() = default;

		virtual ~ObjBase() = default;

		// 序列化
		virtual void Write(ObjManager& o) const = 0;

		// 反序列化
		virtual void Read(ObjManager& o) = 0;

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
		std::string str;


		// 类实例 创建函数
		typedef ObjBase_s(*FT)();

		// typeId : 类实例 创建函数 映射容器
		std::array<FT, std::numeric_limits<uint16_t>::max()> fs{};

		// 注册类型 & ptrTypeId. 将创建函数塞入容器
		template<typename T>
		void Register(uint32_t const& typeId = TypeId_v<T>) {
			static_assert(std::is_base_of_v<ObjBase, T>);
			fs[typeId] = []() -> ObjBase_s { return MakeShared<T>(); };
		}

		// 根据 typeId 来创建对象. 失败返回空
		template<typename T = ObjBase>
		inline Shared<T> Create(uint16_t const& typeId) {
			static_assert(std::is_base_of_v<ObjBase, T>);
			if (!fs[typeId]) return nullptr;
			return fs[typeId]();
		}



		// 向 data 写入数据. 会初始化写入上下文, 并在写入结束后擦屁股( 主要入口 )
		template<typename...Args>
		void WriteTo(Data& d, Args&&...args) {
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

		// 内部函数
		template<typename T>
		void Write_(T const& v) {
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
						//h.offset = (uint32_t)(d.len - baseLen);
						d.WriteVarIntger(h.offset);
						v.pointer->Write(*this);
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
				auto buf = v.data();
				auto len = v.size();
				d.Reserve(d.len + 5 + len * sizeof(T));
				d.WriteVarIntger(len);
				if (!len) return;
				if constexpr (sizeof(T) == 1 || std::is_floating_point_v<T>) {
					memcpy(d.buf + d.len, buf, len * sizeof(T));
					d.len += len * sizeof(T);
				}
				else {
					for (size_t i = 0; i < len; ++i) {
						Write_(buf[i]);
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
			else {
				throw __LINE__;
			}
			// else map? pair? tuple? ...
			// else if constexpr (HasMember_Write for normal struct support?
		}

		// 由 ObjBase 虚函数 或 不依赖序列化上下文的场景调用
		template<typename...Args>
		void Write(Args&&...args) {
			static_assert(sizeof...(args) > 0);
			(Write_(std::forward<Args>(args)), ...);
		}


		// 从 data 读入 / 反序列化, 填充到 v. 原则: 尽量复用, 不新建对象( 主要入口 )
		// 可传入开始读取的位置
		template<typename...Args>
		void ReadFrom(Data& d, Args&...args) {
			static_assert(sizeof...(args) > 0);
			data = &d;
			ptrs.clear();
			(Read_(args), ...);
		}

		// 内部函数
		template<typename T>
		void Read_(T& v) {
			auto& d = *data;
			if constexpr (IsPtrShared_v<T>) {
				using U = typename T::ElementType;
				if constexpr (std::is_same_v<U, ObjBase> || TypeId_v<U> > 0) {
					uint16_t typeId;
					Read_(typeId);
					if (!typeId) return;

					auto len = (uint32_t)ptrs.size();
					uint32_t offs;
					Read_(offs);

					if (offs == len + 1) {
						if (!v || v.typeId() != typeId) {
							auto&& o = Create(typeId);
							// Register 时如果要求传入 BaseType 则能避免 dynamic_cast, 实现快速转换. 理论上讲两种方案可并行
							v = o.As<U>();
							if (!v) throw __LINE__;
						}
						ptrs.emplace_back(v.pointer);
						v->Read(*this);
					}
					else {
						if (offs > len)throw __LINE__;
						auto& o = *(ObjBase_s*)&ptrs[offs - 1];
						if (o.typeId() != typeId) throw __LINE__;
						v = o.As<U>();
						if (!v) throw __LINE__;
					}
				}
				else {
					uint8_t hasValue;
					Read_(hasValue);
					if (!hasValue) {
						v.Reset();
						return;
					}
					if (v.Empty()) {
						v = MakeShared<U>();
					}
					Read_(v.Value());
				}
			}
			else if constexpr (IsPtrWeak_v<T>) {
				Shared<typename T::ElementType> o;
				Read_(o);
				v = o;
			}
			else if constexpr (IsOptional<T>::value) {
				uint8_t hasValue;
				Read_(hasValue);
				if (!hasValue) {
					v.reset();
					return;
				}
				if (!v.has_value()) {
					v.emplace();
				}
				Read_(v.value());
			}
			else if constexpr (IsVector<T>::value) {
				size_t siz = 0;
				Read_(siz);
				if (d.offset + siz > d.len) throw __LINE__;
				v.resize(siz);
				if (siz == 0) return;
				auto buf = v.data();
				if constexpr (sizeof(T) == 1 || std::is_floating_point_v<T>) {
					::memcpy(buf, d.buf + d.offset, siz * sizeof(T));
					d.offset += siz * sizeof(T);
				}
				else {
					for (size_t i = 0; i < siz; ++i) {
						Read_(buf[i]);
					}
				}
			}
			else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>) {
				size_t siz;
				Read_(siz);
				if (d.offset + siz > d.len) throw __LINE__;
				v.assign((char*)d.buf + d.offset, siz);
				d.offset += siz;
			}
			else if constexpr (std::is_integral_v<T>) {
				if constexpr (sizeof(T) == 1) {
					if (int r = d.ReadFixed(v))  throw __LINE__ * 1000000 + r;
				}
				else {
					if (int r = d.ReadVarInteger(v)) throw __LINE__ * 1000000 + r;
				}
			}
			else if constexpr (std::is_enum_v<T>) {
				Read_(*(std::underlying_type_t<T>*) & v);
			}
			else if constexpr (std::is_floating_point_v<T>) {
				if (int r = d.ReadFixed(v))  throw __LINE__ * 1000000 + r;
			}
			else {
				throw std::runtime_error("write: unsupported type");
			}

			// else map? pair? tuple? ...
			// else if constexpr (HasMember_Write for normal struct support?
		}

		// 由 ObjBase 虚函数 或 不依赖序列化上下文的场景调用
		template<typename...Args>
		void Read(Args&...args) {
			(Read_(args), ...);
		}


		// todo: ToString, Cout

		// 内部函数
		template<typename T>
		void Append_(T const& v) {
		//	if constexpr (IsPtrShared_v<T>) {
		//		using U = typename T::ElementType;
		//		if constexpr (std::is_same_v<U, ObjBase> || TypeId_v<U> > 0) {
		//			auto typeId = v.typeId();
		//			d.WriteVarIntger(typeId);
		//			if (typeId == 0) return;

		//			auto&& h = PtrHeader::Get(v.pointer);
		//			if (h.offset == 0) {
		//				ptrs.push_back(&h.offset);
		//				h.offset = (uint32_t)ptrs.size();
		//				//h.offset = (uint32_t)(d.len - baseLen);
		//				d.WriteVarIntger(h.offset);
		//				v.pointer->Write(*this);
		//			}
		//			else {
		//				d.WriteVarIntger(h.offset);
		//			}
		//		}
		//		else {
		//			if (v) {
		//				d.WriteFixed((uint8_t)1);
		//				Write_(*v);
		//			}
		//			else {
		//				d.WriteFixed((uint8_t)0);
		//			}
		//		}
		//	}
		//	else if constexpr (IsPtrWeak_v<T>) {
		//		Write_(v.Lock());
		//	}
		//	else if constexpr (IsOptional<T>::value) {
		//		if (v.has_value()) {
		//			d.WriteFixed((uint8_t)1);
		//			Write_(*v);
		//		}
		//		else {
		//			d.WriteFixed((uint8_t)0);
		//		}
		//	}
		//	else if constexpr (IsVector<T>::value) {
		//		auto buf = v.data();
		//		auto len = v.size();
		//		d.Reserve(d.len + 5 + len * sizeof(T));
		//		d.WriteVarIntger(len);
		//		if (!len) return;
		//		if constexpr (sizeof(T) == 1 || std::is_floating_point_v<T>) {
		//			memcpy(d.buf + d.len, buf, len * sizeof(T));
		//			d.len += len * sizeof(T);
		//		}
		//		else {
		//			for (size_t i = 0; i < len; ++i) {
		//				Write_(buf[i]);
		//			}
		//		}
		//	}
		//	else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>) {
		//		d.WriteVarIntger(v.size());
		//		d.WriteBuf(v.data(), v.size());
		//	}
		//	else if constexpr (std::is_integral_v<T>) {
		//		if constexpr (sizeof(T) == 1) {
		//			d.WriteFixed(v);
		//		}
		//		else {
		//			d.WriteVarIntger(v);
		//		}
		//	}
		//	else if constexpr (std::is_enum_v<T>) {
		//		Write_(*(std::underlying_type_t<T>*) & v);
		//	}
		//	else if constexpr (std::is_floating_point_v<T>) {
		//		d.WriteFixed(v);
		//	}
		//	else {
		//		throw __LINE__;
		//	}
		//	// else map? pair? tuple? ...
		//	// else if constexpr (HasMember_Write for normal struct support?
		}
	};
}
