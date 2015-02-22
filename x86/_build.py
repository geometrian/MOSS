from subprocess import call

import os

root = ""
root_build  = root+"build/"
root_source = root+"source/"

skip = 1
if skip == 0:
    skip_recompile_directories = []
else:
    skip_recompile_directories = [root_source+"kernel/boot",root_source+"mossc/",root_source+"mosst/"]

only = 0
if only == 0:
    only_recompile = []
else:
    only_recompile = ["gui"]

#The standard C++11 is important for override in C++.
#TODO: do we need -nostartfiles?
args_compile = "-ffreestanding -O0 -Wall -Wextra -fno-exceptions -fno-rtti -std=c++11"
args_link = "-ffreestanding -O0 -nostdlib"



##def call(cmd_list):
##    print(cmd_list)

link_files = []

def get_should_compile(in_name):
    for skip_recompile_directory in skip_recompile_directories:
        if in_name.startswith(skip_recompile_directory):
            return False
    if len(only_recompile) != 0:
        found = False
        for only in only_recompile:
            if only in in_name:
                found = True
                break
        if not found:
            return False
    return True
def compile_cpp(in_name,out_name):
    if get_should_compile(in_name):
        print("    Compiling:  \""+in_name+"\"")
        command = ["i586-elf-g++","-c",in_name,"-o",out_name]
        command += args_compile.split(" ")
        call(command)
    else:
        print("  ##SKIPPING##: \""+in_name+"\"")
    link_files.append(out_name)
def assemble_asm(in_name,out_name):
    if get_should_compile(in_name):
        print("    Assembling: \""+in_name+"\"")
        command = ["nasm","-felf",in_name,"-o",out_name]
        call(command)
    else:
        print("  ##SKIPPING##: \""+in_name+"\"")
    link_files.append(out_name)

def compile_directory(directory):
    output_dir = root_build+directory[len(root_source):]
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
            
    for file in os.listdir(directory):
        filename = os.path.join(directory,file)
        
        if os.path.isfile(filename):
            def get_out_name(in_name):
                out_name = output_dir+in_name.split("/")[-1][:-4]
                if out_name+".o" in link_files:
                    i = 0
                    while out_name+str(i)+".o" in link_files:
                        i += 1
                    out_name += str(i)
                out_name += ".o"
                return out_name
            if filename.endswith(".cpp"):
                compile_cpp(filename,get_out_name(filename))
            elif filename.endswith(".asm"):
                assemble_asm(filename,get_out_name(filename))
                
        elif os.path.isdir(filename):
            if "old" in filename: continue
                
            compile_directory(filename)

def link():
    command = ["i586-elf-gcc","-T",root_source+"linker.ld","-o",root_build+"MOSS.bin"]
    command += args_link.split(" ")
    for file in link_files:
        command.append(file)
    command.append("-lgcc")
    call(command)

def main():
    try:
        print("  Compiling:")
        compile_directory(root_source)

        print("  Linking")
        link()
    except KeyboardInterrupt:
        print("  Aborting")


if __name__ == "__main__": main()
























