#pragma warning disable 0169, 0414
using TemplateLibrary;

[Include, Desc("")]
class FishBase {
    int n;
};

[TypeId(1), Include, Desc("")]
class CppFish : FishBase {
};

[TypeId(2), Virtual, Desc("")]
class LuaFishBase : FishBase {
    string fileName;
};
