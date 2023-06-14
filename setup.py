import os
from setuptools import setup, Extension

clp_four_byte_encoder: Extension = Extension(
    name="clp_ffi_py.CLPFourByteEncoder",
    language="c++",
    include_dirs=[
        "src"
    ],
    sources=[
        "src/clp/components/core/src/ffi/ir_stream/encoding_methods.cpp",
        "src/clp/components/core/src/ffi/encoding_methods.cpp",
        "src/clp/components/core/src/string_utils.cpp",
        "src/clp/components/core/src/TraceableException.cpp",
        "src/clp_ffi_py/modules/clp_four_byte_encoder.cpp",
        "src/clp_ffi_py/encoder/encoding_methods.cpp"
    ],
    extra_compile_args=[
        '-std=c++17',
        "-O3"
    ],
    define_macros=[
        ("SOURCE_PATH_SIZE", str(len(os.path.abspath("./src/clp/components/core"))))
    ]
)

clp_four_byte_decoder: Extension = Extension(
    name="clp_ffi_py.CLPFourByteDecoder",
    language="c++",
    include_dirs=[
        "src"
    ],
    sources=[
        "src/clp/components/core/src/TraceableException.cpp",
        "src/clp/components/core/src/ffi/encoding_methods.cpp",
        "src/clp/components/core/src/string_utils.cpp",
        "src/clp/components/core/src/ffi/ir_stream/decoding_methods.cpp",
        "src/clp_ffi_py/modules/clp_four_byte_decoder.cpp",
        "src/clp_ffi_py/decoder/decoding_methods.cpp",
        "src/clp_ffi_py/decoder/PyDecoderBuffer.cpp",
        "src/clp_ffi_py/components/Message.cpp",
        "src/clp_ffi_py/components/Metadata.cpp",
        "src/clp_ffi_py/components/Query.cpp",
        "src/clp_ffi_py/utilities.cpp",
    ],
    extra_compile_args=[
        '-std=c++17',
        "-O3"
    ],
    define_macros=[
        ("SOURCE_PATH_SIZE", str(len(os.path.abspath("./src/clp/components/core"))))
    ]
)

ir: Extension = Extension(
    name="clp_ffi_py.IRComponents",
    language="c++",
    include_dirs=[
        "src"
    ],
    sources=[
        "src/clp/components/core/src/TraceableException.cpp",
        "src/clp/components/core/src/string_utils.cpp",
        "src/clp_ffi_py/modules/ir_components.cpp",
        "src/clp_ffi_py/components/Message.cpp",
        "src/clp_ffi_py/components/Metadata.cpp",
        "src/clp_ffi_py/components/PyMessage.cpp",
        "src/clp_ffi_py/components/PyMetadata.cpp",
        "src/clp_ffi_py/components/PyQuery.cpp",
        "src/clp_ffi_py/components/Query.cpp",
        "src/clp_ffi_py/utilities.cpp",
    ],
    extra_compile_args=[
        '-std=c++17',
        "-O3"
    ],
    define_macros=[
        ("SOURCE_PATH_SIZE", str(len(os.path.abspath("./src/clp/components/core"))))
    ]
)


setup(
    name="clp_ffi_py",
    description="CLP FFI Python Interface",
    ext_modules=[clp_four_byte_decoder, clp_four_byte_encoder, ir],
    packages=["clp_ffi_py"],
)
