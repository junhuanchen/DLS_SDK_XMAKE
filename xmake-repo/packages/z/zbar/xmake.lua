package("zbar")
    set_homepage("https://github.com/mchehab/zbar")
    set_urls("https://github.com/mchehab/zbar/archive/refs/tags/$(version).tar.gz")
    add_versions("0.23.90", "25fdd6726d5c4c6f95c95d37591bfbb2dde63d13d0b10cb1350923ea8b11963b")
    add_versions("0.23.1", "297439f8859089d2248f55ab95b2a90bba35687975365385c87364c77fdb19f3")
    add_versions("0.23", "a4fb80c05fb05af6490f62f62df7000b8287f6f27bf8b723080a5b57556f7911")
    
    on_install(function (package) 
        local configs = {}
        os.vrunv("autoreconf", {"-fiv"})

        if os.arch() == is_arch("armv8a") then
            local result = package:find_tool("python3", opt)
            if result then
                table.insert(configs, "--with-python=auto")
            end
        end

        if is_arch("armv7") then
            table.insert(configs, "--host=arm-linux --target=armv7")
            table.insert(configs, "--prefix=$PWD/output/ --without-gtk --without-qt --without-imagemagick --without-python --without-jpeg --disable-video CC=arm-openwrt-linux-muslgnueabi-gcc CXX=arm-openwrt-linux-muslgnueabi-g++")
        end

        import("package.tools.autoconf").install(package,configs)
        import("package.tools.make").build(package)
    end)

    on_test(function (package)
        assert(package:has_cfuncs("zbar_image_create", {includes = "zbar.h"}))
    end)
    
package_end()