package("cgraph")
    set_homepage("https://github.com/ChunelFeng/CGraph")
    set_urls("https://github.com/ChunelFeng/CGraph.git")
    add_versions("v2.5.2", "cae6776230957f2125c4f204687ef479a69571e1")
    on_install(function (package)
        io.writefile("xmake.lua", [[
                    set_languages("cxx11")
                    add_rules("mode.debug", "mode.release")

                    if is_plat("macosx") then
                        add_defines("_ENABLE_LIKELY_")
                    elseif is_plat("linux") then
                        add_defines("_ENABLE_LIKELY_")
                        add_syslinks("pthread")
                    end

                    target("cgraph")
                        set_kind("static")
                        add_files("src/**.cpp")
                        add_defines("_ENABLE_LIKELY_")
                        add_headerfiles("src/(**.h)")
                        add_headerfiles("src/(**.inl)")
                        add_forceincludes("cstring")
                ]])
        import("package.tools.xmake").install(package, configs)
    end)
    
package_end()

