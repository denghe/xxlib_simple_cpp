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

    /// <summary>
    /// 对应 c++ std::tuple, c# 暂定对应 Dict( 应该对应有序版字典 ). lua table ???
    /// </summary>
    public class Tuple<T1> { }
    public class Tuple<T1, T2> { }
    public class Tuple<T1, T2, T3> { }
    public class Tuple<T1, T2, T3, T4> { }
    public class Tuple<T1, T2, T3, T4, T5> { }
    public class Tuple<T1, T2, T3, T4, T5, T6> { }
    public class Tuple<T1, T2, T3, T4, T5, T6, T7> { }


    /// <summary>
    /// 标记一个类能简单向下兼容( 不可修改顺序和数据类型和默认值, 变量名可改, 不可删除，只能在最后追加, 且须避免老 Weak 引用到追加成员. 老 Shared 引用到新增类 也是无法识别的 )
    /// </summary>
    [System.AttributeUsage(System.AttributeTargets.Class | System.AttributeTargets.Struct)]
    public class Compatible : System.Attribute { 
        // todo: 多套向下兼容方案?
    }

    /// <summary>
    /// 标记一个类不继承自 xx::Object
    /// </summary>
    [System.AttributeUsage(System.AttributeTargets.Class)]
    public class Struct : System.Attribute { }


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
    /// 标记一个类需要抠洞在声明和实现部分分别嵌入 namespace_classname.h , .hpp ( cpp only )
    /// </summary>
    [System.AttributeUsage(System.AttributeTargets.Class | System.AttributeTargets.Struct)]
    public class Include : System.Attribute {
    }

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

}
