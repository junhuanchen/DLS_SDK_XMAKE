set_project("CPP_Xmake_Env_Template")
set_languages("c99", "cxx17")

--armv7 v83x v85x 工具链()
--命令：xmake f -v -y -p linux -a armv7 --toolchain=armv7-toolchain --cross=arm-openwrt-linux-muslgnueabi-
toolchain("armv7-toolchain")
    set_kind("standalone")
    set_sdkdir(string.format("%s/toolchain", os.getenv("PWD")))
toolchain_end()

-- 编译模式 --
-- 模式切换：xmake f -m debug/release
add_rules("mode.debug", "mode.release")

add_requires("sqlite3")

-- 程序检查工具 --
-- 命令：xmake f -m debug -v --policies=build.sanitizer.address,build.sanitizer.undefined
--     set_policy("build.sanitizer.address", true) --快速的内存错误检测工具
--     set_policy("build.sanitizer.thread", true) --检测线程安全问题
--     set_policy("build.sanitizer.memory", true) --检测内存问题
--     set_policy("build.sanitizer.leak", true) --检测内存泄漏问题
--     set_policy("build.sanitizer.undefined", true) --检测未定义行为

-- 项目设置 --
-- 导入二级目录子项目配置
includes("*/xmake.lua")

-- 全局范围内可使用的依赖包 --
-- 使用本地仓库
add_repositories("local-repo xmake-repo")

-- 导入外部依赖包
-- add_requires("boost")

-- -- 脚本域 --
-- -- 安装V831交叉编译工具链：xmake run install_v831_toolchain
-- target("install_v831_toolchain")
--     set_default("false")
--     set_kind("phony") --假的目标，只是用来执行命令 https://xmake.io/#/zh-cn/manual/project_target?id=设置目标编译类型
--     on_run(function(target)
--         import("privilege.sudo")
--         -- 生成配置文件
--         if not os.isdir("/opt/v83x_linux_x86_python3.8_toolchain") then
--             sudo.exec("apt update")
--             sudo.exec("apt install android-tools-adb cmake git make rsync uuid-dev autopoint -y")
--             os.exec("wget http://mirrors.kernel.org/ubuntu/pool/main/libf/libffi/libffi6_3.2.1-8_amd64.deb")
--             sudo.exec("dpkg -i libffi6_3.2.1-8_amd64.deb")
--             os.exec("rm libffi6_3.2.1-8_amd64.deb")
--             -- os.exec("wget https://github.com/sipeed/MaixPy3/releases/download/20210613/v83x_linux_x86_python3.8_toolchain.zip")
--             os.exec("wget https://stream.justasite.cn:12280/backup/public/-/raw/main/v83x_linux_x86_python3.8_toolchain.zip")
--             os.exec("unzip v83x_linux_x86_python3.8_toolchain.zip")
--             os.exec("rm v83x_linux_x86_python3.8_toolchain.zip")
--             sudo.exec("mkdir /opt/v83x_linux_x86_python3.8_toolchain")
--             sudo.exec("mv v83x_linux_x86_python3.8_toolchain /opt/")
--             if os.isfile("/usr/lib/libgcc_s.so.1") then
--                 sudo.exec("rm /usr/lib/libgcc_s.so.1")
--             end
--         end
--     end)
