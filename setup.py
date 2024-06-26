from typing import List

from setuptools import setup, find_namespace_packages, Extension
from setuptools.command.build_ext import build_ext
import sys
import setuptools
import pathlib
import itertools
import os


setuptools.distutils.log.set_verbosity(1)

__version__ = "0.0.8"


class get_pybind_include(object):
    """Helper class to determine the pybind11 include path

    The purpose of this class is to postpone importing pybind11
    until it is actually installed, so that the ``get_include()``
    method can be invoked."""

    def __str__(self):
        import pybind11

        return pybind11.get_include()


def filter_cpp_or_c_files(directory: pathlib.Path) -> List[pathlib.Path]:
    files = list(directory.iterdir())
    files = [
        file for file in files if file.name.endswith(".c") or file.name.endswith(".cpp") or (sys.platform == "darwin" and file.name.endswith(".m"))
    ]
    return files

project_dirs = [
    "zenith_viz/cpp/glad/src",
    "zenith_viz/cpp",
    "zenith_viz/cpp/glfw/src",
    "zenith_viz/cpp/imgui",
]


zenith_viz_srcs_with_deps = list(
    sorted(
        map(
            str,
            list(
                itertools.chain.from_iterable(
                    [filter_cpp_or_c_files(pathlib.Path(d)) for d in project_dirs]
                )
            ),
        )
    )
)

zenith_viz_srcs_with_deps = zenith_viz_srcs_with_deps + [
    "zenith_viz/cpp/imgui/backends/imgui_impl_opengl3.cpp",
    "zenith_viz/cpp/imgui/backends/imgui_impl_glfw.cpp",
]

for item in zenith_viz_srcs_with_deps:
    print("Found Source File: ", item)


def fq_path(p):
    return str(pathlib.Path(p))


zenith_viz_include_dirs = [
    fq_path("zenith_viz/cpp"),
    fq_path("zenith_viz/cpp/glad/include"),
    fq_path("zenith_viz/cpp/glfw/include"),
    fq_path("zenith_viz/cpp/glm"),
    fq_path("zenith_viz/cpp/imgui"),
    str(get_pybind_include()),
]
ext_modules = [
    Extension(
        "_zenith",
        zenith_viz_srcs_with_deps,
        include_dirs=zenith_viz_include_dirs,
        language="c++",
        extra_compile_args=["-g"],
    ),
]


def has_flag(compiler, flagname):
    """Return a boolean indicating whether a flag name is supported on
    the specified compiler.
    """
    import tempfile
    import os

    with tempfile.NamedTemporaryFile("w", suffix=".cpp", delete=False) as f:
        f.write("int main (int argc, char **argv) { return 0; }")
        fname = f.name
    try:
        compiler.compile([fname], extra_postargs=[flagname])
    except setuptools.distutils.errors.CompileError:
        return False
    finally:
        try:
            os.remove(fname)
        except OSError:
            pass
    return True


class BuildExt(build_ext):
    """A custom build extension for adding compiler-specific options."""

    c_opts = {
        "msvc": ["/EHsc"],
        "unix": [],
    }
    l_opts = {
        "msvc": [],
        "unix": [],
    }

    if sys.platform == "darwin":
        frameworks = ["-framework Cocoa", "-framework IOKit", "-framework Foundation", "-framework QuartzCore", "-framework OpenGL"]
        os.environ["LDFLAGS"] = " ".join(frameworks)
        darwin_opts = ["-stdlib=libc++", "-mmacosx-version-min=10.7", "-std=c++17"]
        c_opts["unix"] += darwin_opts
        l_opts["unix"] += darwin_opts

    def build_extensions(self):
        ct = self.compiler.compiler_type
        opts = self.c_opts.get(ct, [])
        link_opts = self.l_opts.get(ct, [])
        if sys.platform == "linux" or sys.platform == "linux2":
            opts.append("-D_GLFW_X11")
        elif sys.platform == "darwin":
            original_compile_func = self.compiler._compile

            def monkey_patched_compile(obj, src, ext, cc_args, extra_postargs, pp_opts):
                if src.endswith(".c") or src.endswith(".m"):
                    cc_args = [cc_arg for cc_arg in cc_args if cc_arg != "-std=c++17"]
                    extra_postargs = [ep for ep in extra_postargs if ep != "-std=c++17"]
                return original_compile_func(obj, src, ext, cc_args, extra_postargs, pp_opts)

            self.compiler._compile = monkey_patched_compile

            opts.append("-D_GLFW_COCOA")
        elif sys.platform == "win32":
            opts.append("-D_GLFW_WIN32")
        else:
            print(
                "This platform cannot support zenith_viz's window/rendering engine! Open an issue on Github."
            )
            sys.exit(0)

        if ct == "unix":

            if has_flag(self.compiler, "-fvisibility=hidden"):
                opts.append("-fvisibility=hidden")
        for ext in self.extensions:
            ext.define_macros = [
                ("VERSION_INFO", '"{}"'.format(self.distribution.get_version()))
            ]
            ext.extra_compile_args = opts
            ext.extra_link_args = link_opts
        self.force = True
        build_ext.build_extensions(self)

setup(
    name="zenith_viz",
    version=__version__,
    author="Michael Pfaffenberger",
    author_email="mike.pfaffenberger@gmail.com",
    url="https://github.com/mpfaffenberger/zenith-viz",
    description="3D Accelerated Data Viz",
    long_description="",
    packages=find_namespace_packages(where="./"),
    package_data={
        "zenith_viz": [
            "zenith_viz/resources/colors.yml",
            "zenith_viz/shaders/fragmentShader.shader",
            "zenith_viz/shaders/vertexShader.shader",
        ],
    },
    include_package_data=True,
    ext_modules=ext_modules,
    cmdclass={"build_ext": BuildExt},
    zip_safe=False,
)
