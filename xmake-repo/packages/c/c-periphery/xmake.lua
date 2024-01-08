package("c-periphery")
    set_homepage("https://github.com/vsergeev/c-periphery")
    set_urls("https://github.com/vsergeev/c-periphery/archive/refs/tags/$(version).tar.gz")
    add_versions("v2.4.2", "24327bc2a22588444b2576fa1c9560619f6faad986c236c54f4b3f36b20dad56")
    
    add_deps("cmake")
    on_install(function (package)
        local configs = {}
        if package:config("shared") then
            table.insert(configs, "-DBUILD_SHARED_LIBS=ON")
        end
        os.mkdir("build")
        import("package.tools.cmake").install(package, configs, {buildir = "build"})
    end)

    on_test(function (package)
        assert(package:has_cxxfuncs("gpio_new", {includes = "periphery/gpio.h"}))
    end)
    
package_end()