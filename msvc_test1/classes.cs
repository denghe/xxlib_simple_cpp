using TemplateLibrary;
[Include, TypeId(1)]
class A {
    int id;
	Weak<A> parent;
	List<Shared<A>> children;
};
