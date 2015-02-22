f = open("font.txt","rb")
fontdata = f.read()
f.close()

out = "static char font[128][5] = {"
for i in xrange(128):
    out += "{"

    #Encode these into 7 bit "byte"s, with MSB=0
    #We don't save anything by using 8-bit bytes; the last five bits of the last byte spill over, so we still get 5 bytes/char
    bits = []
    for j in xrange(5*7):
        bits.append( ord(fontdata[(5*7)*i+j])&0x01 )
##    print bits
    for j in xrange(5):
        byte_bits = bits[7*j:7*j+7]
##        print byte_bits
        byte_bits = [byte_bits[i]<<(7-i-1) for i in xrange(7)]
##        print byte_bits
        byte = 0x00
        for k in byte_bits: byte |= k
##        print byte
        byte_str = "0x%x" % byte
##        print byte_str
##        raw_input()
        
        out += byte_str
        if j != 5-1: out += ","
    
    out += "}"
    if i != 128-1: out += ","
out +="};"

print out
