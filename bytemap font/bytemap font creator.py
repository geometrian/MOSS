import pygame
from pygame.locals import *
import sys, os, traceback
if sys.platform == 'win32' or sys.platform == 'win64':
    os.environ['SDL_VIDEO_CENTERED'] = '1'
pygame.display.init()
pygame.font.init()

screen_size = [500,700]
icon = pygame.Surface((1,1)); icon.set_alpha(0); pygame.display.set_icon(icon)
pygame.display.set_caption("Bytemap Font Creator - Ian Mallett - v.1.0.0 - 2012")
surface = pygame.display.set_mode(screen_size)

font = pygame.font.SysFont("Lucida Console",12)

pygame.key.set_repeat(500,30)

##def rndint(num): return int(round(num))
##def ceilint(num):
##    if num > int(num): return int(num) + 1
##    return num
class Character:
    def __init__(self,index,data):
        self.index = index
        self.data = list(data)
    def draw(self,surf,pos,res):
        step = [res[0]/5.0,res[1]/7.0]
##        def draw_rect(dim):
##            for y in xrange(ceilint(step[1])):
##                for x in xrange(ceilint(step[0])):
##                    surf.set_at((rndint(dim[0]+x),rndint(dim[1]+y)),(255,255,255))
        i = 0
        for y in xrange(7):
            for x in xrange(5):
                if self.data[i] != 0:
##                    draw_rect([pos[0]+x*step[0],pos[1]+y*step[0]])
                    if step[0]==1:
                        surf.set_at((pos[0]+x*step[0],pos[1]+y*step[0]),(255,255,255))
                    else:
                        pygame.draw.rect(surf,(255,255,255),(pos[0]+x*step[0],pos[1]+y*step[0],step[0],step[1]),0)
                i += 1
characters = {}

current_index = 90
charset = None
def get_input():
    global current_index, charset
    key = pygame.key.get_pressed()
    mpress = pygame.mouse.get_pressed()
    mpos = pygame.mouse.get_pos()
    for event in pygame.event.get():
        if   event.type == QUIT: return False
        elif event.type == KEYDOWN:
            if   event.key == K_ESCAPE: return False
            elif event.key == K_LEFT:
                if current_index > 0: current_index -= 1
            elif event.key == K_RIGHT:
                if current_index < 127: current_index += 1
    cell = [mpos[0]*2*5/screen_size[0],mpos[1]*2*7/screen_size[1]]
    if cell[0]>=0 and cell[0]<5 and cell[1]>=0 and cell[1]<7:
        if mpress[0]:
            if characters[current_index].data[cell[1]*5+cell[0]] == 0x00:
                characters[current_index].data[cell[1]*5+cell[0]] = 0xFF
                charset = None
        elif mpress[2]:
            if characters[current_index].data[cell[1]*5+cell[0]] != 0x00:
                characters[current_index].data[cell[1]*5+cell[0]] = 0x00
                charset = None
    return True
def update_charset():
    global charset
    charset = pygame.Surface((25*16,35*8))
    i = 0
    pos = [0,0]
    for y in xrange(8):
        pos[1] = y * 35
        for x in xrange(16):
            pos[0] = x * 25
            characters[i].draw(charset,pos,(25,35))
            pygame.draw.rect(charset,(0,0,255),(pos[0],pos[1],25,35),1)
            i += 1
def draw():
    surface.fill((0,0,0))

    pygame.draw.rect(surface,(255,0,0),(0,0,screen_size[0]/2,screen_size[1]/2),1)
    characters[current_index].draw(surface,(0,0),(screen_size[0]/2,screen_size[1]/2))

    characters[current_index].draw(surface,(screen_size[0]/2+20,100),(5*1,7*1))
    characters[current_index].draw(surface,(screen_size[0]/2+50,100),(5*2,7*2))
    characters[current_index].draw(surface,(screen_size[0]/2+90,100),(5*3,7*3))

    s = "Code point: "+str(current_index)+" (\""
    if current_index == 0:
        s += "NULL"
    else:
        s += chr(current_index)
    s += "\")"

    surface.blit(
        font.render(s,True,(255,255,255)),
        (screen_size[0]/2+10,10)
    )

    if charset == None:
        update_charset()
    surface.blit(charset,(10,screen_size[1]/2+50))
    
    pygame.display.flip()
def main():
    try:
        f = open("font.txt","rb")
        fontdata = f.read()
        f.close()
        if len(fontdata)!=128*7*5:
            fontdata = "\x00"*(128*7*5)
    except:
        fontdata = "\x00"*(128*7*5)

    for i in xrange(128):
        char = []
        for j in xrange(5*7):
            char.append(ord(fontdata[(5*7)*i+j]))
        characters[i] = Character(i,char)
    
    clock = pygame.time.Clock()
    while True:
        if not get_input(): break
        draw()
        clock.tick(60)
##        print clock.get_fps()

    data = ""
    for i in xrange(128):
        for j in xrange(5*7):
            data += chr(characters[i].data[j])

    f = open("font.txt","wb")
    fontdata = f.write(data)
    f.close()
    
    pygame.quit(); sys.exit()
if __name__ == '__main__':
    try:
        main()
    except Exception, e:
        tb = sys.exc_info()[2]
        traceback.print_exception(e.__class__, e, tb)
        pygame.quit()
        raw_input()
        sys.exit()

