from subprocess import call

import os

root = ""
root_build  = root+"build/"
root_source = root+"source/"

#The standard C++11 is important for override in C++.
#TODO: do we need -nostartfiles?
args_compile = "-ffreestanding -O0 -Wall -Wextra -fno-exceptions -fno-rtti -std=c++11"
args_link = "-ffreestanding -O0 -nostdlib"



##def call(cmd_list):
##    print(cmd_list)

link_files = []

def compile_cpp(in_name,out_name):
    print("    Compiling:  \""+in_name+"\"")
    command = ["i586-elf-g++","-c",in_name,"-o",out_name]
    command += args_compile.split(" ")
    call(command)
    link_files.append(out_name)
def assemble_asm(in_name,out_name):
    print("    Assembling: \""+in_name+"\"")
    command = ["nasm","-felf",in_name,"-o",out_name]
    call(command)
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
    print("  Compiling:")
    compile_directory(root_source)

    print("  Linking")
    link()
    

if __name__ == "__main__": main()
























