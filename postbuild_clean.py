"""
Clean up the prebuild strategy of making a copy of the main.cpp file and
replacing the contents of the original main file by deleting the replaced file
and restoring the copy file.
"""
import os
import shutil

BASE_DIR = os.path.dirname(__file__)

main_cpp_file = os.path.join(BASE_DIR, "src", "main.cpp")
copy_cpp_file = os.path.join(BASE_DIR, "src", ".raw_main")

os.remove(main_cpp_file)
os.rename(copy_cpp_file, main_cpp_file)