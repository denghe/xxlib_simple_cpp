#include "xx_object.h"
#include "PKG_class_lite.h"
int main() {
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

        xx::DataWriterEx dw(data);
        dw.WriteOnce(scene);
    }
    xx::CoutN(data);
    {
        xx::ObjectCreators oc;
        PKG::PkgGenTypes::RegisterTo(oc);
        xx::DataReaderEx dr(data, oc);

        std::shared_ptr<xx::Object> o;
        int r = dr.ReadOnce(o);
        assert(!r);
        xx::CoutN(o);
    }
    data.Clear();
    {
        auto&& d = std::make_shared<PKG::D>();
        d->name = "d";
        d->a.x = 1;
        d->a.y = 1;
        d->a.c = d;
        d->b.x = 3;
        d->b.y = 4;
        d->b.z = 5;
        d->b.c = d;
        d->b.wc = d;

        xx::DataWriterEx dw(data);
        dw.WriteOnce(d);
    }
    xx::CoutN(data);
    {
        xx::ObjectCreators oc;
        PKG::PkgGenTypes::RegisterTo(oc);
        xx::DataReaderEx dr(data, oc);

        std::shared_ptr<xx::Object> o;
        int r = dr.ReadOnce(o);
        assert(!r);
        xx::CoutN(o);
    }
}
