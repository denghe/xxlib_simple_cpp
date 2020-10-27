using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using System;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Loader;
using System.Linq;
using System.Collections.Generic;

public static class Program {
    public static Assembly CreateAssembly(IEnumerable<string> fileNames) {
        var dllPath = RuntimeEnvironment.GetRuntimeDirectory();

        var compilation = CSharpCompilation.Create("TmpLib")
            .WithOptions(new CSharpCompilationOptions(OutputKind.DynamicallyLinkedLibrary))
            .AddReferences(MetadataReference.CreateFromFile(typeof(object).GetTypeInfo().Assembly.Location)
                , MetadataReference.CreateFromFile(typeof(Console).GetTypeInfo().Assembly.Location)
                , MetadataReference.CreateFromFile(typeof(TypeHelpers).GetTypeInfo().Assembly.Location)
                , MetadataReference.CreateFromFile(Path.Combine(dllPath, "mscorlib.dll"))
                , MetadataReference.CreateFromFile(Path.Combine(dllPath, "netstandard.dll"))
                , MetadataReference.CreateFromFile(Path.Combine(dllPath, "System.Runtime.dll")))
            .AddSyntaxTrees(from fn in fileNames select CSharpSyntaxTree.ParseText(File.ReadAllText(fn)));

        foreach (var d in compilation.GetDiagnostics()) {
            Console.WriteLine(d);
        }
        using (var m = new MemoryStream()) {
            var r = compilation.Emit(m);
            if (r.Success) {
                m.Seek(0, SeekOrigin.Begin);
                return AssemblyLoadContext.Default.LoadFromStream(m);
            }
            return null;
        }
    }

    public static void TipsAndExit(string msg, int exitCode = 0) {
        Console.WriteLine(msg);
        Console.WriteLine("按任意键退出");
        Console.ReadKey();
        Environment.Exit(exitCode);
    }

    public static void Main(string[] args) {
        if (args.Length < 3) {
            TipsAndExit(@"
*.cs -> codes 生成器使用提示：

缺参数：
    根命名空间,    输出目录,    源码文件清单( 完整路径, 可多个 )

输出目录:
    如果缺失，默认为 工作目录. 必须已存在, 不会帮忙创建

源码文件或目录清单:
    如果缺失，默认为 工作目录下 *.cs
");
        }

        var rootNamespace = args[0];

        var outPath = ".";
        var inPaths = new List<string>();

        if (args.Length > 1) {
            if (!Directory.Exists(args[1])) {
                TipsAndExit("参数2 错误：目录不存在");
            }
            outPath = args[1];
        }

        var fileNames = new HashSet<string>();
        for (int i = 2; i < args.Length; ++i) {
            if (!File.Exists(args[i])) {
                TipsAndExit("参数 " + i + 1 + " 错误：文件不存在: " + args[i]);
            }
            else {
                fileNames.Add(args[i]);
            }
        }

        var asm = CreateAssembly(fileNames);
        if (asm == null) return;

        Console.WriteLine("开始生成");
        try {
            GenCPP_Class_Lite.Gen(asm, outPath, rootNamespace);
        }
        catch (Exception ex) {
            TipsAndExit("生成失败: " + ex.Message + "\r\n" + ex.StackTrace);
        }
        TipsAndExit("生成完毕");


        //var fn = Path.Combine(Environment.CurrentDirectory, "Program.cs");
        //var asm = GetAssembly(new string[] { fn });
        //if (asm != null) {
        //    asm.GetType("Program").GetMethod("Test").Invoke(null, null);
        //}
        //else Console.ReadLine();
    }
}
