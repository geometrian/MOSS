import hashlib
import os
try:    import cPickle as pickle #Only present in Python 2.*; Python 3 automatically imports the
except: import  pickle as pickle #new equivalent of cPickle, if it's available.
from subprocess import call


#Directories
def get_abs_path(rel_path):
    return os.path.abspath(rel_path).replace("\\","/") + "/"
def get_abs_path_from(directory,rel_path):
    return os.path.normpath(directory+rel_path).replace("\\","/")

root = get_abs_path("")#"../")
root_build  = root+".build/"
root_source = root+"source_moss/"

#Commands and Arguments
#   TODO: do we need -nostartfiles?
args_compile = "-ffreestanding   -O0   -Wall -Wextra -Wno-packed-bitfield-compat   -fstack-protector-all -fno-exceptions -fno-rtti   -std=c++11"
args_link = "-ffreestanding   -O0   -nostdlib"

##command_gcc = get_abs_path_from(root,"../../cross/bin/i686-elf-gcc")
##command_gpp = get_abs_path_from(root,"../../cross/bin/i686-elf-g++")
command_gcc = os.path.expanduser("~/cross/bin/i686-elf-gcc")
command_gpp = os.path.expanduser("~/cross/bin/i686-elf-g++")

command_nasm = "nasm"

#Types
class FileBase(object):
    TYPE_HEADER = 0
    TYPE_SOURCE = 1
    def __init__(self, directory,name, type):
        self.directory = directory
        self.name = name
        self.path = directory + name
        self.type = type
        
        self.changed = None
        file=open(self.path,"rb"); data_str=file.read(); file.close()
        self.hash = hashlib.sha224(data_str).hexdigest() #http://docs.python.org/2/library/hashlib.html

        self.user_includes = []
    def add_includes_to(self, file):
        if self not in file.user_includes:
            file.user_includes.append(self)
        for include in self.user_includes:
            include.add_includes_to(file)

class FileHeader(FileBase):
    def __init__(self, directory,name):
        FileBase.__init__(self, directory,name, FileBase.TYPE_HEADER)

class FileSource(FileBase): #and ASM
    def __init__(self, directory,name):
        FileBase.__init__(self, directory,name, FileBase.TYPE_SOURCE)

        self.needs_compile = None

#Enumerate files to build system recursively
files = []
def add_files_directory(directory):
    for name in os.listdir(directory):
        path = directory + name #os.path.join(directory,name)
        if os.path.isfile(path):
            t = None                    
            if path.endswith(".cpp") or path.endswith(".asm"): files.append(FileSource(directory,name))
            else:                                              files.append(FileHeader(directory,name)) #Headers (with or without extension)
        elif os.path.isdir(path):
            add_files_directory(path+"/")
add_files_directory(root_source)

#Figure out which have changed
for file in files:
    file.changed = True #Assume the worst
if os.path.exists(root_build+"_cache.txt"):
    file=open(root_build+"_cache.txt","rb"); data_str=file.read(); file.close()
    file_hashes = pickle.loads(data_str) #map of path onto hash
    for file in files:
        if file.path in file_hashes.keys():
            if file.hash == file_hashes[file.path]:
                file.changed = False

#Figure out which need to be recompiled.  This is every changed source file, plus every source file
#   that includes a changed header.
changed_some_headers = False
for file in files:
    if file.changed and file.type==FileBase.TYPE_HEADER:
        changed_some_headers = True
        break
if changed_some_headers: #optimization
    for file in files:
        fobj=open(file.path,"r"); file_data=fobj.read(); fobj.close()

        for line in file_data.split("\n"):
            line2 = line.strip().split("//")[0]
            if "#include" in line2:
                included_rel = line2.split("#include")[1]
                i=0; j=len(included_rel)-1
                while not (included_rel[i]=="\"" or included_rel[i]=="<"): i+=1
                while not (included_rel[j]=="\"" or included_rel[j]==">"): j-=1
                included_rel = included_rel[i:j+1]

                if included_rel[0] == "<": continue #Not a user include; assume it can't change

                included_abs = get_abs_path_from(file.directory,included_rel[1:-1])
                found = False
                for file2 in files:
                    if file2.path == included_abs:
                        found = True
                        break
                assert found, "Could not find \"#include\"d file \"%s\"!" % included_abs
                file.user_includes.append(file2)
    for file in files:
        for include in file.user_includes:
            include.add_includes_to(file)
for file in files:
    file.needs_compile = False
    if file.type == FileBase.TYPE_SOURCE:
        if file.changed:
            file.needs_compile = True
        else:
            for include in file.user_includes:
                if include.changed:
                    file.needs_compile = True
                    break

#Compile everything that needs compiling
link_files = []
def run(command):
    #print(command)
    call(command)
def get_arg_list(arg_str):
    l = arg_str.split(" ")
    l2 = []
    for a in l:
        if a=="": continue
        l2.append(a)
    return l2
def get_create_out_path(file):
    out_path = root_build + file.path[len(root_source):]+"_obj"
    out_dir = os.path.dirname(out_path)
    if not os.path.exists(out_dir):
        os.makedirs(out_dir)
    return out_path
def compile_cpp(file,out_path):
    print("    Compiling:  \""+file.path+"\"")
    command = [command_gpp,"-c",file.path,"-o",out_path]
    command += get_arg_list(args_compile)
    run(command)
def assemble_asm(file,out_path):
    print("    Assembling: \""+file.path+"\"")
    command = [command_nasm,"-felf",file.path,"-o",out_path]
    run(command)
def link():
    command = [command_gcc,"-T",root_source+"linker.ld","-o",root_build+"MOSS.bin"]
    command += get_arg_list(args_link)
    for file in link_files: command.append(file)
    command.append("-lgcc")
    run(command)
try:
    print("  Compiling:")
    skipped = 0
    for file in files:
        out_path = get_create_out_path(file)
        if file.needs_compile:
            if   file.name.endswith(".cpp"):  compile_cpp(file,out_path)
            elif file.name.endswith(".asm"): assemble_asm(file,out_path)
            else: assert False
            file.needs_compile = False
        else:
            skipped += 1
        if file.type == FileBase.TYPE_SOURCE:
            link_files.append(out_path)
    if skipped > 0:
        print("  Skipped %d files" % (skipped))

    print("  Linking")
    link()
except KeyboardInterrupt:
    print("  Aborting")

#Save compiled cache
file_hashes = {}
for file in files:
    if not file.needs_compile:
        file_hashes[file.path] = file.hash
data_str = pickle.dumps(file_hashes)
file=open(root_build+"_cache.txt","wb"); file.write(data_str); file.close()
























