#pragma once
#include "xx_data.h"
#include <string>
#include <vector>
#include <optional>
#include <cmath>

namespace xx {
	
	/************************************************************************************/
	// TypeId 映射
	template<typename T>
	struct TypeId {
		static const uint16_t value = 0;
	};

	template<typename T>
	constexpr uint16_t TypeId_v = TypeId<T>::value;



	/************************************************************************************/
	// Data 序列化 / 反序列化 基础适配模板
	
	struct DataReader;

	template<typename T, typename ENABLED = void>
	struct DataFuncs {
		// 整数变长写( 1字节除外 ), double 看情况, float 拷贝内存, 容器先变长写长度
		static inline void Write(Data& d, T const& in) noexcept {
			assert(false);
		}
		
		// limits 针对 容器类型，提供创建时的长度限制检查
		// 返回非 0 表示操作失败
		template<size_t ...limits>
		static inline int Read(DataReader& d, T& out) noexcept {
			assert(false);
			return 0;
		}
	};


	template<typename T>
	void WriteCore(Data& d, T const& in) {
		DataFuncs<T>::Write(d, in);
	}

	// 便于向 Data 追加内容的工具函数
	template<typename ...Args>
	void Write(Data& d, Args const& ... args) {
		std::initializer_list<int> n{ ((WriteCore(d, args)), 0)... };
		(void)(n);
	}

	
	/************************************************************************************/
	// 容器深度探测 系列: 计算 Container<Container<... 嵌套深度. 返回 0 表明不是 container

	template<typename T>
	struct IsOptional : std::false_type {};

	template<typename T>
	struct IsOptional<std::optional<T>> : std::true_type {};

	template<typename T>
	struct IsVector : std::false_type {};

	template<typename T>
	struct IsVector<std::vector<T>> : std::true_type {};

	template<typename T>
	struct ChildType {
		using type = void;
	};
	template<typename T>
	using ChildType_t = typename ChildType<T>::type;

	template<typename T>
	struct ChildType<std::optional<T>> {
		using type = T;
	};
	template<typename T>
	struct ChildType<std::vector<T>> {
		using type = T;
	};

	template<typename T>
	constexpr size_t DeepLevel(T* const& v) {
		if constexpr (IsOptional<T>::value) return 0 + DeepLevel((ChildType_t<T>*)0);
		if constexpr (IsVector<T>::value) return 1 + DeepLevel((ChildType_t<T>*)0);
		if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, Data>) return 1;
		return 0;
	}

	template<typename T>
	constexpr size_t DeepLevel_v = DeepLevel((std::decay_t<T>*)0);
	
	
	
	/************************************************************************************/
	// Data 查看器 / 反序列化器. 内存为引用模式, 不持有
	
	struct DataReader {
		// 引用到内存地址
		char const *buf = nullptr;
		
		// 数据长度
		size_t len = 0;
		
		// 读指针偏移量
		size_t offset = 0;

		DataReader(char const* const& buf, size_t const& len, size_t const& offset = 0)
			: buf(buf)
			, len(len)
			, offset(offset)
		{
		}

		DataReader(Data const& d) : DataReader(d.buf, d.len) {}

		// 复制构造
		DataReader(DataReader const&) = default;
		DataReader& operator=(DataReader const&) = default;

		// 移动构造
		DataReader(DataReader&& o) {
			this->operator=(std::move(o));
		}
		inline DataReader& operator=(DataReader&& o) {
			std::swap(buf, o.buf);
			std::swap(len, o.len);
			std::swap(offset, o.offset);
			return *this;
		}

		// 重设参数
		inline void Reset(char* const& buf = nullptr, size_t const& len = 0, size_t const& offset = 0) {
			this->buf = buf;
			this->len = len;
			this->offset = offset;
		}

		// 读指定长度 buf 到 tar. 返回非 0 则读取失败
		int ReadBuf(char* const& tar, size_t const& siz) {
			if (offset + siz > len) return -1;
			memcpy(tar, buf + offset, siz);
			offset += siz;
			return 0;
		}

		// 定长读. 返回非 0 则读取失败
		template<typename T, typename ENABLED = std::enable_if_t<std::is_pod_v<T>>>
		int ReadFixed(T& v) {
			return ReadBuf(&v, sizeof(T));
		}
		
		// 变长读. 返回非 0 则读取失败
		template<typename T>
		inline int ReadVarInteger(T& v) {
			using UT = std::make_unsigned_t<T>;
			UT u(0);
			for (size_t shift = 0; shift < sizeof(T) * 8; shift += 7) {
				if (offset == len) return -9;
				auto b = (UT)buf[offset++];
				u |= UT((b & 0x7Fu) << shift);
				if ((b & 0x80) == 0) {
					if constexpr (std::is_signed_v<T>) {
						v = ZigZagDecode(u);
					}
					else {
						v = u;
					}
					return 0;
				}
			}
			return -10;
		}


		template<typename T, typename ...TS>
		int ReadCore(T& v, TS&...vs) {
			if (auto r = DataFuncs<T>::Read(*this, v)) return r;
			return ReadCore(vs...);
		}
		template<typename T>
		int ReadCore(T& v) {
			return DataFuncs<T>::Read(*this, v);
		}

		// 读出并填充到变量. 可同时填充多个. 返回非 0 则读取失败
		template<typename ...TS>
		int Read(TS&...vs) {
			return ReadCore(vs...);
		}


		// 读 string 同时检查长度限制
		template<size_t limit, size_t ...limits>
		int ReadLimitCore(std::string& out) {
			size_t siz = 0;
			if (auto r = ReadVarInteger(siz)) return r;
			if (limit && siz > limit) return -1;
			if (offset + siz > len) return -2;
			out.assign((char*)buf + offset, siz);
			offset += siz;
			return 0;
		}

		// 读 Data 同时检查长度限制
		template<size_t limit, size_t ...limits>
		int ReadLimitCore(Data& out) {
			size_t siz = 0;
			if (auto r = ReadVarInteger(siz)) return r;
			if (limit && siz > limit) return -1;
			if (offset + siz > len) return -2;
			out.Clear();
			out.WriteBuf(buf + offset, siz);
			offset += siz;
			return 0;
		}

		// 读 vector 同时检查长度限制
		template<size_t limit, size_t ...limits, typename T>
		int ReadLimitCore(std::vector<T>& out) {
			size_t siz = 0;
			if (auto rtv = ReadVarInteger(siz)) return rtv;
			if (limit != 0 && siz > limit) return -1;
			if (offset + siz > len) return -2;
			out.resize(siz);
			if (siz == 0) return 0;
			auto outBuf = out.data();
			if constexpr (sizeof...(limits)) {
				for (size_t i = 0; i < siz; ++i) {
					if (int r = ReadLimitCore<limits...>(outBuf[i])) return r;
				}
			}
			else {
				if constexpr (sizeof(T) == 1 || std::is_same_v<float, T>) {
					::memcpy(outBuf, buf + offset, siz * sizeof(T));
					offset += siz * sizeof(T);
				}
				else {
					for (size_t i = 0; i < siz; ++i) {
						if (int r = Read(outBuf[i])) return r;
					}
				}
			}
			return 0;
		}

		// 读 optional<容器> 传递长度限制到下一级
		template<size_t ...limits, typename T>
		int ReadLimitCore(std::optional<T>& out) {
			char hasValue = 0;
			if (int r = Read(hasValue)) return r;
			if (!hasValue) return 0;
			if (!out.has_value()) {
				out.emplace();
			}
			return ReadLimitCore<limits...>(out.value());
		}

		// 读出容器并做长度限制检查
		template<size_t...limits, typename T, typename ENABLED = std::enable_if_t<xx::DeepLevel_v<T> == sizeof...(limits)>>
		int ReadLimit(T& out) {
			return ReadLimitCore<limits...>(out);
		}
	};

	// 标识内存可移动
	template<>
	struct IsPod<DataReader, void> : std::true_type {};



	// 便于从 Data 读出内容的工具函数
	template<typename ...Args>
	int Read(Data& d, Args& ... args) {
		DataReader dv(d);
		return dv.Read(args...);
	}

	// 便于从 buf + len 读出内容的工具函数( 无法中途 ReadLimit, 只适合简单场景 )
	template<typename ...Args>
	int Read(char const* const& buf, size_t const& len, Args& ... args) {
		DataReader dr(buf, len);
		return dr.Read(args...);
	}



	/**********************************************************************************************************************/
	// 开始适配 DataFuncs

	// 适配 Data
	template<>
	struct DataFuncs<Data, void> {
		static inline void Write(Data& d, Data const& in) {
			assert(&in != &d);
			d.WriteVarIntger(in.len);
			d.WriteBuf(in.buf, in.len);
		}
		static inline int Read(DataReader& d, Data& out) {
			return d.ReadLimit<0>(out);
		}
	};

	// 适配 1 字节长度的 数值 或 float( 这些类型直接 memcpy )
	template<typename T>
	struct DataFuncs<T, std::enable_if_t< (std::is_arithmetic_v<T> && sizeof(T) == 1) || (std::is_floating_point_v<T> && sizeof(T) == 4) >> {
		static inline void Write(Data& d, T const& in) {
			d.Reserve(d.len + sizeof(T));
			memcpy(d.buf + d.len, &in, sizeof(T));
			d.len += sizeof(T);
		}
		static inline int Read(DataReader& d, T& out) {
			if (d.offset + sizeof(T) > d.len) return -12;
			memcpy(&out, d.buf + d.offset, sizeof(T));
			d.offset += sizeof(T);
			return 0;
		}
	};

	// 适配 2+ 字节整数( 变长读写 )
	template<typename T>
	struct DataFuncs<T, std::enable_if_t<std::is_integral_v<T> && sizeof(T) >= 2>> {
		static inline void Write(Data& d, T const& in) {
			d.WriteVarIntger(in);
		}
		static inline int Read(DataReader& d, T& out) {
			return d.ReadVarInteger(out);
		}
	};

	// 适配 enum( 根据原始数据类型调上面的适配 )
	template<typename T>
	struct DataFuncs<T, std::enable_if_t<std::is_enum_v<T>>> {
		typedef std::underlying_type_t<T> UT;
		static inline void Write(Data& d, T const& in) {
			DataFuncs<UT>(d, (UT const&)in);
		}
		static inline int Read(DataReader& d, T& out) {
			return d.Read((UT&)out);
		}
	};

	// 适配 double
	template<>
	struct DataFuncs<double, void> {
		static inline void Write(Data& d, double const& in) {
			d.Reserve(d.len + sizeof(double) + 1);
			if (in == 0) {
				d.buf[d.len++] = 0;
			}
			else if (std::isnan(in)) {
				d.buf[d.len++] = 1;
			}
			else if (in == -std::numeric_limits<double>::infinity()) {	// negative infinity
				d.buf[d.len++] = 2;
			}
			else if (in == std::numeric_limits<double>::infinity()) {	// positive infinity
				d.buf[d.len++] = 3;
			}
			else {
				auto i = (int32_t)in;
				if (in == (double)i) {
					d.buf[d.len++] = 4;
					d.WriteVarIntger<int32_t, false>(i);
				}
				else {
					d.buf[d.len] = 5;
					memcpy(d.buf + d.len + 1, &in, sizeof(double));
					d.len += sizeof(double) + 1;
				}
			}
		}
		static inline int Read(DataReader& d, double& out) {
			if (d.offset >= d.len) return -13;	// 确保还有 1 字节可读
			switch (d.buf[d.offset++]) {			// 跳过 1 字节
			case 0:
				out = 0;
				return 0;
			case 1:
				out = std::numeric_limits<double>::quiet_NaN();
				return 0;
			case 2:
				out = -std::numeric_limits<double>::infinity();
				return 0;
			case 3:
				out = std::numeric_limits<double>::infinity();
				return 0;
			case 4: {
				int32_t i = 0;
				if (auto rtv = DataFuncs<int32_t>::Read(d, i)) return rtv;
				out = i;
				return 0;
			}
			case 5: {
				if (d.len < d.offset + sizeof(double)) return -14;
				memcpy(&out, d.buf + d.offset, sizeof(double));
				d.offset += sizeof(double);
				return 0;
			}
			default:
				return -15;							// failed
			}
		}
	};


	// 适配 literal char[len] string  ( 写入 变长长度-1 + 内容. 不写入末尾 0 )
	template<size_t len>
	struct DataFuncs<char[len], void> {
		static inline void Write(Data& d, char const(&in)[len]) {
			d.WriteVarIntger((size_t)(len - 1));
			d.WriteBuf((char*)in, len - 1);
		}
		static inline int Read(DataReader& d, char(&out)[len]) {
			size_t readLen = 0;
			if (auto r = d.Read(readLen)) return r;
			if (d.offset + readLen > d.len) return -19;
			if (readLen >= len) return -20;
			memcpy(out, d.buf + d.offset, readLen);
			out[readLen] = 0;
			d.offset += readLen;
			return 0;
		}
	};

	// 适配 std::string ( 写入 变长长度 + 内容 )
	template<>
	struct DataFuncs<std::string, void> {
		static inline void Write(Data& d, std::string const& in) {
			d.WriteVarIntger(in.size());
			d.WriteBuf((char*)in.data(), in.size());
		}
		static inline int Read(DataReader& d, std::string& out) {
			return d.ReadLimit<0>(out);
		}
	};



	// 适配 std::optional<T>
	template<typename T>
	struct DataFuncs<std::optional<T>, void> {
		static inline void Write(Data& d, std::optional<T> const& in) {
			if (in.has_value()) {
				d.WriteFixed((char)1, in.value());
			}
			else {
				d.WriteFixed((char)0);
			}
		}
		static inline int Read(DataReader& d, std::optional<T>& out) {
			char hasValue = 0;
			if (int r = d.Read(hasValue)) return r;
			if (!hasValue) return 0;
			if (!out.has_value()) {
				out.emplace();
			}
			return d.Read(out.value());
		}
	};

	// 适配 std::vector<T>
	template<typename T>
	struct DataFuncs<std::vector<T>, void> {
		static inline void Write(Data& d, std::vector<T> const& in) {
			auto buf = in.data();
			auto len = in.size();
			d.Reserve(d.len + 5 + len * sizeof(T));
			d.WriteVarIntger(len);
			if (!len) return;
			if constexpr (sizeof(T) == 1 || std::is_same_v<float, T>) {
				::memcpy(d.buf + d.len, buf, len * sizeof(T));
				d.len += len * sizeof(T);
			}
			else {
				for (size_t i = 0; i < len; ++i) {
					DataFuncs<T>::Write(d, buf[i]);
				}
			}
		}
		// 内容为 ReadLimit 的精简版
		static inline int Read(DataReader& d, std::vector<T>& out) {
			size_t siz = 0;
			if (auto rtv = d.Read(siz)) return rtv;
			if (d.offset + siz > d.len) return -2;
			out.resize(siz);
			if (siz == 0) return 0;
			auto buf = out.data();
			if constexpr (sizeof(T) == 1 || std::is_same_v<float, T>) {
				::memcpy(buf, d.buf + d.offset, siz * sizeof(T));
				d.offset += siz * sizeof(T);
			}
			else {
				for (size_t i = 0; i < siz; ++i) {
					if (int r = d.Read(buf[i])) return r;
				}
			}
			return 0;

		}
	};
	
}
