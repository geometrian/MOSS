from subprocess import call

from scripts import _paths


def main():
    call([
        _paths.qemu32,
        "-hda",_paths.vmdk
    ])
if __name__ == "__main__": main()
