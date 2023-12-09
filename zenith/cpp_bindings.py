import os
os.environ["EXTRA_CLING_ARGS"] = "-g"
import platform
import cppyy

# These must be installed at the system level
cppyy.add_include_path("/opt/homebrew/include")
cppyy.add_library_path("/opt/homebrew/lib")
cppyy.add_include_path("/usr/local/include")
cppyy.add_library_path("/usr/local/lib")
cppyy.c_include("GL/glew.h")
cppyy.load_library("libGLEW")
cppyy.c_include("GLFW/glfw3.h")
directory = os.path.dirname(os.path.abspath(__file__))
cpp_dir = directory + "/cpp"

sys = platform.system()
if sys == "Linux":
    lib_zenith_suffix = "so"
elif sys == "Windows":
    lib_zenith_suffix = "dll"
elif sys == "Darwin":
    lib_zenith_suffix = "dylib"
else:
    lib_zenith_suffix = "so"

cppyy.add_include_path(cpp_dir)
cppyy.add_include_path(cpp_dir + "/imgui/")
cppyy.include(cpp_dir + "/GLModel.hpp")
cppyy.include(cpp_dir + "/Engine.hpp")
cppyy.include(cpp_dir + "/GLBoilerPlate.hpp")
cppyy.include(cpp_dir + "/vptree.hpp")
cppyy.include(cpp_dir + "/Controls.hpp")
cppyy.include(cpp_dir + "/imgui/imgui_impl_glfw.h")
cppyy.include(cpp_dir + "/imgui/imgui.h")
cppyy.include(cpp_dir + "/imgui/imgui_impl_opengl3.h")
cppyy.include(cpp_dir + "/imgui/imgui_internal.h")
cppyy.include(cpp_dir + "/imgui/imconfig.h")
cppyy.include(cpp_dir + "/imgui/imstb_rectpack.h")
cppyy.include(cpp_dir + "/imgui/imstb_textedit.h")
cppyy.include(cpp_dir + "/imgui/imstb_truetype.h")
cppyy.load_library(directory + '/lib/libzenith.{}'.format(lib_zenith_suffix))

from cppyy.gbl import create2dWorld, create3dWorld, Engine, Engine3d
from cppyy.gbl import GLModel
from cppyy.gbl import GLModelAnimated
