#pragma once
#include "xx_base.h"
#include "xx_math.h"
#include <string.h>

namespace xx {
	// ������Ķ�������������, bbuffer �Ļ���
	// ����ģʽ: 1. ׷��ģʽ    2. ֻ������ģʽ
	struct Data {
		char*				buf = nullptr;
		size_t				len = 0;
		size_t				cap = 0;

		// buf ͷ��Ԥ���ռ��С. ������Ҫװ���� sizeof(size_t)
		static const size_t	recvLen = 16;

		// cap ��������÷� 2 ������ֵ
		static const size_t	special = (size_t)-1;

		Data() = default;

		// Ԥ����ռ� ����
		explicit Data(size_t const& newCap) {
			if (newCap) {
				auto siz = Round2n(recvLen + cap);
				buf = (char*)::malloc(siz) + recvLen;
				cap = siz - recvLen;
			}
		}

		// ͨ�� ����һ������ ������
		Data(char const* const& ptr, size_t const& siz) {
			WriteBuf(ptr, siz);
		}

		// ͨ�� ��ʼ���б� ������
		Data(std::initializer_list<char> bytes)	: Data( bytes.begin(), bytes.size() ){}

		// ���ƹ���
		Data(Data const& o) {
			operator=(o);
		}
		inline Data& operator=(Data const& o) {
			if (o.cap == special) {
				buf = o.buf;
				len = o.len;
				cap = o.cap;
				++Refs();
			}
			else {
				Clear();
				WriteBuf(o.buf, o.len);
			}
			return *this;
		}

		// �ƶ�����
		Data(Data&& o) {
			operator=(std::move(o));
		}
		inline Data& operator=(Data&& o) {
			std::swap(buf, o.buf);
			std::swap(len, o.len);
			std::swap(cap, o.cap);
			return *this;
		}

		// �ж������Ƿ�һ��
		inline bool operator==(Data const& o) {
			if (&o == this) return true;
			if (len != o.len) return false;
			return 0 == ::memcmp(buf, o.buf, len);
		}

		// ȷ���ռ��㹻
		inline void Reserve(size_t const& newCap) {
			assert(cap != special);
			if (newCap <= cap) return;

			auto siz = Round2n(recvLen + newCap);
			auto newBuf = (char*)::malloc(siz) + recvLen;
			::memcpy(newBuf, buf, len);

			// �����ж� cap ���ж� buf, ����Ϊ gcc �Ż��ᵼ�� if ʧЧ, ������ζ���ִ�� free
			if (cap) {
				::free(buf - recvLen);
			}
			buf = newBuf;
			cap = siz - recvLen;
		}

		// ���ؾɳ���
		inline size_t Resize(size_t const& newLen) {
			assert(cap != special);
			if (newLen > len) {
				Reserve(newLen);
			}
			auto rtv = len;
			len = newLen;
			return rtv;
		}

		// �±����
		inline char& operator[](size_t const& idx) {
			assert(idx < len);
			return buf[idx];
		}
		inline char const& operator[](size_t const& idx) const {
			assert(idx < len);
			return buf[idx];
		}


		// ��ͷ���Ƴ�ָ����������( �����ڲ�������Ƴ����Ѿ����ʹ��İ�����, �����������ƶ���ͷ�� )
		inline void RemoveFront(size_t const& siz) {
			assert(cap != special);
			assert(siz <= len);
			if (!siz) return;
			len -= siz;
			if (len) {
				::memmove(buf, buf + siz, len);
			}
		}

		// ׷��д��һ�� buf
		inline void WriteBuf(char const* const& ptr, size_t const& siz) {
			assert(cap != special);
			Reserve(len + siz);
			::memcpy(buf + len, ptr, siz);
			len += siz;
		}

		// ����Ϊֻ��ģʽ, ����ʼ�����ü���( ����ֻ�����ü���ģʽ. û���ݲ������� )
		inline void SetReadonlyMode() {
			assert(cap != special);
			assert(len);
			cap = special;
			Refs() = 1;
		}

		// �ж��Ƿ�Ϊֻ��ģʽ
		inline bool Readonly() const {
			return cap == special;
		}

		// �������ü���
		inline size_t& Refs() const {
			assert(cap == special);
			return *(size_t*)(buf - recvLen);
		}

		// ����ģʽ����, ׷��ģʽ�ͷ� buf
		~Data() {
			if (cap == special && --Refs()) return;
			if (cap) {
				::free(buf - recvLen);
			}
		}

		// len �� 0, �ɳ����ͷ� buf
		inline void Clear(bool const& freeBuf = false) {
			assert(cap != special);
			if (freeBuf && cap) {
				::free(buf - recvLen);
				buf = nullptr;
				cap = 0;
			}
			len = 0;
		}
	};

	template<>
	struct IsPod<Data, void> : std::true_type {};
}
