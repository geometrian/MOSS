f = open( "font.txt", "rb" )
fontdata = f.read()
f.close()

out_str = "static char const font[128][5] = {"
for i in range(128):
	out_str += "{"

	# Encode these into 7 bit "byte"s, with MSB=0
	# We don't save anything by using 8-bit bytes; the last five bits of the last byte spill over,
	# so we still get 5 bytes/char

	bits = []
	for j in range(5*7):
		bits.append( ord(fontdata[(5*7)*i+j]) & 0b1 )
	#print( bits )

	for j in range(5):
		byte_bits = bits[ 7*j : 7*j+7 ]
		#print( byte_bits )
		byte_bits = [ byte_bits[i]<<(7-i-1) for i in range(7) ]
		#print( byte_bits )

		byte = 0x00
		for b in byte_bits: byte|=b
		#print( byte )

		byte_str = "0x%x" % byte
		#print( byte_str )

		#input()

		out_str += byte_str
		if j != 5-1: out_str+=","
	
	out_str += "}"
	if i != 128-1: out_str+=","
out_str +="};"

print( out_str )
