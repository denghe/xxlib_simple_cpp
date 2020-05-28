ubuntu 20.04 LTS server/desktop， 安装时勾安 OpenSSH server 默认安全选项
可选择 mini install 并且去掉 检查更新 之类的勾勾 以提升安装速度。

建议安装 desktop 版本时可选择 中文, 之后按提示操作可搞定输入法问题. 
安完后先 sudo apt update 一次

如果网络异常, 可通过代理操作:
sudo apt-get -o Acquire::http::proxy="http://IP:PORT/" update 或 install ..........

server 版安装：
sudo apt install gcc g++ gdb gdbserver libreadline-dev libboost-all-dev

desktop 版继续安装:
sudo apt install cmake openssh-server net-tools git vim

vmware 下面的 desktop 版继续安装( 安完注销下 )：
sudo apt install open-vm-tools-desktop open-vm-tools

如果无法向 vmware 的 ubuntu 拖拽文件入内，则可通过 WinSCP 等 SFTP 工具传文件, putty 远程终端控制, 或者 Bitvist SSH Client
用 ifconfig 看 ip

完整依赖列表: gcc g++ gdb gdbserver libreadline-dev libboost-all-dev uuid-dev libsqlite3-dev libmariadb-dev cmake openssh-server net-tools git vim open-vm-tools-desktop open-vm-tools





初次拉代码, 打开控制台, 输入
cd ~
git clone https://github.com/denghe/xxlib_simple_cpp.git
之后便可以在 clion 中 open/import 打开目录




clion 下载解压后 控制台进入 bin 目录运行 ./clion.sh
向导结束位置会生成菜单图标

clion 可设置为 vs 热建习惯并追加 ctrl + w 关闭文件:
File -- Settings -- Keymap 选 VS 风格 搜索 close , 设为 ctrl + w 会提示 冲突， 修改冲突的为 alt + w 再设.


clion 添加 Release 版本生成:
File -- Settings -- Build,Execution,Deployment -- CMake 点 "+"



代码找不到配置文件的解决方案: 
1. 复制文件到编译出来的执行文件目录
2. 建立软连接到执行文件目录
3. clion 右上角项目选择下拉里选 edit configurations... 修改 Working directory: 为文件所在目录
	例如 $ProjectFileDir$/server1/




解除 linux fd 限制: /etc/security/limits.conf 追加下面的内容, 保存重启
root hard nofile 65535
root soft nofile 65535
root soft core unlimited
root hard core unlimited
* hard nofile 65535
* soft nofile 65535
* soft core unlimited
* hard core unlimited



ubuntu 18.04 LTS server，安装时勾安 OpenSSH server 默认安全选项

安装编译器，依赖库：
sudo apt install gcc-8 g++-8 gdb gdbserver libreadline-dev libboost-all-dev

设置 gcc g++ 命令指向 8.0:
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 60 --slave /usr/bin/g++ g++ /usr/bin/g++-8



