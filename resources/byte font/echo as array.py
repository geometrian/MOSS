import pygame
pygame.init()

font8x8 = pygame.image.load("font8x8.png")

#http://www3.telus.net/anapan8/8x16%20font%20ASCII%20DOS%20437.gif
font8x16 = pygame.image.load("font8x16.png")

out = "static uint64_t const  font8x8_backing[16*16 * 8*8 / sizeof(uint8_t) / sizeof(uint64_t)] = {"""
for y in range(16):
    for x in range(16):
        data = 0x0000000000000000
        for y2 in range(8):
            row = 0x00
            for x2 in range(8):
                sample = font8x8.get_at((x*8+x2,y*8+y2))
                if sample[0] != 0:
                    row |= 1 << (7-x2)
            data |= row << (y2*8)
        out += "0x%xULL" % data
        if not (y == 15 and x == 15): out+=","
out += "};"
print(out)

out = "static  uint8_t const font8x16_backing[16*16 * 8*8 / sizeof(uint8_t) / sizeof( uint8_t)] = {"""
for y in range(16):
    for x in range(16):
        for y2 in range(16):
            row = 0x00
            for x2 in range(8):
                sample = font8x16.get_at((x*8+x2,y*16+y2))
                if sample[0] != 0:
                    row |= 1 << (7-x2)
            row_str = "0x%x" % row
            if y2 != 15: row_str+=","
            out += row_str
        if not (y == 15 and x == 15): out+=","
out += "};"
print(out)
