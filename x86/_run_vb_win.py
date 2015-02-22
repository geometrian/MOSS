from subprocess import call

def main():
    call([
        "C:\\Program Files\\Oracle\\VirtualBox\\VBoxManage.exe",
        "startvm","MOSS"
    ])
if __name__ == "__main__": main()
