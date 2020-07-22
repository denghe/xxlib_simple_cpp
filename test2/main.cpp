#include "xx_object.h"
#include "PKG_class_lite.h"
int main() {
    // 创建类辅助器
    xx::ObjectHelper oh;

    // 注册类型
    PKG::PkgGenTypes::RegisterTo(oh);

    xx::Data data;
    {
        // 构建一个场景
        auto&& scene = std::make_shared<PKG::Scene>();
        auto&& node1 = std::make_shared<PKG::Node>();
        scene->childs.push_back(node1);
        node1->parent = scene;

        auto&& node1_1 = std::make_shared<PKG::Node>();
        node1->childs.push_back(node1_1);
        node1_1->parent = node1;

        auto&& node1_2 = std::make_shared<PKG::Node>();
        node1->childs.push_back(node1_2);
        node1_2->parent = node1;

        scene->nodes["1"] = node1;
        scene->nodes["1_1"] = node1_1;
        scene->nodes["1_2"] = node1_2;

        // 序列化进 data
        oh.WriteTo(data, scene);
        // 打印 data 的内容
        oh.CoutN(data);

        // 测试下克隆
        auto&& scene2 = oh.Clone(scene);
        // 比较数据是否相同。相同则篡改下
        if (!oh.Compare(scene, scene2)) {
            // 故意改点东西
            scene2->parent = scene2;
            // 如果比较结果不一致则输出
            if (oh.Compare(scene, scene2)) {
                oh.CoutCompareResult();
            }
        }
    }

    oh.CoutN(data);
    {
        oh.CoutN(oh.ReadObjectFrom(data));
    }

    data.Clear();
    {
        auto&& d = std::make_shared<PKG::D>();
        d->name = "d";
        d->desc = "nullable";
        d->a.x = 1;
        d->a.y = 2;
        d->a.c = d;
        d->b.x = 3;
        d->b.y = 4;
        d->b.z = 5;
        d->b.c = d;
        d->b.wc = d;

        oh.WriteTo(data, d);
    }
    oh.CoutN(data);
    {
        oh.CoutN(oh.ReadObjectFrom(data));
    }
}
