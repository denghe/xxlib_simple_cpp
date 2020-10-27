using System;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Collections.Generic;

// 基于 xx_object.h
// 针对 struct 或标记有 [Struct] 的 class: 生成 DataFuncsEx 模板适配
// 针对 Shared<T>, 生成 std::shared_ptr 类型, 无需模板适配. 但生成 ObjectCreators.Register. 要求每个类自己标注 [TypeId]. 检查到漏标就报错
// 针对 Weak<T> 类成员属性, 生成 std::weak_ptr

public static class GenCPP_Class_Lite {
    // 存放标记了 [TypeId(xxx)] 的 id, type 映射
    static Dictionary<ushort, Type> typeIdMappings = new Dictionary<ushort, Type>();

    static void GenH_Head(this StringBuilder sb, string templateName) {
        sb.Append(@"#pragma once
#include ""xx_object.h""
#include """ + templateName + @"_class_lite.h.inc""  // user create it for extend include files
namespace " + templateName + @" {
	struct PkgGenMd5 {
		inline static const std::string value = """ + StringHelpers.MD5PlaceHolder + @""";
    };
	struct PkgGenTypes {
        static void RegisterTo(xx::ObjectHelper& oh);
    };
");
    }


    static void GenH_ClassPredefine(this StringBuilder sb, List<Type> cs, string templateName, Assembly asm) {
        for (int i = 0; i < cs.Count; ++i) {
            var c = cs[i];
            var o = asm.CreateInstance(c.FullName);

            if (c.Namespace != null && (i == 0 || (i > 0 && cs[i - 1].Namespace != c.Namespace))) // namespace 去重
            {
                sb.Append(@"
namespace " + c.Namespace.Replace(".", "::") + @" {");
            }

            if (c._IsUserStruct()) continue;
            sb.Append(@"
    struct " + c.Name + ";");

            if (c.Namespace != null && ((i < cs.Count - 1 && cs[i + 1].Namespace != c.Namespace) || i == cs.Count - 1)) {
                sb.Append(@"
}");
            }
        }

        sb.Append(@"
}
namespace xx {");
        foreach (var c in cs) {
            sb.GenCPP_TypeId(templateName, c);
        }
        sb.Append(@"
}
namespace " + templateName + @" {
");

    }


    static void GenH_Enums(this StringBuilder sb, List<Type> ts) {
        var es = ts._GetEnums();
        for (int i = 0; i < es.Count; ++i) {
            var e = es[i];
            if (e.Namespace != null && (i == 0 || (i > 0 && es[i - 1].Namespace != e.Namespace))) // namespace 去重
            {
                sb.Append(@"
namespace " + e.Namespace.Replace(".", "::") + @" {");
            }

            sb.Append(e._GetDesc()._GetComment_Cpp(4) + @"
    enum class " + e.Name + @" : " + e._GetEnumUnderlyingTypeName_Cpp() + @" {");

            var fs = e._GetEnumFields();
            foreach (var f in fs) {
                sb.Append(f._GetDesc()._GetComment_Cpp(8) + @"
        " + f.Name + " = " + f._GetEnumValue(e) + ",");
            }

            sb.Append(@"
    };");

            if (e.Namespace != null && ((i < es.Count - 1 && es[i + 1].Namespace != e.Namespace) || i == es.Count - 1)) {
                sb.Append(@"
}");
            }
        }
    }


    static void GenH_Structs(this StringBuilder sb, List<Type> cs, string templateName, Assembly asm) {
        for (int i = 0; i < cs.Count; ++i) {
            var c = cs[i];
            if (!c._IsUserStruct()) continue;
            var o = asm.CreateInstance(c.FullName);

            if (c.Namespace != null && (i == 0 || (i > 0 && cs[i - 1].Namespace != c.Namespace))) // namespace 去重
            {
                sb.Append(@"
namespace " + c.Namespace.Replace(".", "::") + @" {");
            }

            sb.GenH_Struct(c, templateName, o);

            if (c.Namespace != null && ((i < cs.Count - 1 && cs[i + 1].Namespace != c.Namespace) || i == cs.Count - 1)) {
                sb.Append(@"
}");
            }
        }
    }

    static void GenH_Classs(this StringBuilder sb, List<Type> cs, string templateName, Assembly asm) {
        for (int i = 0; i < cs.Count; ++i) {
            var c = cs[i];
            if (c._IsUserStruct()) continue;
            var o = asm.CreateInstance(c.FullName);

            if (c.Namespace != null && (i == 0 || (i > 0 && cs[i - 1].Namespace != c.Namespace))) // namespace 去重
            {
                sb.Append(@"
namespace " + c.Namespace.Replace(".", "::") + @" {");
            }

            sb.GenH_Struct(c, templateName, o);

            if (c.Namespace != null && ((i < cs.Count - 1 && cs[i + 1].Namespace != c.Namespace) || i == cs.Count - 1)) {
                sb.Append(@"
}");
            }
        }
    }

    static void GenH_Struct(this StringBuilder sb, Type c, string templateName, object o) {
        // 定位到基类
        var bt = c.BaseType;


        if (c._IsUserStruct()) {
            var btn = c._HasBaseType() ? (" : " + bt._GetTypeDecl_Cpp(templateName)) : "";
            sb.Append(c._GetDesc()._GetComment_Cpp(4) + @"
    struct " + c.Name + btn + @" {
        XX_GENCODE_STRUCT_H(" + c.Name + @")");
        }
        else {
            var btn = c._HasBaseType() ? bt._GetTypeDecl_Cpp(templateName) : "xx::Object";
            sb.Append(c._GetDesc()._GetComment_Cpp(4) + @"
    struct " + c.Name + " : " + btn + @" {
        XX_GENCODE_OBJECT_H(" + c.Name + @", " + btn + @")");
        }

        sb.GenH_Struct_Fields(c, templateName, o);

        if (c._Has<TemplateLibrary.Include>()) {
            sb.Append(@"
#include """ + c._GetTypeDecl_Lua(templateName) + @".inc""");
        }

        sb.Append(@"
    };");
    }

    static void GenH_Struct_Fields(this StringBuilder sb, Type c, string templateName, object o) {
        var fs = c._GetFieldsConsts();
        foreach (var f in fs) {
            var ft = f.FieldType;
            var ftn = ft._GetTypeDecl_Cpp(templateName);
            sb.Append(f._GetDesc()._GetComment_Cpp(8) + @"
        " + (f.IsStatic ? "constexpr " : "") + ftn + " " + f.Name);

            if (ft._IsExternal() && !ft._GetExternalSerializable() && !string.IsNullOrEmpty(ft._GetExternalCppDefaultValue())) {
                sb.Append(" = " + ft._GetExternalCppDefaultValue() + ";");
            }
            else {
                var v = f.GetValue(f.IsStatic ? null : o);
                var dv = ft._GetDefaultValueDecl_Cpp(v, templateName);
                if (dv != "") {
                    sb.Append(" = " + dv + ";");
                }
                else {
                    sb.Append(";");
                }
            }
        }
    }

    static void GenH_StructTemplates(this StringBuilder sb, List<Type> cs, string templateName) {
        sb.Append(@"
}
namespace xx {");

        foreach (var c in cs) {
            if (!c._IsUserStruct()) continue;
            var ctn = c._GetTypeDecl_Cpp(templateName);
            sb.Append(@"
	template<>
	struct StringFuncsEx<" + ctn + @", void> {
		static void Append(ObjectHelper &oh, " + ctn + @" const& in);
		static void AppendCore(ObjectHelper &oh, " + ctn + @" const& in);
    };
	template<>
	struct DataFuncsEx<" + ctn + @", void> {
		static void Write(DataWriterEx& dw, " + ctn + @" const& in);
		static int Read(DataReaderEx& dr, " + ctn + @"& out);
	};
    template<>
	struct CloneFuncs<" + ctn + @", void> {
		static void Clone1(ObjectHelper &oh, " + ctn + @" const& in, " + ctn + @"& out);
		static void Clone2(ObjectHelper &oh, " + ctn + @" const& in, " + ctn + @"& out);
	};");
        }

        sb.Append(@"
}");
    }


    static void GenH_Tail(this StringBuilder sb, string templateName) {
        sb.Append(@"
#include """ + templateName + @"_class_lite_.h.inc""  // user create it for extend include files at the end
");
    }


    static void GenCPP_Struct_CopyAssign(this StringBuilder sb, string templateName, Type c) {
        sb.Append(@"
    " + c.Name + @"::" + c.Name + @"(" + c.Name + @"&& o) noexcept {
        this->operator=(std::move(o));
    }
    " + c.Name + @"& " + c.Name + @"::operator=(" + c.Name + @"&& o) noexcept {");
        if (c._HasBaseType()) {
            var bt = c.BaseType;
            var btn = bt._GetTypeDecl_Cpp(templateName);
            sb.Append(@"
        this->" + (c._IsUserStruct() ? btn : "BaseType") + "::operator=(std::move(o));");
        }
        var fs = c._GetFields();
        foreach (var f in fs) {
            var ft = f.FieldType;
            sb.Append(@"
        std::swap(this->" + f.Name + ", o." + f.Name + ");");
        }
        sb.Append(@"
        return *this;
    }");

        if (c._IsUserClass()) {
            sb.Append(@"
    void " + c.Name + @"::Clone1(xx::ObjectHelper &oh, std::shared_ptr<Object> const &tar) const {");
            if (c._HasBaseType()) {
                var bt = c.BaseType;
                sb.Append(@"
        this->BaseType::Clone1(oh, tar);");
            }
            sb.Append(@"
        auto&& o = xx::As<" + c._GetTypeDecl_Cpp(templateName) + @">(tar);");
            foreach (var f in fs) {
                var ft = f.FieldType;
                if (ft._IsExternal() && !ft._GetExternalSerializable()) continue;
                if (f._Has<TemplateLibrary.Custom>()) {
                    // todo: call custom func
                    throw new NotImplementedException();
                }
                else {
                    sb.Append(@"
        xx::CloneFuncs<" + ft._GetTypeDecl_Cpp(templateName) + @">::Clone1(oh, this->" + f.Name + ", o->" + f.Name + ");");
                }
            }
            sb.Append(@"
    }");
            sb.Append(@"
    void " + c.Name + @"::Clone2(xx::ObjectHelper &oh, std::shared_ptr<Object> const &tar) const {");
            if (c._HasBaseType()) {
                var bt = c.BaseType;
                sb.Append(@"
        this->BaseType::Clone2(oh, tar);");
            }
            sb.Append(@"
        auto&& o = xx::As<" + c._GetTypeDecl_Cpp(templateName) + @">(tar);");

            foreach (var f in fs) {
                var ft = f.FieldType;
                if (ft._IsExternal() && !ft._GetExternalSerializable()) continue;
                if (f._Has<TemplateLibrary.Custom>()) {
                    // todo: call custom func
                    throw new NotImplementedException();
                }
                else {
                    sb.Append(@"
        xx::CloneFuncs<" + ft._GetTypeDecl_Cpp(templateName) + @">::Clone2(oh, this->" + f.Name + ", o->" + f.Name + ");");
                }
            }
            sb.Append(@"
    }");

            sb.Append(@"
    uint16_t " + c.Name + @"::GetTypeId() const {
        return xx::TypeId_v<" + c._GetTypeDecl_Cpp(templateName) + @">;
    }");
            sb.Append(@"
    void " + c.Name + @"::Serialize(xx::DataWriterEx& dw) const {");

            if (c._HasBaseType()) {
                var bt = c.BaseType;
                sb.Append(@"
        this->BaseType::Serialize(dw);");
            }

            foreach (var f in fs) {
                var ft = f.FieldType;
                if (ft._IsExternal() && !ft._GetExternalSerializable()) continue;
                if (f._Has<TemplateLibrary.Custom>()) {
                    // todo: call custom func
                    throw new NotImplementedException();
                }
                else {
                    sb.Append(@"
        dw.Write(this->" + f.Name + ");");
                }
            }

            sb.Append(@"
    }");

            sb.Append(@"
    int " + c.Name + @"::Deserialize(xx::DataReaderEx& dr) {");

            if (c._HasBaseType()) {
                var bt = c.BaseType;
                sb.Append(@"
        if (int r = this->BaseType::Deserialize(dr)) return r;");
            }

            foreach (var f in fs) {
                if (f.FieldType._IsExternal() && !f.FieldType._GetExternalSerializable()) continue;
                sb.Append(@"
        if (int r = dr.Read(this->" + f.Name + @")) return r;");
            }

            sb.Append(@"
        return 0;
    }");
            var ctn = c._GetTypeDecl_Cpp(templateName);
            sb.Append(@"
    void " + c.Name + @"::ToString(xx::ObjectHelper &oh) const {
        auto&& iter = oh.objOffsets.find((void*)this);
        if (iter != oh.objOffsets.end()) {
        	xx::AppendEx(oh, iter->second);
        	return;
        }
        else {
            oh.objOffsets[(void*)this] = oh.s.size();
        }
        xx::AppendEx(oh, ""{\""#\"":"", GetTypeId());
        ToStringCore(oh);
        oh.s.push_back('}');
    }
    void " + c.Name + @"::ToStringCore(xx::ObjectHelper &oh) const {");

            if (c._HasBaseType()) {
                var bt = c.BaseType;
                sb.Append(@"
        this->BaseType::ToStringCore(oh);");
            }

            foreach (var f in fs) {
                var ft = f.FieldType;
                if (ft._IsExternal() && !ft._GetExternalSerializable()) continue;
                sb.Append(@"
        xx::AppendEx(oh, "",\""" + f.Name + @"\"":"", this->" + f.Name + @");");
            }
            sb.Append(@"
    }");

        }
    }

    static void GenCPP_Serialize(this StringBuilder sb, string templateName, Type c) {
        if (c._IsUserStruct()) {
            var ctn = c._GetTypeDecl_Cpp(templateName);
            var fs = c._GetFields();

            sb.Append(@"
    void CloneFuncs<" + ctn + @">::Clone1(xx::ObjectHelper &oh, " + ctn + @" const& in, " + ctn + @" &out) {");
            if (c._HasBaseType()) {
                var bt = c.BaseType;
                var btn = bt._GetTypeDecl_Cpp(templateName);
                sb.Append(@"
        CloneFuncs<" + btn + ">::Clone1(oh, in, out);");
            }
            foreach (var f in fs) {
                var ft = f.FieldType;
                if (ft._IsExternal() && !ft._GetExternalSerializable()) continue;
                if (f._Has<TemplateLibrary.Custom>()) {
                    // todo: call custom func
                    throw new NotImplementedException();
                }
                else {
                    sb.Append(@"
        CloneFuncs<" + ft._GetTypeDecl_Cpp(templateName) + @">::Clone1(oh, in." + f.Name + ", out." + f.Name + ");");
                }
            }
            sb.Append(@"
    }");
            sb.Append(@"
    void CloneFuncs<" + ctn + @">::Clone2(xx::ObjectHelper &oh, " + ctn + @" const& in, " + ctn + @" &out) {");
            if (c._HasBaseType()) {
                var bt = c.BaseType;
                var btn = bt._GetTypeDecl_Cpp(templateName);
                sb.Append(@"
        CloneFuncs<" + btn + ">::Clone2(oh, in, out);");
            }
            foreach (var f in fs) {
                var ft = f.FieldType;
                if (ft._IsExternal() && !ft._GetExternalSerializable()) continue;
                if (f._Has<TemplateLibrary.Custom>()) {
                    // todo: call custom func
                    throw new NotImplementedException();
                }
                else {
                    sb.Append(@"
        CloneFuncs<" + ft._GetTypeDecl_Cpp(templateName) + @">::Clone2(oh, in." + f.Name + ", out." + f.Name + ");");
                }
            }
            sb.Append(@"
    }");


            sb.Append(@"
	void DataFuncsEx<" + ctn + @", void>::Write(DataWriterEx& dw, " + ctn + @" const& in) {");

            if (c._HasBaseType()) {
                var bt = c.BaseType;
                var btn = bt._GetTypeDecl_Cpp(templateName);
                sb.Append(@"
        DataFuncsEx<" + btn + ">::Write(dw, in);");
            }

            foreach (var f in fs) {
                var ft = f.FieldType;
                if (ft._IsExternal() && !ft._GetExternalSerializable()) continue;
                if (f._Has<TemplateLibrary.Custom>()) {
                    // todo
                    throw new NotImplementedException();
                }
                else {
                    sb.Append(@"
        dw.Write(in." + f.Name + ");");
                }
            }

            sb.Append(@"
    }");
        }
        else {
            // todo object funcs
        }
    }

    static void GenCPP_Deserialize(this StringBuilder sb, string templateName, Type c) {
        if (!c._IsUserStruct()) return;
        var ctn = c._GetTypeDecl_Cpp(templateName);
        sb.Append(@"
	int DataFuncsEx<" + ctn + @", void>::Read(DataReaderEx& d, " + ctn + @"& out) {");

        if (c._HasBaseType()) {
            var bt = c.BaseType;
            var btn = bt._GetTypeDecl_Cpp(templateName);

            sb.Append(@"
        if (int r = DataFuncsEx<" + btn + ">::Read(d, out)) return r;");
        }

        var fs = c._GetFields();
        foreach (var f in fs) {
            if (f.FieldType._IsExternal() && !f.FieldType._GetExternalSerializable()) continue;
            sb.Append(@"
        if (int r = d.Read(out." + f.Name + @")) return r;");
        }

        sb.Append(@"
        return 0;
    }");
    }

    static void GenCPP_ToString(this StringBuilder sb, string templateName, Type c) {
        if (!c._IsUserStruct()) return;
        var ctn = c._GetTypeDecl_Cpp(templateName);
        sb.Append(@"
	void StringFuncsEx<" + ctn + @", void>::Append(ObjectHelper &oh, " + ctn + @" const& in) {
        oh.s.push_back('{');
        AppendCore(oh, in);
        oh.s.push_back('}');
    }
	void StringFuncsEx<" + ctn + @", void>::AppendCore(ObjectHelper &oh, " + ctn + @" const& in) {
        auto sizeBak = oh.s.size();");
        if (c._HasBaseType()) {
            var bt = c.BaseType;
            var btn = bt._GetTypeDecl_Cpp(templateName);
            sb.Append(@"
        StringFuncsEx<" + btn + ">::AppendCore(oh, in);");
        }

        var fs = c._GetFields();
        foreach (var f in fs) {
            var ft = f.FieldType;
            if (ft._IsExternal() && !ft._GetExternalSerializable()) continue;
            if (f == fs[0]) {
                sb.Append(@"
        if (sizeBak == oh.s.size()) {
            oh.s.push_back(',');
        }
        xx::AppendEx(oh, ""\""" + f.Name + @"\"":"", in." + f.Name + @"); ");
            }
            else {
                sb.Append(@"
        xx::AppendEx(oh, "",\""" + f.Name + @"\"":"", in." + f.Name + @");");
            }
        }
        sb.Append(@"
    }");
    }

    static void GenCPP_TypeId(this StringBuilder sb, string templateName, Type c) {
        if (c._IsUserStruct()) return;
        var ctn = c._GetTypeDecl_Cpp(templateName);
        var typeId = c._GetTypeId();
        if (typeId.HasValue) {
            sb.Append(@"
    template<> struct TypeId<" + ctn + @"> { static const uint16_t value = " + typeId + @"; };");
        }
    }

    static void GenCPP_Register(this StringBuilder sb, string templateName) {
        sb.Append(@"
namespace " + templateName + @" {
	void PkgGenTypes::RegisterTo(xx::ObjectHelper& oh) {");
        foreach (var kv in typeIdMappings) {
            var ctn = kv.Value._GetTypeDecl_Cpp(templateName);
            if (!kv.Value._Has<TemplateLibrary.Virtual>()) {
                sb.Append(@"
	    oh.Register<" + ctn + @">();");
            }
        }
        sb.Append(@"
	}
}
");
    }

    static void GenCPP_Includes(this StringBuilder sb, string templateName) {
        sb.Append("#include \"" + templateName + @"_class_lite.h""
#include """ + templateName + @"_class_lite.cpp.inc""");
    }

    static void GenCPP_FuncImpls(this StringBuilder sb, string templateName, List<Type> cs) {
        GenCPP_Register(sb, templateName);

        sb.Append(@"
namespace xx {");
        foreach (var c in cs) {
            sb.GenCPP_Serialize(templateName, c);
            sb.GenCPP_Deserialize(templateName, c);
            sb.GenCPP_ToString(templateName, c);
        }
        sb.Append(@"
}");

        sb.Append(@"
namespace " + templateName + @" {");

        for (int i = 0; i < cs.Count; ++i) {
            var c = cs[i];

            // namespace c_ns {
            if (c.Namespace != null && (i == 0 || (i > 0 && cs[i - 1].Namespace != c.Namespace))) // namespace 去重
            {
                sb.Append(@"
namespace " + c.Namespace.Replace(".", "::") + @" {");
            }

            sb.GenCPP_Struct_CopyAssign(templateName, c);

            // namespace }
            if (c.Namespace != null && ((i < cs.Count - 1 && cs[i + 1].Namespace != c.Namespace) || i == cs.Count - 1)) {
                sb.Append(@"
}");
            }
        }

        sb.Append(@"
}
");    // namespace templateName

    }



    static void GenH_AJSON(this StringBuilder sb, Type c) {
        if (c._HasBaseType()) {
            GenH_AJSON(sb, c.BaseType);
        }
        var fs = c._GetFields();
        foreach (var f in fs) {
            sb.Append(", " + f.Name);
        }
    }

    static void GenH_AJSON(this StringBuilder sb, string templateName, List<Type> cs) {
        sb.Append(@"#pragma once
#include """ + templateName + @"_class_lite.h.inc""
#include ""ajson.hpp""");
        foreach (var c in cs) {
            if (!c._IsUserStruct()) continue;
            sb.Append(@"
AJSON(" + templateName + "::" + c.Name);
            GenH_AJSON(sb, c);
            sb.Append(");");
        }
        sb.Append(@"
");
    }

    public static void Gen(Assembly asm, string outDir, string templateName) {
        var ts = asm._GetTypes();

        // 填充 typeId for class
        foreach (var c in ts._GetClasss().Where(o => !o._Has<TemplateLibrary.Struct>())) {
            var id = c._GetTypeId();
            if (id == null) { }// throw new Exception("type: " + c.FullName + " miss [TypeId(xxxxxx)]");
            else {
                if (typeIdMappings.ContainsKey(id.Value)) {
                    throw new Exception("type: " + c.FullName + "'s typeId is duplicated with " + typeIdMappings[id.Value].FullName);
                }
                else {
                    typeIdMappings.Add(id.Value, c);
                }
            }
        }

        var sb = new StringBuilder();
        var cs = ts._GetClasssStructs();
        cs._SortByInheritRelation();
        // todo: 要将 class 的 members 引用到的 struct 排到前面去

        // 生成 .h
        sb.GenH_Head(templateName);
        sb.GenH_ClassPredefine(cs, templateName, asm);
        sb.GenH_Enums(ts);
        sb.GenH_Structs(cs, templateName, asm);
        sb.GenH_Classs(cs, templateName, asm);
        sb.GenH_StructTemplates(cs, templateName);
        sb.GenH_Tail(templateName);
        sb._WriteToFile(Path.Combine(outDir, templateName + "_class_lite.h"));
        sb.Clear();
        sb.GenCPP_Includes(templateName);
        sb.GenCPP_FuncImpls(templateName, cs);
        sb._WriteToFile(Path.Combine(outDir, templateName + "_class_lite.cpp"));
        sb.Clear();
        sb.GenH_AJSON(templateName, cs);
        sb._WriteToFile(Path.Combine(outDir, templateName + "_class_lite.ajson.h"));
    }
}
