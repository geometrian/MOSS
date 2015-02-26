from subprocess import call

import os

import hashlib

try:    import cPickle as pickle #Only present in Python 2.*; Python 3 automatically imports the
except: import  pickle as pickle #new equivalent of cPickle, if it's available.


root = ""
root_build  = root+".build/"
root_source = root+"source_moss/"

skip = 1
if skip == 0:
    skip_recompile_directories = [root_source+"grub/"]
else:
    skip_recompile_directories = [root_source+"grub/",root_source+"kernel/boot",root_source+"mossc/",root_source+"mosst/"]
only = 0
if only == 0:
    only_recompile = []
else:
    only_recompile = ["gui"]

#TODO: do we need -nostartfiles?
args_compile = "-ffreestanding -O0 -Wall -Wextra -Wno-packed-bitfield-compat -fno-exceptions -fno-rtti -std=c++11"
args_link = "-ffreestanding -O0 -nostdlib"


link_files = []

def init_hash():
    global file_hashes
    try:
        file = open(root_build+"hash_cache.txt","rb")
        data_str = file.read()
        file.close()

        file_hashes = pickle.loads(data_str)
    except: #FileNotFoundError #Only in Python 3, apparently
        file_hashes = {}
def deinit_hash():
    data_str = pickle.dumps(file_hashes)
    
    file = open(root_build+"hash_cache.txt","wb")
    file.write(data_str)
    file.close()

def get_will_compile(in_name):
    #http://docs.python.org/2/library/hashlib.html
    file = open(in_name,"rb")
    data_str = file.read()
    file.close()

    file_hash = hashlib.sha224(data_str).hexdigest()

    #Check if we've seen it before in the cache
    try:
        if file_hashes[in_name] == file_hash:
            return False
    #If not, then definitely need to compile
    except KeyError:
        pass

    file_hashes[in_name] = file_hash

    return True
def compile_cpp(in_name,out_name):
    if get_will_compile(in_name):
        print("    Compiling:  \""+in_name+"\"")
        command = ["../../cross/bin/i586-elf-g++","-c",in_name,"-o",out_name]
        command += args_compile.split(" ")
        call(command)
    else:
        print("  ##SKIPPING##: \""+in_name+"\"")
    link_files.append(out_name)
def assemble_asm(in_name,out_name):
    if get_will_compile(in_name):
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
            if   filename.endswith(".cpp"):
                compile_cpp(filename,get_out_name(filename))
            elif filename.endswith(".asm"):
                assemble_asm(filename,get_out_name(filename))
                
        elif os.path.isdir(filename):
            if "old" in filename: continue

            compile_directory(filename)

def link():
    command = ["../../cross/bin/i586-elf-gcc","-T",root_source+"linker.ld","-o",root_build+"MOSS.bin"]
    command += args_link.split(" ")
    for file in link_files:
        command.append(file)
    command.append("-lgcc")
    call(command)

def main():
    init_hash()
    try:
        print("  Compiling:")
        compile_directory(root_source)

        print("  Linking")
        link()
    except KeyboardInterrupt:
        print("  Aborting")
    deinit_hash()

if __name__ == "__main__": main()
























