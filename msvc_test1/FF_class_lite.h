#pragma once
#include "xx_obj.h"
#include "FF_class_lite.h.inc"  // user create it for extend include files
namespace FF {
	struct PkgGenMd5 {
		inline static const ::std::string value = "#*MD5<52784e4f39db46d57696969c07cc75bf>*#";
    };
	struct PkgGenTypes {
        static void RegisterTo(::xx::ObjManager& om);
    };

    struct A;
    struct B;
}
namespace xx {
    template<> struct TypeId<::FF::A> { static const uint16_t value = 1; };
    template<> struct TypeId<::FF::B> { static const uint16_t value = 2; };
}
namespace FF {

    struct C {
        XX_GENCODE_STRUCT_H(C)
        float x = 0;
        float y = 0;
        ::std::vector<::xx::Weak<::FF::A>> targets;
    };
    struct A : ::xx::ObjBase {
        XX_GENCODE_OBJECT_H(A, ::xx::ObjBase)
        int32_t id = 0;
        ::std::optional<::std::string> nick;
        ::xx::Weak<::FF::A> parent;
        ::std::vector<::xx::Shared<::FF::A>> children;
#include "FF_A.inc"
    };
    struct B : ::FF::A {
        XX_GENCODE_OBJECT_H(B, ::FF::A)
        ::xx::Data data;
        ::FF::C c;
        ::std::optional<::FF::C> c2;
        ::std::vector<::std::vector<::std::optional<::FF::C>>> c3;
#include "FF_B.inc"
    };
}
namespace xx {
	template<>
	struct ObjFuncs<::FF::C, void> {
		static void Write(ObjManager& om, ::FF::C const& in);
		static int Read(ObjManager& om, ::FF::C& out);
		static void ToString(ObjManager& om, ::FF::C const& in);
		static void ToStringCore(ObjManager& om, ::FF::C const& in);
		static void Clone1(ObjManager& om, ::FF::C const& in, ::FF::C& out);
		static void Clone2(ObjManager& om, ::FF::C const& in, ::FF::C& out);
		static void RecursiveReset(ObjManager& om, ::FF::C& in);
	};
}
#include "FF_class_lite_.h.inc"  // user create it for extend include files at the end
