ubuntu 20.04 LTS server/desktop， 安装时勾安 OpenSSH server 默认安全选项
可选择 mini install 并且去掉 检查更新 之类的勾勾 以提升安装速度。

server 版安装：
sudo apt install gcc g++ gdb gdbserver libreadline-dev

desktop 版继续安装:
sudo apt install cmake openssh-server net-tools git

vmware 下面的 desktop 版继续安装：
sudo apt install open-vm-tools-desktop open-vm-tools

如果无法向 vmware 的 ubuntu 拖拽文件入内，则可通过 WinSCP 等 SFTP 工具传文件, putty 远程终端控制, 或者 Bitvist SSH Client






clion 编辑器创建启动菜单：
在 /usr/share/applications 创建并复制 clion.desktop 文件，内容如下：( Exec 的路径自己改改 )

[Desktop Entry]
Encoding=UTF-8
Name=realsense
Type=Application
Icon=cheese
Categories=Application
StartupNotify=true
Terminal=false
Exec=sh ~/Desktop/clion-2020.1.1/bin/clion.sh

 









ubuntu 18.04 LTS server，安装时勾安 OpenSSH server 默认安全选项

安装编译器，依赖库：
sudo apt install gcc-8 g++-8 gdb gdbserver libreadline-dev

设置 gcc g++ 命令指向 8.0:
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 60 --slave /usr/bin/g++ g++ /usr/bin/g++-8









如果网络异常, 可通过代理安装:
sudo apt-get -o Acquire::http::proxy="http://xxxxxxxxxxxxxx:xxx/" install ..............








解除 linux fd 限制: /etc/security/limits.conf 追加下面的内容, 保存重启
root hard nofile 65535
root soft nofile 65535
root soft core unlimited
root hard core unlimited
* hard nofile 65535
* soft nofile 65535
* soft core unlimited
* hard core unlimited
