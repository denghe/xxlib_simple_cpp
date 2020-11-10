using TemplateLibrary;

[Include, TypeId(1)]
class A {
    int id;
	Nullable<string> nick;
	Weak<A> parent;
	List<Shared<A>> children;
};

[Include, TypeId(2)]
class B : A {
	byte[] data;
	C c;
	Nullable<C> c2;
};

struct C  {
	float x, y;
	Weak<A> target;
};
