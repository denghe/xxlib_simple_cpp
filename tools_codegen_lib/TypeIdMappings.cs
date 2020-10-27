using System;
using System.Collections.Generic;
using System.Reflection;

namespace TemplateLibrary
{
    public class TypeIds
    {
        /// <summary>
        /// 存的是 type id 映射接口中填的值
        /// </summary>
        public Dictionary<Type, ushort> dict = new Dictionary<Type, ushort>();

        /// <summary>
        /// 自增 typeId. 初始化完毕后该值为 typeId 最大值
        /// </summary>
        public ushort currTypeId = 0;

        /// <summary>
        /// 是否存在映射接口
        /// </summary>
        public bool typeIdMappingsExists = false;

        /// <summary>
        /// 映射接口并未完整的包含所有数据类型
        /// </summary>
        public bool hasNewMappings = false;

        /// <summary>
        /// 存的是从所有类中扫出来的相关 type
        /// </summary>
        public Dictionary<Type, ushort> types = new Dictionary<Type, ushort>();

        private ushort[] typeIdRanges = null;

        public static ushort[] GetTypeIdRangeInfo(Assembly asm)
        {
            var idxs = new System.Collections.Generic.List<ushort>();

            var types = asm._GetTypes();
            var ifaces = types._GetInterfaces<TypeIdRange>();
            foreach (var iface in ifaces)
            {
                var ranges = iface.GetCustomAttributes<TypeIdRange>();
                foreach (var range in ranges)
                {
                    //int a = 3;
                    idxs.Add(range.s);
                    idxs.Add(range.e);
                }
            }


            return idxs.ToArray();
        }

        public TypeIds(Assembly asm)
        {
            typeIdRanges = GetTypeIdRangeInfo(asm);
            if (typeIdRanges.Length > 0)
            {
                currTypeId = typeIdRanges[0];
            }

            // 读接口中配置的映射id
            var ts = asm._GetTypes();
            var ifaces = ts._GetInterfaces<TemplateLibrary.TypeIdMappings>();
            foreach (var iface in ifaces)
            {
                var ps = iface._GetProperties();
                foreach (var p in ps)
                {
                    typeIdMappingsExists = true;
                    var t = p.PropertyType;
                    var id = ushort.Parse(p.Name.Substring(1));
                    if (dict.ContainsValue(id))
                    {
                        throw new Exception("typeid 映射接口中的 typeid: " + id + " 已存在.");
                    }
                    if (dict.ContainsKey(t))
                    {
                        throw new Exception("typeid 映射接口中的 type: " + t + " 已存在.");
                    }
                    dict.Add(t, id);
                    if (id > currTypeId) currTypeId = id;
                }
            }

            // 遍历实际的类型, 逐个映射id
            var cs = ts._GetClasss();
            foreach (var c in cs)
            {
                if (!types.ContainsKey(c)) types.Add(c, GetTypeId(c));

                if (c._HasBaseType())
                {
                    var bt = c.BaseType;
                    if (!types.ContainsKey(bt)) types.Add(bt, GetTypeId(bt));
                }

                var fs = c._GetFields();
                foreach (var f in fs)
                {
                    var ft = f.FieldType;
                    //if (ft._IsUserClass() || ft._IsList())
                    if (ft._IsUserClass())
                    {
                        if (!types.ContainsKey(ft)) types.Add(ft, GetTypeId(ft));
                    }

                    // 递归添加 List 的子类型
                    //LabBegin:
                    //while (ft._IsList())
                    //{
                    //    ft = ft.GenericTypeArguments[0];
                    //    if (ft._IsUserClass() || ft._IsList())
                    //    {
                    //        if (!types.ContainsKey(ft)) types.Add(ft, GetTypeId(ft));
                    //    }
                    //    goto LabBegin;
                    //}
                }
            }
        }

        public ushort GetTypeId(Type t)
        {
            if (dict.ContainsKey(t)) return dict[t];
            hasNewMappings = true;
            if (typeIdRanges.Length > 0 && currTypeId >= typeIdRanges._Last())
            {
                throw new Exception("typeId已经耗尽，请检查配置.");
            }

            dict.Add(t, currTypeId);
            ++currTypeId;

            int len = typeIdRanges.Length >> 1;
            for (int i = 0; i < len; ++i)
            {
                if (currTypeId == typeIdRanges[(i << 1) + 1])
                {
                    if (i == len - 1)
                    {
                        throw new Exception("typeId已经耗尽，请检查配置.");
                    }
                    else
                    {
                        currTypeId = typeIdRanges[(i + 1) << 1];
                    }
                    break;
                }
            }

            return currTypeId;
        }
    }
}
