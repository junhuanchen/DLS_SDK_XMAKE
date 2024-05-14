add_rules("mode.debug", "mode.release")

target("components")
    set_kind("static")
    add_includedirs("include", {public = true})
    add_includedirs("src", {public = true})
    add_files("src/*.c")
    add_files("src/*.cpp")
    add_files("lib/*.a")
