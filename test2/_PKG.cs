#pragma warning disable 0169, 0414
using TemplateLibrary;

[Struct]
class A {
    int x, y;
    Shared<C> c;
}

[Struct]
class B : A {
    int z;
    Weak<C> wc;
}

[TypeId(12)]
class C {
    A a;
    B b;
}

[TypeId(34)]
class D : C {
    string name;
}

[TypeId(56)]
class Scene : Node {
}

[TypeId(78)]
class Node {
    Weak<Node> parent;
    List<Shared<Node>> childs;
}
