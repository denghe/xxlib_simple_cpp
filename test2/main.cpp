#include "xx_object.h"
#include "PKG_class_lite.h"
int main() {
    xx::ObjectHelper oh;
    PKG::PkgGenTypes::RegisterTo(oh);
    xx::Data data;
    {
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

        oh.WriteTo(data, scene);

        std::shared_ptr<PKG::Scene> scene2;
        int r = oh.Clone(scene, scene2);
        assert(!r);
        auto i = oh.EqualsTo(scene, scene2);
        assert(i == -1);
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
