print('### Pre-debug script ###############################')

from distutils.dir_util import copy_tree
import shutil, os

copy_tree("src" , "test")
if os.path.exists("test/main.cpp"):
    os.remove("test/main.cpp")

print('### Finish Pre-debug script ########################')
