import sys, time

def print_info(text="",extra_newlines=0):
    if text != "":
        print "############## "+text+" ##############"
    for i in xrange(extra_newlines): print
    sys.stdout.flush()