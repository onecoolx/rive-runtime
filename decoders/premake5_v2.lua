dofile('rive_build_config.lua')

if _OPTIONS['no-rive-decoders'] then
    return
end

local rive = path.getabsolute('../')

newoption({
    trigger = 'no_rive_png',
    description = 'don\'t build png (or zlib) support into the rive_decoders library (built-in png decoding will fail)',
})

newoption({
    trigger = 'no_rive_jpeg',
    description = 'don\'t build jpeg support into the rive_decoders library (built-in jpeg decoding will fail)',
})

newoption({
    trigger = 'no_rive_webp',
    description = 'don\'t build webp support into the rive_decoders library (built-in webp decoding will fail)',
})

if not _OPTIONS["no_rive_png"] then
    dofile(rive .. '/dependencies/premake5_libpng_v2.lua')
end

if not _OPTIONS["no_rive_jpeg"] then
    dofile(rive .. '/dependencies/premake5_libjpeg_v2.lua')
end

if not _OPTIONS["no_rive_webp"] then
    dofile(rive .. '/dependencies/premake5_libwebp_v2.lua')
end

dofile(rive .. '/dependencies/premake5_libwebp_v2.lua')

project('rive_decoders')
do
    dependson('libwebp')
    kind('StaticLib')
    flags({ 'FatalCompileWarnings' })

    includedirs({
        'include',
        '../include',
        '%{cfg.targetdir}/include/libpng',
    })

    files({
        'src/bitmap_decoder.cpp',
        'src/bitmap_decoder_thirdparty.cpp',
    })

    filter({ 'options:not no-libjpeg-renames' })
    do
        includedirs({
            rive .. '/dependencies',
        })
        forceincludes({ 'rive_libjpeg_renames.h' })
    end

    filter({'options:not no_rive_png'})
    do
        includedirs({
            libpng
            })
        dependson('zlib', 'libpng')
        defines({ 'RIVE_PNG' })
        files({ 'src/decode_png.cpp' })
    end

    filter({'options:not no_rive_jpeg'})
    do
        includedirs({
            libjpeg
            })
        dependson('libjpeg')
        defines({ 'RIVE_JPEG' })
        files({ 'src/decode_jpeg.cpp' })
    end

    filter({'options:not no_rive_webp'})
    do
        includedirs({
            libwebp .. '/src'
            })
        dependson('libwebp')
        defines({ 'RIVE_WEBP' })
        files({ 'src/decode_webp.cpp' })
    end
end
