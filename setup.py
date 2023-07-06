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
        ("SOURCE_PATH_SIZE", 256)
    ]
)

clp_ir_decoder: Extension = Extension(
    name="clp_ffi_py.CLPIRDecoder",
    language="c++",
    include_dirs=[
        "src"
    ],
    sources=[
        "src/clp/components/core/src/TraceableException.cpp",
        "src/clp/components/core/src/ffi/encoding_methods.cpp",
        "src/clp/components/core/src/string_utils.cpp",
        "src/clp/components/core/src/ffi/ir_stream/decoding_methods.cpp",
        "src/clp_ffi_py/modules/clp_ir_decoder.cpp",
        "src/clp_ffi_py/decoder/decoding_methods.cpp",
        "src/clp_ffi_py/decoder/PyDecoderBuffer.cpp",
        "src/clp_ffi_py/decoder/Message.cpp",
        "src/clp_ffi_py/decoder/Metadata.cpp",
        "src/clp_ffi_py/decoder/PyMessage.cpp",
        "src/clp_ffi_py/decoder/PyMetadata.cpp",
        "src/clp_ffi_py/decoder/PyQuery.cpp",
        "src/clp_ffi_py/decoder/Query.cpp",
        "src/clp_ffi_py/utilities.cpp",
        "src/clp_ffi_py/Py_utils.cpp",
    ],
    extra_compile_args=[
        '-std=c++17',
        "-O3"
    ],
    define_macros=[
        ("SOURCE_PATH_SIZE", 256)
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
        "src/clp_ffi_py/decoder/Message.cpp",
        "src/clp_ffi_py/decoder/Metadata.cpp",
        "src/clp_ffi_py/decoder/PyMessage.cpp",
        "src/clp_ffi_py/decoder/PyMetadata.cpp",
        "src/clp_ffi_py/decoder/PyQuery.cpp",
        "src/clp_ffi_py/decoder/Query.cpp",
        "src/clp_ffi_py/utilities.cpp",
        "src/clp_ffi_py/Py_utils.cpp",
    ],
    extra_compile_args=[
        '-std=c++17',
        "-O3"
    ],
    define_macros=[
        ("SOURCE_PATH_SIZE", 256)
    ]
)


setup(
    name="clp_ffi_py",
    description="CLP FFI Python Interface",
    ext_modules=[clp_ir_decoder, clp_four_byte_encoder],
    packages=["clp_ffi_py"],
)
