import os
import shutil

BASE_DIR = os.path.dirname(__file__)

default_conf      = ""
default_conf_path = os.path.join(BASE_DIR, "src", "default_config.yml")
main_cpp_file     = os.path.join(BASE_DIR, "src", "main.cpp")
copy_cpp_file     = os.path.join(BASE_DIR, "src", ".raw_main")

with open(default_conf_path) as f:
    default_conf = f.read()

default_conf = default_conf.replace('\n', '\\n')

# copy main file to a hidden file to facilitate build without replacing the
# config macro in the actual source code
shutil.copyfile(main_cpp_file, copy_cpp_file)

main_file_contents = ""

with open(main_cpp_file, "r") as f:
    main_file_contents = f.read()
    main_file_contents = main_file_contents.replace("{{ default_config }}", default_conf)

with open(main_cpp_file, "w") as f:
    f.write(main_file_contents)