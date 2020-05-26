项目为 vs2019 linux cpp. 要求有 linux 服务器或 win10 wsl 可供连接并远程编译调试运行


ubuntu 18.04 LTS server，安装时勾安 OpenSSH server 默认安全选项

安装编译器，依赖库：
sudo apt install gcc-8 g++-8 gdb gdbserver libreadline-dev

设置 gcc g++ 命令指向 8.0:
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 60 --slave /usr/bin/g++ g++ /usr/bin/g++-8





win10 WSL, 安装编译器，依赖库同上, 进一步安装 ssh zip
修改 /etc/ssh/sshd_config, 开启 Port Addresxxxxxx 那几个, 以及 PasswordAuthentication 改为 yes
并 sudo service ssh restart
如果报类似下列错误:
Could not load host key: /etc/ssh/ssh_host_ed25519_key
...
修正：
sudo ssh-keygen -t rsa -f /etc/ssh/ssh_host_rsa_key
sudo ssh-keygen -t dsa -f /etc/ssh/ssh_host_dsa_key
sudo ssh-keygen -t ecdsa -f /etc/ssh/ssh_host_ecdsa_key
sudo ssh-keygen -t ed25519 -f /etc/ssh/ssh_host_ed25519_key

注意: 每次重启系统之后 下次 需要执行  sudo service ssh restart  来启动 ssh 服务. 
不想麻烦的话自己也可以参考网上一些自动开启 ssh 服务的方案来一发





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
