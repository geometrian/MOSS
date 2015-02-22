import pygame
pygame.init()
font = pygame.image.load("font.png")

out = "//256 characters, each 8x8 pixel-bits, packed into a single 8-byte long long (uint64_t) each\nstatic uint64_t font[256 * 8*8 / 8] = {"
for y in range(16):
    for x in range(16):
        long_long = 0x0000000000000000
        i = 0
        for y2 in range(8):
            for x2 in range(8):
                sample = font.get_at((x*8+x2,y*8+y2))
                if sample[0]!=0:
                    long_long |= 0x1 << (64-i-1)
                i += 1
        long_long_str = "0x%x" % long_long
        out += long_long_str + "ULL"
        if not (y == 15 and x == 15): out += ","
    
out += "};"

print(out)
