package("ne10")
    set_homepage("https://github.com/projectNe10/Ne10")
    set_urls("https://github.com/projectNe10/Ne10.git")
    add_versions("github:v1.2.1", "f077c0d4892271cb8a7332aa28a4155fcb5477d47d1e024ff019a2471e2cfbd3")

    add_deps("cmake")
    on_install(function (package)
        local configs = { }
        if is_arch("armv7") then
            table.insert(configs, "-DNE10_LINUX_TARGET_ARCH=armv7")
            table.insert(configs, "-DCMAKE_TOOLCHAIN_FILE=../GNUlinux_config.cmake ..")
            -- table.insert(configs, "-DGNULINUX_PLATFORM=ON")
        end
        if is_arch("arm64-v8a") then
            table.insert(configs, "-DNE10_LINUX_TARGET_ARCH=aarch64")
            table.insert(configs, "-DGNULINUX_PLATFORM=ON")
        end
        os.mkdir("build")
        import("package.tools.cmake").install(package, configs, {buildir = "build"})
    end)

package_end()
