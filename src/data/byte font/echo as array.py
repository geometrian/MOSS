import pygame

pygame.init()



font8x8 = pygame.image.load("font8x8.png")

#https://web.archive.org/web/20150220013749/www3.telus.net/anapan8/8x16%20font%20ASCII%20DOS%20437.gif
font8x16 = pygame.image.load("font8x16.png")



out = "static uint64_t const font8x8_backing [ 16*16 * 8*8 / sizeof(uint8_t) / sizeof(uint64_t) ] = {"
for     j in range(16):
	for i in range(16):
		data = 0x0000000000000000
		for     j2 in range(8):
			row = 0x00
			for i2 in range(8):
				sample = font8x8.get_at(( i*8+i2, j*8+j2 ))
				if sample[0] != 0:
					row |= 1 << (7-i2)
			data |= row << (j2*8)
		out += "0x%xull" % data
		if not (j == 15 and i == 15): out+=","
out += "};"
print(out)

out = "static uint8_t  const font8x16_backing[ 16*16 * 8*8 / sizeof(uint8_t) / sizeof(uint8_t) ] = {"
for     j in range(16):
	for i in range(16):
		for     j2 in range(16):
			row = 0x00
			for i2 in range( 8):
				sample = font8x16.get_at(( i*8+i2, j*16+j2 ))
				if sample[0] != 0:
					row |= 1 << (7-i2)
			row_str = "0x%x" % row
			if j2 != 15: row_str+=","
			out += row_str
		if not (j == 15 and i == 15): out+=","
out += "};"
print(out)
