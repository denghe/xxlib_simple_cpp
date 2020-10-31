#pragma once

#include "xx_ptr.h"
#include "xx_data.h"

namespace xx {

	struct BytesWriter {
		std::vector<void*> ptrs;
		size_t baseLen = 0;
		xx::Data* data = nullptr;

		template<typename T>
		void Write(xx::Data& d, xx::Shared<T> const& v) {
			data = &d;
			baseLen = d.len;
			assert(ptrs.empty());

			(*this)(v);

			for (auto&& p : ptrs) {
				*(int*)p = 0;
			}
			ptrs.clear();
		}

		template<typename T>
		BytesWriter& operator()(T const& v) {
			auto& d = *data;
			if constexpr (IsPtrShared_v<T>) {
				if constexpr (PtrTypeId_v<typename T::ElementType> > 0) {
					auto typeId = v.typeId();
					d.WriteVarIntger(typeId);
					if (typeId == 0) return *this;

					auto&& h = PtrHeader::Get(v.pointer);
					if (h.offset == 0) {
						h.offset = (uint32_t)(d.len - baseLen);
						ptrs.push_back(&h.offset);
						d.WriteVarIntger(h.offset);
						(*v.pointer)(*this);
					}
					else {
						d.WriteVarIntger(h.offset);
					}
				}
				else {
					if (v) {
						d.WriteFixed((uint8_t)1);
						(*this)(*v);
					}
					else {
						d.WriteFixed((uint8_t)0);
					}
				}
			}
			else if constexpr (IsPtrWeak_v<T>) {
				(*this)(v.Lock());
			}
			else if constexpr (IsOptional<T>::value) {
				if (v.has_value()) {
					d.WriteFixed((uint8_t)1);
					(*this)(*v);
				}
				else {
					d.WriteFixed((uint8_t)0);
				}
			}
			else if constexpr (IsVector<T>::value) {
				d.WriteVarIntger(v.size());
				for (auto&& o : v) {
					(*this)(o);
				}
			}
			else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>) {
				d.WriteVarIntger(v.size());
				d.WriteBuf(v.data(), v.size());
			}
			else if constexpr (std::is_integral_v<T>) {
				d.WriteVarIntger(v);
			}
			else if constexpr (std::is_enum_v<T>) {
				d.WriteVarIntger(std::underlying_type_t<T>(v));
			}
			else if constexpr (std::is_floating_point_v<T>) {
				d.WriteFixed(v);
			}
			else {
				throw std::runtime_error("write: unsupported type");
			}
			// else map? pair? tuple? ...
			// else if constexpr (HasMember_Write for normal struct support?
			return *this;
		}
	};
}
