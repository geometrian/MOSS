# This makes a *bytemap* font, where every pixel is 1 byte.



import os, sys
import traceback

with open(os.devnull, "w") as f:
	oldstdout=sys.stdout; sys.stdout=f; import pygame; sys.stdout=oldstdout
from pygame.locals import *


if sys.platform in ("win32","win64"): os.environ["SDL_VIDEO_CENTERED"]="1"

pygame.display.init()
pygame.font.init()



#WINDOW_RES = [1024,768]
WINDOW_RES = [500,700]

icon = pygame.Surface((1,1)); icon.set_alpha(0); pygame.display.set_icon(icon)
pygame.display.set_caption("Bytemap Font Creator")

surface = pygame.display.set_mode( WINDOW_RES )



font = pygame.font.SysFont("Lucida Console",12)

pygame.key.set_repeat(500,30)

def rndint(num): return int(round(num))
#def ceilint(num):
#	if num > int(num): return int(num) + 1
#	return num
def clamp( x, lo,hi ):
	if x < lo: return lo
	if x > hi: return hi
	return x

class Character:
	def __init__( self, index, data ):
		self.index = index
		self.data = list(data)

	def draw( self, surf, pos, res ):
		step = [ res[0]/5.0, res[1]/7.0 ]
		#def draw_rect(dim):
		#	for     j in range(ceilint(step[1])):
		#		for i in range(ceilint(step[0])):
		#			surf.set_at( (rndint(dim[0]+i),rndint(dim[1]+j)), (255,255,255) )
		k = 0
		for     j in range(7):
			for i in range(5):
				if self.data[k] != 0:
					#draw_rect([ pos[0]+i*step[0], pos[1]+j*step[0] ])
					if step[0] == 1:
						surf.set_at( (pos[0]+i*step[0],pos[1]+j*step[0]), (255,255,255) )
					else:
						pygame.draw.rect(
							surf, (255,255,255),
							( pos[0]+i*step[0], pos[1]+j*step[0], step[0], step[1] ),
							0
						)
				k += 1
characters = {}



current_index = 90
charset = None

def get_input():
	global current_index, charset

	mouse_buttons = pygame.mouse.get_pressed()
	mouse_position = pygame.mouse.get_pos()
	for event in pygame.event.get():
		if   event.type == QUIT: return False
		elif event.type == KEYDOWN:
			old_index = current_index
			if   event.key == K_ESCAPE: return False
			elif event.key == K_LEFT : current_index-= 1
			elif event.key == K_RIGHT: current_index+= 1
			elif event.key == K_UP   : current_index-=16
			elif event.key == K_DOWN : current_index+=16
			current_index = clamp( current_index, 0,127 )
			if current_index != old_index:
				charset = None

	cell = [ mouse_position[0]*2*5//WINDOW_RES[0], mouse_position[1]*2*7//WINDOW_RES[1] ]
	if cell[0]>=0 and cell[0]<5 and cell[1]>=0 and cell[1]<7:
		if mouse_buttons[0]:
			if characters[current_index].data[cell[1]*5+cell[0]] == 0x00:
				characters[current_index].data[cell[1]*5+cell[0]] = 0xFF
				charset = None
		elif mouse_buttons[2]:
			if characters[current_index].data[cell[1]*5+cell[0]] != 0x00:
				characters[current_index].data[cell[1]*5+cell[0]] = 0x00
				charset = None

	return True

def update_charset():
	global charset
	charset = pygame.Surface(( 25*16, 35*8 ))
	k = 0
	pos = [0,0]
	for     j in range(8):
		pos[1] = j * 35
		for i in range(16):
			pos[0] = i * 25
			characters[k].draw( charset, pos, (25,35) )
			if k == current_index: color=(255,0,0)
			else                 : color=(0,0,255)
			pygame.draw.rect( charset, color, (pos[0],pos[1],25,35), 1 )
			k += 1

def draw():
	surface.fill(( 0, 0, 0 ))

	pygame.draw.rect( surface, (255,0,0), (0,0,WINDOW_RES[0]//2,WINDOW_RES[1]//2), 1 )
	characters[current_index].draw( surface, (0,0), (WINDOW_RES[0]//2,WINDOW_RES[1]//2) )

	characters[current_index].draw( surface, (WINDOW_RES[0]//2+20,100), (5*1,7*1) )
	characters[current_index].draw( surface, (WINDOW_RES[0]//2+50,100), (5*2,7*2) )
	characters[current_index].draw( surface, (WINDOW_RES[0]//2+90,100), (5*3,7*3) )

	s = f"Code point: {current_index} (\""
	if current_index == 0:
		s += "NULL"
	else:
		s += chr(current_index)
	s += "\")"

	surface.blit(
		font.render( s, True, (255,255,255) ),
		( WINDOW_RES[0]//2+10, 10 )
	)

	if charset == None:
		update_charset()
	surface.blit( charset, (10,WINDOW_RES[1]//2+50) )
	
	pygame.display.flip()

def font_read( path ):
	f = open( path, "rb" )
	fontdata = f.read()
	f.close()

	if len(fontdata) != 128 * 7*5:
		fontdata = "\x00"*(128*7*5)

def font_save( path ):
	data = ""
	for k in range(128):
		for b in range(5*7):
			data += chr(characters[k].data[b])

	f = open( path, "wb" )
	f.write( data )
	f.close()

def main():
	try:
		fontdata = font_read( "font.txt" )
	except:
		print( "WARNING: font load failed:" )
		traceback.print_exc()
		fontdata = b"\x00"*(128*7*5)
		print( "Loading new font." )
	print( "WARNING: saving disabled!" ) #commented at bottom of this function

	for k in range(128):
		char = []
		for b in range(5*7):
			sample = fontdata[(5*7)*k+b]
			char.append( sample )
		characters[k] = Character( k, char )

	clock = pygame.time.Clock()
	while True:
		if not get_input():
			break

		draw()

		clock.tick(60)
		#print clock.get_fps()

	#font_save( "font.txt" )

	pygame.quit()

if __name__ == "__main__":
	try:
		main()
	except:
		traceback.print_exc()
		pygame.quit()
		input()
