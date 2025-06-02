import pygame



img = pygame.image.load("sunspirenight.png")
w, h = img.get_size()

out = ""
for     j in range(h):
    for i in range(w):
        col = img.get_at((i,j))
        out += "%X%X%X" % (col[0],col[1],col[2])
    print(f"{j+1}/{h}")
#input(out)

file = open( "out.txt", "wb" )
file.write(out)
file.close()

pygame.quit()
