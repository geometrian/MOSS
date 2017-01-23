from subprocess import call

from scripts import _paths


def main():
    #http://bochs.sourceforge.net/doc/docbook/user/using-bochs.html#COMMANDLINE
    call([
        _paths.bochs,
        "-q",
        "-f",_paths.bochsdbg_config,
        "-log",_paths.bochs_log
    ])
if __name__ == "__main__": main()
