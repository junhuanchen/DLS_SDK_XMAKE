package("highway")
    set_homepage("https://github.com/google/highway")
    set_urls("https://github.com/google/highway.git")
    add_versions("github:1.0.5", "f6874b352d917a2fd268f0d72000d9937354d3b3c3dc5fee8ffa2f27b2ff7b5f")
    
    add_deps("cmake")
    on_install(function (package)
        local configs = { }
        if is_arch("armv7") then
            table.insert(configs, "-DHWY_CMAKE_ARM7:BOOL=ON")
        end
        os.mkdir("build")
        import("package.tools.cmake").install(package, configs, {buildir = "build"})
    end)
    
package_end()