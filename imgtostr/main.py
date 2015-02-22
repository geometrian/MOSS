import pygame
from pygame.locals import *

img = pygame.image.load("sunspirenight.png")

out = ""

def make():
    global out
    for y in range(img.get_height()):
        for x in range(img.get_width()):
            col = img.get_at((x,y))
            out += "%X%X%X" % (col[0],col[1],col[2])

make()
#input(out)

file = open("out.txt","w")
file.write(out)
file.close()
