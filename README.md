# 针对 linux 服务器 cpp 单线程主体 开发的 .h 库
库代码全是 .h 文件，位于 xxlib 目录（其中也复制了一些常用第三方库在内）  
别的目录是示例  


# 开发环境配置教程
## ubuntu 20.04 LTS server
可狂选 [Done] [Continue],  记住拿到的 ip 地址, 空格勾 Install OpenSSH server  
最后显示 Installation complete! 时 要是下载太慢 可取消更新下载 重启后用代理啥的再说  
最后移除光盘 / U盘( 可以用 rufus 3.10+ 通过 iso 做 ) 按 "回车" 重启  
出现一些 [ OK ] 没动静之后 狂按回车出现 login 提示  

要是忘了 ip, 可以
```  
sudo apt-get install net-tools  
ifconfig
```  

windows 下可用 Bitvist SSH Client 远程终端控制, 方便缩放 和 传文件  

win10 2004+  WSL2 with ubuntu 20.04 LTS 启用 SSH:  
```
sudo vim /etc/ssh/sshd_config  
Port = 22 # 去掉前面的#号  
ListenAddress 0.0.0.0		#去掉前面的#号  
PasswordAuthentication yes # 将 no 改为 yes 表示使用帐号密码方式登录
```  
保存退出后:  
```
sudo dpkg-reconfigure openssh-server  
sudo service ssh restart
```  





## ubuntu 20.04 LTS desktop
建议安装 desktop 版本时可选择 中文, 重启之后各种更新补齐可搞定输入法问题.   
可选择 mini install 并且去掉 检查更新 之类的勾勾 以提升安装速度。  
在 vmware 里安装注意选择 "使用桥接网络". 配置建议 2核 4g, 虚拟磁盘单个 500g  


如果网络异常, 可通过代理操作:
```  
sudo apt-get -o Acquire::http::proxy="http://IP:PORT/" update 或 install ..........
```  
小提示: 半夜 2:30 过后无需代理, 下载速度飞快  
小提示2: 下载没动静, ctrl c 中断再来 或许就快了  


首先：
```  
sudo apt update  
sudo apt install gcc g++ gdb gdbserver libreadline-dev cmake
```  

desktop 版继续安装:
```  
sudo apt install openssh-server net-tools git vim
```  

vmware 下面的 desktop 版继续安装( 安完注销下 )：  
```
sudo apt install open-vm-tools-desktop open-vm-tools
```  

### 如果无法向 vmware 的 ubuntu 拖拽文件入内，则可通过 Bitvist SSH Client, WinSCP 等 SFTP 工具传文件  


### 如果要安装 gcc/g++10 版本, 则执行下列脚本:( ubuntu 18.04 也能用 )  
```
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt install gcc-10 g++-10
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 70 --slave /usr/bin/g++ g++ /usr/bin/g++-10
```



clion 下载解压后 控制台进入 bin 目录运行 ./clion.sh  
向导结束位置会生成菜单图标  


####clion 可设置为 vs 热建习惯并追加 ctrl + w 关闭文件:  
File -- Settings -- Keymap   
选 VS 风格   
搜索 close , 设为 ctrl + w. 提示冲突 选择 remove other.   
搜 Extend selection 设为 alt + w  


####clion 可修改字体字号, 令汉字清晰锐利:  
File -- Settings -- Editor -- Font -- Size: 15  Line spacing 1.1   Fallback font: AR PL UMing CN  


####clion 添加 Release 版本生成:  
File -- Settings -- Build,Execution,Deployment -- CMake 点 "+"  


####clion 设置 valgrind: 
先 sudo apt install valgrind  
File -- Settings -- Build,Execution,Deployment -- Dynamic Analysis Tools -- valgrind -- 设置 路径 /usr/bin/valgrind  


####clion 设置使用 clang 替代默认的 gcc:   
先 sudo apt install llvm-10 llvm-10-dev clang-10 llvm-10-tools  
File -- Settings -- Build,Execution,Deployment -- Toolchains -- 可点击 + 添加一条设置,  c compiler 填写 /usr/bin/clang-10, c++ compiler 填写 /usr/bin/clang++-10  


####clion 典型的 clang 配套 cmake 文件内容( 可能存在的 bug: 远程连接到 linux, C:\Users\xx\AppData\Local\JetBrains\CLion2020.2\.remote\192.168.1.74_22\f64dd6e3-2ee0-4551-bfcd-614330181507\usr\lib\llvm-10 这个目录下有可能缺 include 目录导致 clion 编辑器染色感知失败 但是不影响编译，可从 linux 自行拷贝入内修复这个问题 ):  
```
set(CMAKE_C_COMPILER clang-10)
set(CMAKE_CXX_COMPILER clang++-10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -v -fcoroutines-ts")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto -fuse-ld=gold")
```


## 各种需要安装的东西总结如下: ( 可以按需窜起来一波流 )  
```
gcc g++ gdb gdbserver cmake valgrind
llvm-10 llvm-10-dev clang-10 llvm-10-tools libc++-10-dev libc++abi-10-dev
libreadline-dev
libboost-all-dev
libsqlite3-dev
libmariadb-dev
uuid-dev
libluajit-5.1-dev
liblua5.3-dev
libuv1-dev
openssh-server net-tools git vim

open-vm-tools-desktop open-vm-tools
```





初次拉代码, 打开控制台, 输入  
```
cd ~
git clone https://github.com/denghe/xxlib_simple_cpp.git
```  
之后便可以在 clion 中 open/import 打开目录  




代码找不到配置文件的解决方案:   
1. 复制文件到编译出来的执行文件目录  
2. 建立软连接到执行文件目录  
3. clion 右上角项目选择下拉里选 edit configurations... 修改 Working directory: 为文件所在目录  
	例如 $ProjectFileDir$/server1/  




直接编译的 命令行 参考:   

1. 建目录弄配置出来  
```
cd ~
mkdir xxx
cd xxx
```  
设置为 gcc 编译 release 版: ( 走默认 或 cmakefiles.txt 指定 编译器 )  
```
cmake -DCMAKE_BUILD_TYPE=Release ~/xxlib_simple_cpp
```  
设置为 clang 编译 debug 版:  
```
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=/usr/bin/clang-10 -DCMAKE_CXX_COMPILER=/usr/bin/clang++-10 -G "CodeBlocks - Unix Makefiles" ~/xxlib_simple_cpp
```  

2. 构建  
指定 目标: ( j 2 指并行编译个数为 2. 和 cpu 颗数一致 )  
```
cmake --build ~/xxx --target client1 -- -j 2
```  
不指定目标: ( 全部编译 )
```  
cmake --build ~/xxx -- -j 4
```  






解除 linux fd 限制: /etc/security/limits.conf 追加下面的内容, 保存重启  
```
root hard nofile 65535
root soft nofile 65535
root soft core unlimited
root hard core unlimited
* hard nofile 65535
* soft nofile 65535
* soft core unlimited
* hard core unlimited
```

修改 linux 收发缓冲区最大长度限制( 主要针对 UDP 端口做服务器的情况 ): /etc/sysctl.conf 增加两行, 保存重启
```
net.core.rmem_max=26214400
* net.core.rmem_default=26214400
net.core.wmem_max=26214400
* net.core.wmem_default=26214400
net.core.netdev_max_backlog=2048
```
临时修改并立即生效的命令行:
```
sudo sysctl -w net.core.rmem_max=26214400 net.core.wmem_max=26214400
```




ubuntu 18.04 LTS server，安装时勾安 OpenSSH server 默认安全选项  

安装编译器，依赖库：  
```
sudo apt install gcc-8 g++-8 gdb gdbserver libreadline-dev libboost-all-dev
```  

设置 gcc g++ 命令指向 8.0:
```  
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 60 --slave /usr/bin/g++ g++ /usr/bin/g++-8
```
  
