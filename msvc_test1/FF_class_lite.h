#pragma once
#include "xx_obj.h"
#include "FF_class_lite.h.inc"  // user create it for extend include files
namespace FF {
	struct PkgGenMd5 {
		inline static const std::string value = "#*MD5<d0f87e82bdf16cd45af24d55acf51709>*#";
    };
	struct PkgGenTypes {
        static void RegisterTo(xx::ObjManager& om);
    };

    struct A;
}
namespace xx {
    template<> struct TypeId<FF::A> { static const uint16_t value = 1; };
}
namespace FF {

    struct A : xx::ObjBase {
        XX_GENCODE_OBJECT_H(A, xx::ObjBase)
        int32_t id = 0;
        xx::Weak<FF::A> parent;
        std::vector<xx::Shared<FF::A>> children;
#include "FF_A.inc"
    };
}
namespace xx {
}
#include "FF_class_lite_.h.inc"  // user create it for extend include files at the end
