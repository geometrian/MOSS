from build_print_info import *

def main():
    print_info("Begin build",1)

    import build_bootloader
    build_bootloader.main()

    import build_kernel
    build_kernel.main()

    print_info("All modules built!")

    import build_clean

    print_info("",1)
    print_info("Build complete!",2)
if __name__ == '__main__':
    main()
