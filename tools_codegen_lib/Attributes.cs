// 全是 Attribute
namespace TemplateLibrary {
    /// <summary>
    /// 对应 c++ std::optional, c# Nullable<>, lua variable
    /// </summary>
    public class Nullable<T> { }

    /// <summary>
    /// 对应 c++ std::vector, c# List, lua table
    /// </summary>
    public class List<T> { }

    /// <summary>
    /// 标记一个类不继承自 xx::Object
    /// </summary>
    [System.AttributeUsage(System.AttributeTargets.Class)]
    public class Struct : System.Attribute { }

    /// <summary>
    /// 对应 c++ std::weak_ptr, c# Weak/Property, lua ???
    /// </summary>
    public class Weak<T> { }

    /// <summary>
    /// 对应 c++ std::shared_ptr, c# Shared/Property, lua ???
    /// </summary>
    public class Shared<T> { }

    /// <summary>
    /// 对应 c++ std::map, c# 暂定对应 Dict( 应该对应有序版字典 ). lua table ???
    /// </summary>
    public class Dict<K, V> { }

    /// <summary>
    /// 对应 c++ std::pair, c# 暂定对应 Dict( 应该对应有序版字典 ). lua table ???
    /// </summary>
    public class Pair<K, V> { }

    // todo: tuple ?


    /// <summary>
    /// 备注。可用于类/枚举/函数 及其 成员
    /// </summary>
    public class Desc : System.Attribute {
        public Desc(string v) { value = v; }
        public string value;
    }


    /// <summary>
    /// 外部扩展。命名空间根据类所在实际命名空间获取，去除根模板名。参数如果传 false 则表示该类不支持序列化，无法用于收发
    /// </summary>
    [System.AttributeUsage(System.AttributeTargets.Enum | System.AttributeTargets.Class | System.AttributeTargets.Struct)]
    public class External : System.Attribute {
        public External(bool serializable = true
            , string cppDefaultValue = "nullptr"
            , string csharpDefaultValue = "null"
            , string luaDefaultValue = "null") {
            this.serializable = serializable;
            this.cppDefaultValue = cppDefaultValue;
            this.csharpDefaultValue = csharpDefaultValue;
            this.luaDefaultValue = luaDefaultValue;
        }
        public bool serializable;
        public string cppDefaultValue;
        public string csharpDefaultValue;
        public string luaDefaultValue;
    }


    /// <summary>
    /// 针对最外层级的 List, Data, string 做最大长度保护限制
    /// 如果是类似 List List... 多层需要限制的情况, 就写多个 Limit, 有几层写几个
    /// </summary>
    [System.AttributeUsage(System.AttributeTargets.Field | System.AttributeTargets.ReturnValue, AllowMultiple = true)]
    public class Limit : System.Attribute {
        public Limit(int value) {
            this.value = value;
        }
        public int value;
    }




    /// <summary>
    /// 标记一个类含有 virtual xxxxxx = 0 的纯虚函数, 这将导致不会生成 Register TypeId 代码  ( cpp only )
    /// </summary>
    [System.AttributeUsage(System.AttributeTargets.Class | System.AttributeTargets.Struct)]
    public class Virtual : System.Attribute {
    }

    /// <summary>
    /// 标记一个类需要抠洞在声明和实现部分分别嵌入 namespace_classname.h , .hpp ( cpp only )
    /// </summary>
    [System.AttributeUsage(System.AttributeTargets.Class | System.AttributeTargets.Struct)]
    public class Include : System.Attribute {
    }

    /// <summary>
    /// 标记一个类成员通过调用 用户于生成物之外提供的手写版函数实现序列化
    /// </summary>
    [System.AttributeUsage(System.AttributeTargets.Field)]
    public class Custom : System.Attribute {
        // 传入 读 和 写 的成员函数名. 函数带1个参数: Serializer/Deserializer. 其实现为自定义读写
        public Custom(string rf, string wf) { this.rf = rf; this.wf = wf; }
        public string rf, wf;
        public override string ToString() {
            throw new System.Exception("please get value from rf | wf");
        }
    }

    /************************************************************************************************/
    // 下面是 生成过滤 相关
    /************************************************************************************************/

    /// <summary>
    /// Lua 生成物之命名空间过滤( 白名单 ), 用到 生成配置 接口上
    /// </summary>
    [System.AttributeUsage(System.AttributeTargets.Interface, AllowMultiple = true)]
    public class LuaFilter : System.Attribute {
        public LuaFilter(string v) { value = v; }
        public string value;
        public override string ToString() {
            return value;
        }
    }

    /// <summary>
    /// CPP client 生成物之命名空间过滤( 白名单 ), 用到 生成配置 接口上
    /// </summary>
    [System.AttributeUsage(System.AttributeTargets.Interface, AllowMultiple = true)]
    public class CppFilter : System.Attribute {
        public CppFilter(string v) { value = v; }
        public string value;
        public override string ToString() {
            return value;
        }
    }

    /// <summary>
    /// C# client 生成物之命名空间过滤( 白名单 ), 用到 生成配置 接口上
    /// </summary>
    [System.AttributeUsage(System.AttributeTargets.Interface, AllowMultiple = true)]
    public class CSharpFilter : System.Attribute {
        public CSharpFilter(string v) { value = v; }
        public string value;
        public override string ToString() {
            return value;
        }
    }

    // todo: more filter here

    /************************************************************************************************/
    // 下面是 type id 相关
    /************************************************************************************************/

    /// <summary>
    /// 用来做类型到 typeId 的固定映射生成
    /// </summary>
    [System.AttributeUsage(System.AttributeTargets.Class | System.AttributeTargets.Struct)]
    public class TypeId : System.Attribute {
        public TypeId(ushort value) {
            this.value = value;
        }
        public ushort value;
    }

    /// <summary>
    /// 用来做类型到 typeId 的固定映射生成. 对应的 interface 的成员长相为  Type _id { get; }
    /// </summary>
    [System.AttributeUsage(System.AttributeTargets.Interface)]
    public class TypeIdMappings : System.Attribute {
    }


    /// <summary>
    /// TypeId生成配置范围, [s, e)
    /// </summary>
    [System.AttributeUsage(System.AttributeTargets.Interface, AllowMultiple = true)]
    public class TypeIdRange : System.Attribute {
        public TypeIdRange(ushort s, ushort e) { this.s = s; this.e = e; }
        public ushort s;
        public ushort e;
        public override string ToString() {
            return string.Format("[{0}, {1})", s, e);
        }
    }


    /************************************************************************************************/
    // 下面是数据库相关
    /************************************************************************************************/

    /// <summary>
    /// 标记一个类成员是一个数据库字段( 填充时仅考虑其出现的顺序而不管其名称 )
    /// </summary>
    [System.AttributeUsage(System.AttributeTargets.Field)]
    public class Column : System.Attribute {
        public bool readOnly;
        public Column(bool readOnly = false) {
            this.readOnly = readOnly;
        }
    }

    /// <summary>
    /// 标记一个类成员在数据库中不可空( 主要是指 String, Data 这种指针类型 )
    /// </summary>
    [System.AttributeUsage(System.AttributeTargets.Field)]
    public class NotNull : System.Attribute {
    }

    /// <summary>
    /// 标记一个类成员以 "Property" 方式生成( 例如 int xxxx { get; set; } ( c# only )
    /// </summary>
    [System.AttributeUsage(System.AttributeTargets.Field)]
    public class Property : System.Attribute {
    }

    /// <summary>
    /// 标记一个类成员是在生成的时候跳过某些readOnly字段
    /// </summary>
    [System.AttributeUsage(System.AttributeTargets.Parameter)]
    public class SkipReadOnly : System.Attribute {
    }

    /// <summary>
    /// 标记 interface 对应 mysql 的连接对象
    /// </summary>
    [System.AttributeUsage(System.AttributeTargets.Interface)]
    public class MySql : System.Attribute {
    }

    /// <summary>
    /// 标记 interface 对应 microsoft sql server 的连接对象
    /// </summary>
    [System.AttributeUsage(System.AttributeTargets.Interface)]
    public class MsSql : System.Attribute {
    }

    /// <summary>
    /// 标记 interface 对应 sqlite 的连接对象
    /// </summary>
    [System.AttributeUsage(System.AttributeTargets.Interface)]
    public class SQLite : System.Attribute {
    }

    /// <summary>
    /// 标记 TSQL 语句的内容. 用于参数时表示 "不转义" 的字串直接拼接
    /// </summary>
    [System.AttributeUsage(System.AttributeTargets.Method)]
    public class Sql : System.Attribute {
        public string value;
        public Sql(string v) { value = v; }
    }

    /// <summary>
    /// 标记函数参数为不转义的字符串, 或是不纳入"传参" 的部分
    /// </summary>
    [System.AttributeUsage(System.AttributeTargets.Parameter)]
    public class Literal : System.Attribute {
    }

    /// <summary>
    /// 对应一次返回多个 SELECT 的查询结果
    /// </summary>
    public class Tuple<T1, T2> { }
    public class Tuple<T1, T2, T3> { }
    public class Tuple<T1, T2, T3, T4> { }
    public class Tuple<T1, T2, T3, T4, T5> { }
    public class Tuple<T1, T2, T3, T4, T5, T6> { }
    public class Tuple<T1, T2, T3, T4, T5, T6, T7> { }

}













///// <summary>
///// 标记一个函数参数为 const& ( cpp only )
///// </summary>
//[System.AttributeUsage(System.AttributeTargets.Parameter)]
//public class ConstRef : System.Attribute
//{
//}

///// <summary>
///// 标记一个函数参数为 * const& ( cpp only )
///// </summary>
//[System.AttributeUsage(System.AttributeTargets.Parameter)]
//public class PointerConstRef : System.Attribute
//{
//}


///// <summary>
///// 模拟 System.DateTime
///// </summary>
//public class DateTime { }

///// <summary>
///// 标记一个枚举的内容用于 生成时的 项目分类( 与项目对应的 根命名空间 白名单 ), 以便做分类转发之类的功能
///// 该标记最多只可以出现一次
///// </summary>
//[System.AttributeUsage(System.AttributeTargets.Enum)]
//public class CategoryNamespaces : System.Attribute
//{
//}


///// <summary>
///// 用于导出为 excel 时的列名生成
///// </summary>
//public class Title : System.Attribute
//{
//    public Title(string v) { value = v; }
//    public string value;
//}




///// <summary>
///// 标记一个类为 "多主键" 类. 需要进一步标记类成员的 "主" "副" key
///// </summary>
//[System.AttributeUsage(System.AttributeTargets.Class | System.AttributeTargets.Struct)]
//public class MultiKeyDict : System.Attribute
//{
//}

///// <summary>
///// 标记一个 "多主键" 类的成员, 是否为 key. 传入 true 的为主 key.
///// </summary>
//[System.AttributeUsage(System.AttributeTargets.Field)]
//public class Key : System.Attribute
//{
//    public bool value;
//    public Key(bool value = false)
//    {
//        this.value = value;
//    }
//}


///// <summary>
///// 标记一个类成员不参与序列化
///// </summary>
//[System.AttributeUsage(System.AttributeTargets.Field)]
//public class NotSerialize : System.Attribute
//{
//}



///// <summary>
///// 标记当反列化 float / double 时, 如果值是 nan( not a number ), 就设成 v
///// </summary>
//[System.AttributeUsage(System.AttributeTargets.Field)]
//public class Nan : System.Attribute
//{
//    public string value;
//    public Nan(float v)
//    {
//        if (float.IsNaN(v) || float.IsInfinity(v))
//        {
//            throw new System.Exception("不恰当的 NaN 标记值");
//        }
//        value = v.ToString() + "f";
//    }
//    public Nan(double v)
//    {
//        if (double.IsNaN(v) || double.IsInfinity(v))
//        {
//            throw new System.Exception("不恰当的 NaN 标记值");
//        }
//        value = v.ToString();
//    }
//    public Nan()
//    {
//        value = "";
//    }
//}

///// <summary>
///// 标记当反列化 float / double 时, 如果值是 infinity, 就设成 v
///// </summary>
//[System.AttributeUsage(System.AttributeTargets.Field)]
//public class Infinity : System.Attribute
//{
//    public string value;
//    public Infinity(float v)
//    {
//        if (float.IsNaN(v) || float.IsInfinity(v))
//        {
//            throw new System.Exception("不恰当的 Infinity 标记值");
//        }
//        value = v.ToString() + "f";
//    }
//    public Infinity(double v)
//    {
//        if (double.IsNaN(v) || double.IsInfinity(v))
//        {
//            throw new System.Exception("不恰当的 Infinity 标记值");
//        }
//        value = v.ToString();
//    }
//    public Infinity()
//    {
//        value = "";
//    }
//}