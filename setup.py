import os
from setuptools import setup, Extension

clp_four_byte_encoder: Extension = Extension(
    name="clp_ffi_py.CLPFourByteEncoder",
    language="c++",
    sources=[
        "src/clp/components/core/src/ffi/ir_stream/encoding_methods.cpp",
        "src/clp/components/core/src/ffi/encoding_methods.cpp",
        "src/clp/components/core/src/string_utils.cpp",
        "src/clp/components/core/src/TraceableException.cpp",
        "src/clp_ffi/modules/clp_four_byte_encoder.cpp",
        "src/clp_ffi/encoder/encoding_methods.cpp"
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
    sources=[
        "src/clp/components/core/src/TraceableException.cpp",
        "src/clp/components/core/src/ffi/encoding_methods.cpp",
        "src/clp/components/core/src/string_utils.cpp",
        "src/clp/components/core/src/ffi/ir_stream/decoding_methods.cpp",
        "src/clp_ffi/modules/clp_four_byte_decoder.cpp",
        "src/clp_ffi/decoder/decoding_methods.cpp",
        "src/clp_ffi/decoder/PyDecoderBuffer.cpp",
        "src/clp_ffi/components/Message.cpp",
        "src/clp_ffi/components/Metadata.cpp",
        "src/clp_ffi/utilities.cpp",
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
    sources=[
        "src/clp/components/core/src/TraceableException.cpp",
        "src/clp/components/core/src/string_utils.cpp",
        "src/clp_ffi/modules/ir_components.cpp",
        "src/clp_ffi/components/Message.cpp",
        "src/clp_ffi/components/Metadata.cpp",
        "src/clp_ffi/components/PyMessage.cpp",
        "src/clp_ffi/components/PyMetadata.cpp",
        "src/clp_ffi/components/PyQuery.cpp",
        "src/clp_ffi/components/Query.cpp",
        "src/clp_ffi/utilities.cpp",
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
