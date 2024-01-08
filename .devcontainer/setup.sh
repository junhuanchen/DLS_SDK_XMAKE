# 更换国内软件源
sudo cp /etc/apt/sources.list /etc/apt/sources.list.bak
sudo rm /etc/apt/sources.list
sudo touch /etc/apt/sources.list
sudo chmod 777 /etc/apt/sources.list
sudo echo deb http://cn.archive.ubuntu.com/ubuntu/ jammy main restricted >> /etc/apt/sources.list ;
sudo echo deb http://cn.archive.ubuntu.com/ubuntu/ jammy-updates main restricted >> /etc/apt/sources.list ;
sudo echo deb http://cn.archive.ubuntu.com/ubuntu/ jammy universe >> /etc/apt/sources.list ;
sudo echo deb http://cn.archive.ubuntu.com/ubuntu/ jammy-updates universe >> /etc/apt/sources.list ;
sudo echo deb http://cn.archive.ubuntu.com/ubuntu/ jammy multiverse >> /etc/apt/sources.list ;
sudo echo deb http://cn.archive.ubuntu.com/ubuntu/ jammy-updates multiverse >> /etc/apt/sources.list ;
sudo echo deb http://cn.archive.ubuntu.com/ubuntu/ jammy-backports main restricted universe multiverse >> /etc/apt/sources.list ;
sudo echo deb http://security.ubuntu.com/ubuntu jammy-security main restricted >> /etc/apt/sources.list ;
sudo echo deb http://security.ubuntu.com/ubuntu jammy-security universe >> /etc/apt/sources.list ;
sudo echo deb http://security.ubuntu.com/ubuntu jammy-security multiverse >> /etc/apt/sources.list ;
sudo chmod 777 /etc/apt/sources.list

# 安装开发环境工具(代码检查工具：CppCheck)
sudo apt update && sudo apt upgrade -y
sudo apt install -y \
    build-essential \
    cmake \
    curl \
    git \
    libboost-all-dev \
    libeigen3-dev \
    libgflags-dev \
    libgoogle-glog-dev \
    libopencv-dev \
    libprotobuf-dev \
    protobuf-compiler \
    python3-dev \
    python3-pip \
    python3-setuptools \
    python3-wheel \
    vim \
    wget \
    zip \
    cppcheck \
    ninja-build \
    unzip

# 安装 Xmake
wget https://xmake.io/shget.text -O - | bash

# 使用xmake脚本进行特定环境配置
xmake run install_v831_toolchain # 安装V831工具链

# 安装 gh
type -p curl >/dev/null || (sudo apt update && sudo apt install curl -y)
curl -fsSL https://cli.github.com/packages/githubcli-archive-keyring.gpg | sudo dd of=/usr/share/keyrings/githubcli-archive-keyring.gpg \
&& sudo chmod go+r /usr/share/keyrings/githubcli-archive-keyring.gpg \
&& echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/githubcli-archive-keyring.gpg] https://cli.github.com/packages stable main" | sudo tee /etc/apt/sources.list.d/github-cli.list > /dev/null \
&& sudo apt update \
&& sudo apt install gh -y
