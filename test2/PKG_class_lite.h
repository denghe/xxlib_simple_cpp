#pragma once
#include "xx_object.h"
#include "PKG_class_lite.h.inc"  // user create it for extend include files
namespace PKG {
	struct PkgGenMd5 {
		inline static const std::string value = "#*MD5<03b90734596cc8486fd2a848e9c7e13b>*#";
    };
	struct PkgGenTypes {
        static void RegisterTo(xx::ObjectCreators& oc);
    };

    struct C;
    struct Node;
    struct D;
    struct Scene;
    struct A {
        XX_GENCODE_STRUCT_H(A)
        int32_t x = 0;
        int32_t y = 0;
        std::shared_ptr<PKG::C> c;
    };
    struct B : PKG::A {
        XX_GENCODE_STRUCT_H(B)
        int32_t z = 0;
        std::weak_ptr<PKG::C> wc;
    };
    struct C : xx::Object {
        XX_GENCODE_OBJECT_H(C, xx::Object)
        PKG::A a;
        PKG::B b;
    };
    struct Node : xx::Object {
        XX_GENCODE_OBJECT_H(Node, xx::Object)
        std::weak_ptr<PKG::Node> parent;
        std::vector<std::shared_ptr<PKG::Node>> childs;
    };
    struct D : PKG::C {
        XX_GENCODE_OBJECT_H(D, PKG::C)
        std::string name;
    };
    struct Scene : PKG::Node {
        XX_GENCODE_OBJECT_H(Scene, PKG::Node)
    };
}
namespace xx {
	template<>
	struct StringFuncs<PKG::A, void> {
		static void Append(std::string& s, PKG::A const& in);
		static void AppendCore(std::string& s, PKG::A const& in);
    };
	template<>
	struct DataFuncsEx<PKG::A, void> {
		static void Write(DataWriterEx& dw, PKG::A const& in);
		static int Read(DataReaderEx& dr, PKG::A& out);
	};
	template<>
	struct StringFuncs<PKG::B, void> {
		static void Append(std::string& s, PKG::B const& in);
		static void AppendCore(std::string& s, PKG::B const& in);
    };
	template<>
	struct DataFuncsEx<PKG::B, void> {
		static void Write(DataWriterEx& dw, PKG::B const& in);
		static int Read(DataReaderEx& dr, PKG::B& out);
	};
}