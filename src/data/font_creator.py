import math
import os
import struct
import sys
import tkinter as tk
import tkinter.commondialog
import tkinter.messagebox
import tkinter.filedialog
import traceback
from typing import Self

with open(os.devnull, "w") as f:
	oldstdout=sys.stdout; sys.stdout=f; import pygame; sys.stdout=oldstdout



#WINDOW_RES = [ 1024, 768 ]
WINDOW_RES = [ 500, 700 ]



def rndint( num:float ): return int(round(num))

def clamp( x:int|float, lo:int|float,hi:int|float ):
	if x < lo: return lo
	if x > hi: return hi
	return x

def next_power_of_2( x:int ): return 1 if x==0 else (1<<(x-1).bit_length())

def ask_font_config( with_codepoints ):
	win = tk.Toplevel( root )
	win.title("Specify Font Parameters")
	win.resizable( False, False )
	win.grab_set()

	if with_codepoints:
		lbl_n = tk.Label( win, text="Number of codepoints:  " )
	lbl_w     = tk.Label( win, text="Glyph Width [px]:  "     )
	lbl_h     = tk.Label( win, text="Glyph Height [px]:  "    )
	if with_codepoints:
		lbl_n.grid( row=0, column=0 )
		lbl_w.grid( row=1, column=0 )
		lbl_h.grid( row=2, column=0 )
	else:
		lbl_w.grid( row=0, column=0 )
		lbl_h.grid( row=1, column=0 )

	if with_codepoints:
		n_var = tk.IntVar( value=128 )
	w_var     = tk.IntVar( value=  8 )
	h_var     = tk.IntVar( value= 16 )

	if with_codepoints:
		n = tk.Entry( win, textvariable=n_var )
	w     = tk.Entry( win, textvariable=w_var )
	h     = tk.Entry( win, textvariable=h_var )
	w.focus_set()
	if with_codepoints:
		n.grid( row=0, column=1 )
		w.grid( row=1, column=1 )
		h.grid( row=2, column=1 )
	else:
		w.grid( row=0, column=1 )
		h.grid( row=1, column=1 )

	codepoints = gly_w = gly_h = -1
	def end_dialog():
		nonlocal codepoints, gly_w,gly_h
		def get_validated( var, defl ):
			try:
				val = var.get()
				if val <= 0: raise ValueError()
				return val
			except:
				return defl
		if with_codepoints:
			codepoints = get_validated( n_var, -1 )
		gly_w          = get_validated( w_var, -1 )
		gly_h          = get_validated( h_var, -1 )
		win.destroy()
	sub_btn = tk.Button( win, text="Submit", command=end_dialog )
	if with_codepoints: sub_btn.grid( row=3, column=0, columnspan=2 )
	else              : sub_btn.grid( row=2, column=0, columnspan=2 )

	root.wait_window( win )

	if with_codepoints and codepoints==-1:
		return False
	if gly_w==-1 or gly_h==-1:
		return False

	if with_codepoints: return ( codepoints, gly_w,gly_h )
	else              : return (             gly_w,gly_h )



class Glyph:
	def __init__( self:Self, codepoint:int, res:tuple[int,int], data:list[bool]=[] ):
		self.codepoint = codepoint
		self.res = tuple(res)
		if data == []:
			self._data = [ False for b in range(res[1]*res[0]) ]
		else:
			self._data = list(data)

	def __len__( self:Self ): return len(self._data)

	def __getitem__( self:Self, ind:int           ): return self._data[ind]
	def __setitem__( self:Self, ind:int, val:bool ): self._data[ind]=val

	def set_data( self:Self, data:list[bool] ):
		self._data = list(data)

	def edit_clear( self:Self ): self._data=[ False for b in self._data ]
	def edit_fill ( self:Self ): self._data=[ True  for b in self._data ]
	def edit_neg  ( self:Self ): self._data=[ not b for b in self._data ]

	def to_bitstring( self:Self, pad_rows:bool=True, pad_end:bool=True ):
		if pad_rows: padding=(8-self.res[0]%8)%8
		else       : padding=0

		bits = ""
		ib = 0
		for     j in range( self.res[1] ):
			for i in range( self.res[0] ):
				if self._data[ib]: bits+="1"
				else             : bits+="0"
				ib += 1
			if pad_rows:
				bits += "0" * padding
		if pad_end:
			bits += "0" * ( (8-len(bits)%8) % 8 )

		return bits

	def to_bytes( self:Self, pad_rows:bool=True ):
		bits = self.to_bitstring( pad_rows )
		return bytes([
			int(bits[ 8*ibyte : (ibyte+1)*8 ],2)   for ibyte in range( len(bits) // 8 )
		])

	def draw( self:Self, dstsurf:pygame.Surface, pos:tuple[int,int], res:tuple[int,int] ):
		scale = [ res[0]/self.res[0], res[1]/self.res[1] ]
		#def draw_rect(dim):
		#	for     j in range(ceilint(scale[1])):
		#		for i in range(ceilint(scale[0])):
		#			dstsurf.set_at( (rndint(dim[0]+i),rndint(dim[1]+j)), (255,255,255) )
		b = 0
		for     j in range( self.res[1] ):
			for i in range( self.res[0] ):
				if self._data[b]: color=(255,255,255)
				else            : color=(  0,  0,  0)
				#draw_rect([ pos[0]+i*scale[0], pos[1]+j*scale[0] ])
				if scale[0] == 1:
					dstsurf.set_at( (pos[0]+i*scale[0],pos[1]+j*scale[0]), color )
				else:
					pygame.draw.rect(
						dstsurf, color,
						( pos[0]+i*scale[0],pos[1]+j*scale[0], scale[0],scale[1] ),
						0
					)
				b += 1

class Font:
	def __init__( self:Self, num_codepoints:int, glyph_res:tuple[int,int], cols_rows:tuple[int,int]|None=None ):
		self.glyph_res = tuple(glyph_res)
		self.glyphs = [ Glyph(cp,glyph_res) for cp in range(num_codepoints) ]

	@staticmethod
	def load_psf( path:str ):
		# https://en.wikipedia.org/wiki/PC_Screen_Font

		# Read file data
		f = open( path, "rb" )
		data = f.read()
		f.close()
		ldata = len(data)

		# Parse header
		if ldata>=4 and data[:2]==b"\x36\x04": # psf1
			flags1 = data[2]
			if flags1 & 0b00000001: num_codepoints=512
			else                  : num_codepoints=256
			if flags1 & 0b00000110: has_uni_table=True
			else                  : has_uni_table=False
			bytes_per_glyph = data[3]
			gly_w = 8
			gly_h = bytes_per_glyph
			data = data[4:]
		elif ldata>=16 and data[:4]==b"\x72\xb5\x4a\x86": #psf2
			ver, hdrsz, flags4, num_codepoints, bytes_per_glyph, gly_h,gly_w = struct.unpack(
				"<IIIIIII", data[4:4+7*4]
			)
			if flags4 & 0x00000001: has_uni_table=True
			else                  : has_uni_table=False
			data = data[hdrsz:]
		else:
			raise ValueError("Unrecognized PSF file type!")

		# Parse data into `Font` object
		font = Font( num_codepoints, [gly_w,gly_h] )
		padding = ( 8 - gly_w%8 ) % 8
		for cp in range( num_codepoints ):
			gly_bytes = data[ cp*bytes_per_glyph : (cp+1)*bytes_per_glyph ]
			gly_bits = "".join([ f"{gly_byte:08b}" for gly_byte in gly_bytes ])

			glyph = Glyph( cp, [gly_w,gly_h] )
			isrc=0; idst=0
			for     j in range( gly_h ):
				for i in range( gly_w ):
					if gly_bits[isrc]=="1": glyph[idst]=True
					isrc+=1; idst+=1
				isrc += padding
			font.glyphs[cp] = glyph

		if has_uni_table:
			print("WARNING: Skipping unicode table---not implemented!")

		return font

	@staticmethod
	def load_image( path:str, glyph_res:tuple[int,int] ):
		surf = pygame.image.load( path )

		img_w, img_h = surf.get_size()
		gly_w, gly_h = glyph_res
		if img_w%gly_w != 0 or img_h%gly_h != 0:
			raise ValueError(
				f"Glyph resolution {gly_w}⨯{gly_h} does not divide image resolution {img_w}⨯{img_h}!"
			)
		num_h = img_w // gly_w
		num_w = img_h // gly_h
		num_codepoints = num_h * num_w

		font = Font( num_codepoints, [ gly_w, gly_h ])

		cp = 0
		for     j in range( num_h ):
			for i in range( num_w ):
				glyph = Glyph( cp, (gly_w,gly_h) )
				for     jj in range( gly_h ):
					for ii in range( gly_w ):
						sample = surf.get_at(( i*gly_w+ii, j*gly_h+jj ))
						if sample[0] >= 128:
							glyph[ jj*gly_w + ii ] = True
				font.glyphs[cp] = glyph
				cp += 1

		return font

	@staticmethod
	def load_bytemap( path:str, num_codepoints:int=128, glyph_res:tuple[int,int]=(5,7) ):
		f = open( path, "rb" )
		data = f.read()
		f.close()

		bytes_per_glyph = glyph_res[1] * glyph_res[0]
		if len(data) != num_codepoints * bytes_per_glyph:
			raise ValueError(
				f"Could not interpret file as bytemap font; unexpected size for {num_codepoints} "+
				f"codepoints and {glyph_res[0]}⨯{glyph_res[1]} glyph resolution!"
			)

		font = Font( num_codepoints, glyph_res )
		for cp in range( num_codepoints ):
			gly_data = [ bool(val) for val in data[ cp*bytes_per_glyph : (cp+1)*bytes_per_glyph ] ]
			font.glyphs[cp].set_data(gly_data)

		return font

	@staticmethod
	def save_psf2( font:Self, path:str ):
		padding = ( 8 - font.glyph_res[0]%8 )%8
		bytes_per_glyph = font.glyph_res[1] * (font.glyph_res[0] + padding)//8

		f = open( path, "wb" )
		f.write(struct.pack( "<IIIIIIII",
			0x864ab572, 0, 32, 0x00000000,
			len(font.glyphs), bytes_per_glyph, font.glyph_res[1],font.glyph_res[0]
		))
		for glyph in font.glyphs:
			f.write( glyph.to_bytes() )
		f.close()

	@staticmethod
	def save_image( font:Self, path:str, cols_rows:tuple[int,int]|None=None ):
		num_codepoints = len( font.glyphs )
		if cols_rows == None:
			if   num_codepoints == 128:
				cols=16; rows= 8
			elif num_codepoints == 256:
				cols=16; rows=16
			elif num_codepoints == 512:
				cols=16; rows=32
			else:
				cols = next_power_of_2(int( math.sqrt(num_codepoints) ))
				rows = int(math.ceil( num_codepoints / cols ))
		else:
			cols, rows = cols_rows
		gly_w, gly_h = font.glyph_res
		img_w = cols * gly_w
		img_h = rows * gly_h

		surf = pygame.Surface(( img_w, img_h ))
		cp = 0
		for     irow in range( rows ):
			for icol in range( cols ):
				if cp >= num_codepoints: break
				font.glyphs[cp].draw( surf, (icol*gly_w,irow*gly_h), (gly_w,gly_h) )
				cp += 1

		pygame.image.save( surf, path )

	@staticmethod
	def save_bytemap( font:Self, path:str ):
		f = open( path, "wb" )
		for glyph in font.glyphs:
			bits = glyph.to_bitstring( False, False )
			data = bytes([ 0xFF*(ord(b)-ord('0')) for b in bits ])
			f.write( data )
		f.close()

	@staticmethod
	def save_c_array( font:Self, path:str, gly_pad_rows:bool=True, cols:int=100,tabcols:int=4 ):
		byte_strs = [ glyph.to_bytes(gly_pad_rows) for glyph in font.glyphs ]

		strs = []
		bytes_per_glyph = len( byte_strs[0] )
		elem_count = 0
		for byte_str in byte_strs:
			s = ""
			if   bytes_per_glyph%8 == 0:
				for indbyte0 in range( 0, bytes_per_glyph, 8 ):
					s += ",0x"
					for indbytei in range( 8 ):
						s += f"{byte_str[indbyte0+indbytei]:02x}"
					s += "ull"
					elem_count += 1
			elif bytes_per_glyph%4 == 0:
				for indbyte0 in range( 0, bytes_per_glyph, 4 ):
					s += ",0x"
					for indbytei in range( 4 ):
						s += f"{byte_str[indbyte0+indbytei]:02x}"
					s += "u"
					elem_count += 1
			elif bytes_per_glyph%2 == 0:
				for indbyte0 in range( 0, bytes_per_glyph, 2 ):
					s += ",0x"
					for indbytei in range( 2 ):
						s += f"{byte_str[indbyte0+indbytei]:02x}"
					elem_count += 1
			else:
				for indbytei in range( bytes_per_glyph ):
					s += f",0x{byte_str[indbytei]:02x}"
					elem_count += 1
			strs.append(s[1:])
		del byte_strs

		gly_w, gly_h = font.glyph_res

		f = open( path, "w" )

		s = f"/* bitmap font, {len(font.glyphs)} {gly_w}x{gly_h} glyphs, "
		if gly_pad_rows: s+=f"{(8-gly_w%8)%8} padding bits per row"
		else           : s+="each bitwise packed"
		s += " */\n"
		f.write(s)

		s = "static "
		if   bytes_per_glyph%8 == 0: s+="uint64_t"
		elif bytes_per_glyph%4 == 0: s+="uint32_t"
		elif bytes_per_glyph%2 == 0: s+="uint16_t"
		else                       : s+="uint8_t"
		s += f" const font{gly_w}x{gly_h}_data[{elem_count}] ="

		if cols == -1:
			s += " { "
			s += ", ".join(strs)
			s += " };\n"
			f.write(s)

		else:
			f.write(s+"\n{\n")
			del s

			line     = "\t"
			line_len = tabcols
			for istr,s in enumerate(strs):
				s += ", "
				if line_len + len(s) > cols:
					f.write(line[:-1]+"\n")
					line     = "\t"
					line_len =  tabcols
				line     +=     s
				line_len += len(s)
			if line != "\t":
				f.write(line[:-2]+"\n")
			f.write("};\n")

		f.close()

	def edit_clear( self ):
		for glyph in self.glyphs: glyph.edit_clear()
	def edit_fill ( self ):
		for glyph in self.glyphs: glyph.edit_fill ()
	def edit_neg  ( self ):
		for glyph in self.glyphs: glyph.edit_neg  ()

class FontEditor:
	def __init__( self:Self ):
		self._font = Font( 128, (5,7) )
		self.curr_ind:int = 90

		self.zoom:int = 2

		self.saved:bool = True
		self.save_path:str = "(new)" #"(new)", or path

		self.resize()

	def update_title( self:Self ):
		name = self.save_path
		if not self.saved:
			name = "*" + name
		root.title( name + " - Font Creator" )

	def set_font( self:Self, font:Font, save_path:str|None=None ):
		self._font = font
		self.curr_ind = min(( len(font.glyphs)-1, self.curr_ind ))

		if save_path != None: self.save_path=save_path
		else                : self.save_path="(new)"
		self.saved = save_path != None

		self.resize()

		self.update_title()

	def resize( self:Self, cols_rows:tuple[int,int]|None=None ):
		gly_w, gly_h = self._font.glyph_res
		gly_w *= self.zoom
		gly_h *= self.zoom

		num_codepoints = len( self._font.glyphs )
		if cols_rows == None:
			self.num_cols = (WINDOW_RES[0]-10-10) // gly_w
			self.num_rows = int(math.ceil( num_codepoints / self.num_cols ))
		else:
			self.num_cols, self.num_rows = cols_rows
		img_w = self.num_cols * gly_w
		img_h = self.num_rows * gly_h

		self.surf = pygame.Surface(( img_w, img_h ))
		for cp in range( num_codepoints ):
			self.update_glyph(cp)

	def _get_cell( self:Self, mpos:tuple[int,int] ):
		gly_w, gly_h = self._font.glyph_res
		edit_h = WINDOW_RES[1] // 2
		edit_w = edit_h * gly_w // gly_h

		cell = ( mpos[0]*gly_w//edit_w, mpos[1]*gly_h//edit_h )
		if cell[0]>=0 and cell[0]<gly_w and cell[1]>=0 and cell[1]<gly_h:
			return cell[1]*gly_w + cell[0]

		return None

	def _select( self:Self, codepoint:int ):
		if   codepoint <  0                     : codepoint=0
		elif codepoint >= len(self._font.glyphs): codepoint=len(self._font.glyphs)-1
		if self.curr_ind == codepoint: return
		old_ind = self.curr_ind
		self.curr_ind = codepoint
		self.update_glyph(old_ind  )
		self.update_glyph(codepoint)

	def save( self:Self ):
		ext = os.path.splitext( self.save_path )[1]
		if   ext == ".psf":
			Font.save_psf2   ( self._font, self.save_path )
		elif ext == ".txt":
			Font.save_bytemap( self._font, self.save_path )
		elif ext in ( ".c", ".cpp" ):
			Font.save_c_array( self._font, self.save_path )
		else:
			Font.save_image  ( self._font, self.save_path )
		self.saved = True
		self.update_title()

	def on_file_new   ( self:Self ):
		if self.save_as_needed_get_should_abort():
			return False

		ret = ask_font_config( True )
		if ret == False: return ret
		codepoints, gly_w,gly_h = ret

		self.set_font(Font( codepoints, (gly_w,gly_h) ))

		return True
	def on_file_open  ( self:Self ):
		if self.save_as_needed_get_should_abort():
			return False

		path = tk.filedialog.askopenfilename( filetypes=[
			( "All Files", "*.*" ),
			( "PC Screen Font", "*.psf" ),
			( "Bytemap (1B/b)", "*.txt" ),
			( "Bitmap Image", "*.bmp *.dib" ),
			( "GIF Image", "*.gif" ),
			( "JPEG Image", "*.jpg *.jpeg" ),
			( "LBM Image", "*.lbm" ),
			( "Netpbm Image", "*.pam *.pbm *pgm *ppm *.pnm" ),
			( "PCX Image", "*.pcx" ),
			( "PNG Image", "*.png" ),
			( "QOI Image", "*.qoi" ),
			( "TGA Image", "*.tga *.icb *.vda *.vst" ),
			( "TIFF Image", "*.tif *.tiff" ),
			( "WebP Image", "*.webp" ),
			( "X PixMap Image", "*.xpm" ),
			( "XCF Image", "*.xcf" )
		] )
		if path:
			ext = os.path.splitext( path )[1]
			if   ext == ".psf":
				font = Font.load_psf( path )
			elif ext == ".txt":
				ret = ask_font_config( True )
				if ret == False: return ret
				codepoints, gly_w,gly_h = ret

				try:
					font = Font.load_bytemap( path, codepoints, (gly_w,gly_h) )
				except ValueError as err:
					tk.messagebox.showerror( "Invalid Parameters", str(err) )
					return False
			else:
				ret = ask_font_config( False )
				if ret == False: return ret
				gly_w, gly_h = ret

				try:
					font = Font.load_image( path, (gly_w,gly_h) )
				except ValueError as err:
					tk.messagebox.showerror( "Invalid Parameters", str(err) )
					return False
			self.set_font( font, path )
			return True

		return False
	def on_file_save  ( self:Self ):
		if self.save_path == "(new)": return self.on_file_saveas()
		self.save()
		return True
	def on_file_saveas( self:Self ):
		path = tk.filedialog.asksaveasfilename( filetypes=[
			( "All Files", "*.*" ),
			( "PC Screen Font", "*.psf" ),
			( "C / C++ Array", "*.c *.cpp" ),
			( "Bytemap (1B/b)", "*.txt" ),
			( "Bitmap Image", "*.bmp *.dib" ),
			( "JPEG Image", "*.jpg *.jpeg" ),
			( "PNG Image", "*.png" ),
			( "TGA Image", "*.tga *.icb *.vda *.vst" )
		] )
		if path:
			self.save_path = path
			self.save()
			return True
		return False
	def on_file_exit  ( self:Self ):
		global looping
		if self.save_as_needed_get_should_abort():
			return False
		looping = False
		return True

	def _on_edit_one( self:Self ):
		self.update_glyph()
		self.saved = False
		self.update_title()
	def _on_edit_all( self:Self ):
		self.resize()
		self.saved = False
		self.update_title()
	def on_edit_clear   ( self:Self ):
		self._font.glyphs[self.curr_ind].edit_clear()
		self._on_edit_one()
	def on_edit_clearall( self:Self ):
		self._font.edit_clear()
		self._on_edit_all()
	def on_edit_neg     ( self:Self ):
		self._font.glyphs[self.curr_ind].edit_neg()
		self._on_edit_one()
	def on_edit_negall  ( self:Self ):
		self._font.edit_neg()
		self._on_edit_all()

	def save_as_needed_get_should_abort( self:Self ):
		if not self.saved:
			ret = tk.messagebox.askyesnocancel(
				icon = tk.messagebox.WARNING,
				title="Unsaved Work", message="Some edits have not yet been saved.  Save work?",
			)
			if   ret == None : return True  # Cancel
			elif ret == False: return False # No
			else             :              # Yes
				if not self.on_file_save():
					return True
		return False

	def on_lclick( self:Self, mpos:tuple[int,int] ):
		cell = self._get_cell( mpos )
		if cell != None:
			self._font.glyphs[self.curr_ind][cell] = True
			self.update_glyph()
			self.saved = False
			self.update_title()
			return

		x = mpos[0] - 10
		y = mpos[1] - (WINDOW_RES[1]//2+50)
		gly_w, gly_h = self._font.glyph_res
		col = x // (gly_w*self.zoom)
		row = y // (gly_h*self.zoom)
		if row < 0: return
		self._select( row*self.num_cols + col )

	def on_rclick( self:Self, mpos:tuple[int,int] ):
		cell = self._get_cell( mpos )
		if cell != None:
			self._font.glyphs[self.curr_ind][cell] = False
			self.update_glyph()
			self.saved = False
			self.update_title()

	def on_left ( self:Self ): self._select( self.curr_ind - 1 )
	def on_right( self:Self ): self._select( self.curr_ind + 1 )
	def on_up   ( self:Self ): self._select( self.curr_ind - self.num_cols )
	def on_down ( self:Self ): self._select( self.curr_ind + self.num_cols )
	def on_home ( self:Self ): self._select( 0 )
	def on_end  ( self:Self ): self._select( len(self._font.glyphs) - 1 )

	def on_wheel( self:Self, evt ):
		self.zoom += evt.delta // 120
		self.zoom = clamp( self.zoom, 1,64 )
		self.resize()

	def update_glyph( self:Self, codepoint:int=-1 ):
		if codepoint == -1:
			codepoint = self.curr_ind

		gly_w, gly_h = self._font.glyph_res
		gly_w *= self.zoom
		gly_h *= self.zoom

		icol = codepoint %  self.num_cols
		irow = codepoint // self.num_cols

		pos = ( icol*gly_w, irow*gly_h )

		self._font.glyphs[codepoint].draw( self.surf, pos, (gly_w,gly_h) )

		if codepoint == self.curr_ind: color=(255,0,0)
		else                         : color=(0,0,255)
		pygame.draw.rect( self.surf, color, (pos[0],pos[1],gly_w,gly_h), 1 )

	def draw( self:Self ):
		glyph = self._font.glyphs[self.curr_ind]
		gly_w, gly_h = self._font.glyph_res

		edit_h = WINDOW_RES[1] // 2
		edit_w = edit_h * gly_w // gly_h

		glyph.draw( surf_display, (0,0), (edit_w,edit_h) )
		pygame.draw.rect( surf_display, (255,0,0), (0,0,edit_w,edit_h), 2 )

		def _draw_zoom_preview( zoom, x ):
			glyph.draw( surf_display, (x,100), (gly_w*zoom,gly_h*zoom) )
		_draw_zoom_preview( 1, edit_w+ 20 )
		_draw_zoom_preview( 2, edit_w+ 50 )
		_draw_zoom_preview( 3, edit_w+ 90 )
		_draw_zoom_preview( 4, edit_w+140 )
		_draw_zoom_preview( 5, edit_w+200 )

		s = f"Code point: {self.curr_ind}"
		ch = chr(self.curr_ind)
		if   ch.isprintable():
			name = f"'{ch}'"
		elif self.curr_ind>=0 and self.curr_ind<=31:
			name = (
				"NULL", "Start Heading", "Start Text", "End Text", "End Transmission", "ENQ", "ACK",
				"'\\a' Bell", "Backspace", "\\t", "\\n", "Vertical Tab", "Form Feed", "\\r",
				"Shift Out", "Shift In", "Data Esc", "Dev Ctrl 1", "Dev Ctrl 2", "Dev Ctrl 3",
				"Dev Ctrl 4", "NAK", "SYN", "End Block", "Cancel", "End Medium", "Sub", "Esc \\e",
				"File Sep", "Group Sep", "Record Sep", "Unit Sep"
			)[self.curr_ind]
		elif self.curr_ind == 127:
			name = "Delete"
		else:
			name = ""
		if len(name) > 0:
			s += f" ({name})"
		surf_display.blit(
			ui_font.render( s, True, (255,255,255) ),
			( edit_w+10, 10 )
		)

		surf_display.blit( self.surf, (10,edit_h+50) )

def on_save():
	return False
def get_mouse_input():
	mouse_buttons  = pygame.mouse.get_pressed()
	mouse_position = pygame.mouse.get_pos()
	if mouse_buttons[0]:
		editor.on_lclick( mouse_position )
	if mouse_buttons[2]:
		editor.on_rclick( mouse_position )

def draw():
	global editor, root, surf_display

	surf_display.fill(( 0, 0, 0 ))

	editor.draw()

	pygame.display.flip()
	root.update()

def main():
	global editor, looping, root, surf_display, ui_font

	editor = FontEditor()

	looping = True

	root = tk.Tk()
	root.title("Font Creator")

	menu_bar = tk.Menu( root )

	menu_file = tk.Menu( menu_bar, tearoff=0 )
	menu_file.add_command( label="New"       , accelerator="Ctrl+N"      , command=editor.on_file_new    )
	menu_file.add_command( label="Open..."   , accelerator="Ctrl+O"      , command=editor.on_file_open   )
	menu_file.add_command( label="Save"      , accelerator="Ctrl+S"      , command=editor.on_file_save   )
	menu_file.add_command( label="Save As...", accelerator="Ctrl+Shift+S", command=editor.on_file_saveas )
	menu_file.add_separator()
	menu_file.add_command( label="Exit"      , accelerator="Alt+F4"      , command=editor.on_file_exit   )
	menu_bar.add_cascade( menu=menu_file, label="File" )

	menu_edit = tk.Menu( menu_bar, tearoff=0 )
	menu_edit.add_command( label="Clear"         , command=editor.on_edit_clear    )
	menu_edit.add_command( label="Clear (All)"   , command=editor.on_edit_clearall )
	menu_edit.add_command( label="Negative"      , command=editor.on_edit_neg      )
	menu_edit.add_command( label="Negative (All)", command=editor.on_edit_negall   )
	menu_bar.add_cascade( menu=menu_edit, label="Edit" )

	root.config( menu=menu_bar )

	frame = tk.Frame( root, width=WINDOW_RES[0],height=WINDOW_RES[1] )
	frame.pack()

	os.environ["SDL_WINDOWID"] = str( frame.winfo_id() )

	pygame.display.init()
	pygame.font.init()

	surf_display = pygame.display.set_mode( WINDOW_RES )

	pygame.key.set_repeat( 500, 30 )

	ui_font = pygame.font.SysFont("Lucida Console",12)

	root.update()

	root.bind( "<Escape>", lambda evt: editor.on_file_exit() )
	root.protocol( "WM_DELETE_WINDOW", lambda: editor.on_file_exit() )
	root.bind( "<Control-n>"      , lambda evt: editor.on_file_new   () )
	root.bind( "<Control-o>"      , lambda evt: editor.on_file_open  () )
	root.bind( "<Control-s>"      , lambda evt: editor.on_file_save  () )
	root.bind( "<Control-Shift-S>", lambda evt: editor.on_file_saveas() )
	root.bind( "<Alt-F4>", lambda evt: editor.on_file_exit() )
	root.bind( "<Left>" , lambda evt: editor.on_left () )
	root.bind( "<Right>", lambda evt: editor.on_right() )
	root.bind( "<Up>"   , lambda evt: editor.on_up   () )
	root.bind( "<Down>" , lambda evt: editor.on_down () )
	root.bind( "<Home>" , lambda evt: editor.on_home () )
	root.bind( "<End>"  , lambda evt: editor.on_end  () )
	root.bind( "<MouseWheel>", editor.on_wheel )
	def on_resize( evt ):
		global WINDOW_RES, surf_display
		if evt.width==WINDOW_RES[0] and evt.height==WINDOW_RES[1]: return
		WINDOW_RES = [ evt.width, evt.height ]
		frame.pack( fill="both", expand=1 )
		surf_display = pygame.display.set_mode( WINDOW_RES )
		editor.resize()
	root.bind( "<Configure>", on_resize )

	path = "src/data/fonts/font0_bytemap.txt"
	try:
		editor.set_font(Font.load_bytemap( path, 128, (5,7) ),path)
	except:
		pass

	clock = pygame.time.Clock()
	while looping:
		get_mouse_input()
		draw()

		clock.tick(60)
		#print clock.get_fps()

	pygame.quit()
	root.destroy()

if __name__ == "__main__":
	try:
		main()
	except:
		traceback.print_exc()
		pygame.quit()
		input()
