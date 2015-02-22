from subprocess import call


def main():
    #http://bochs.sourceforge.net/doc/docbook/user/using-bochs.html#COMMANDLINE
    call([
        "C:\\Program Files (x86)\\Bochs-2.6.2\\bochs.exe",
        "-q",
        "-f","C:/dev/C++/MOSS/x86/scripts/bochs-MOSS-win.txt",
        "-log","C:/dev/C++/MOSS/x86/bochs-log.txt"
    ])
if __name__ == "__main__": main()
