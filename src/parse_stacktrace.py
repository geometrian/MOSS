#Use k from VirtualBox debug terminal, copy-paste into window on run

#MAP_FILE_PATH = ".build/kernel/moss.map"
MAP_FILE_PATH = "D:/projects/programming/C++/MOSS/.build/kernel/moss.map"



import tkinter as tk



root = tk.Tk()
root.title("Stack Trace Converter")

lbl_dir = tk.Label( root,
	text="Get stacktrace from VirtualBox (cmd \"k\") and copy-paste here:"
)

textbox_in = tk.Text( root, width=85,height=15 )

lbl_out = tk.Label( root,
	text="Output:"
)

textbox_out = tk.Text( root, width=85,height=30 )

lbl_dir.pack()
textbox_in.pack()
lbl_out.pack()
textbox_out.pack()

textbox_out.tag_config( "style-background", background="#FFF",foreground="#aaa" )
textbox_out.tag_config( "style-symbol", background="#FFF",foreground="#007" )

root.bind( "<Escape>", lambda evt: root.destroy() )
root.bind( "<Alt-F4>", lambda evt: root.destroy() )



def load_symbols():
	global syms

	sym_file = open( MAP_FILE_PATH, "r" )
	sym_data = sym_file.read()
	sym_file.close()

	syms = []
	for line in sym_data.split("\n"):
		line = list(filter( lambda s:len(s)>0, line.split(" ") ))
		if len(line) != 5: continue
		vma, lma, size, align, symname = line
		addr = int( lma, 16 )
		syms.append(( addr, symname ))

def callback( evt ):
	# Load the symbols each time, since the user probably re-built the program and is trying again.
	load_symbols()

	# This weird index means "row 1, col 0" (with row being 1-indexed and col being 0-indexed).  I
	# don't make the rules.
	inp = textbox_in.get( "1.0", tk.END )

	textbox_out.tag_remove( "style-symbol", "1.0",tk.END )
	textbox_out.delete( "1.0", tk.END )

	#"#  RBP              Ret SS:RBP            Ret RIP          CS:RIP / Symbol [line]"
	#"00 fffffffffffffb70 0010:fffffffffffffc50 ffffffffffe04cf1 0008:ffffffffffe04c4c"
	#"01 fffffffffffffc50 0010:fffffffffffffd50 ffffffffffe0bd21 0008:ffffffffffe04cf1"
	#"02 fffffffffffffd50 0010:fffffffffffffe30 ffffffffffe0b486 0008:ffffffffffe0bd21"
	#"03 fffffffffffffe30 0010:fffffffffffffe80 ffffffffffe0b8d9 0008:ffffffffffe0b486"
	#"04 fffffffffffffe80 0010:ffffffffffffff20 ffffffffffe04d65 0008:ffffffffffe0b8d9"
	#"05 ffffffffffffff20 0010:ffffffffffffffe0 ffffffffffe048d6 0008:ffffffffffe04d65"
	#"06 ffffffffffffffe0 0010:fffffffffffffff0 ffffffffffe043eb 0008:ffffffffffe048d6"
	#"07 fffffffffffffff0 0010:0000000000000000 0000000000000000 0008:ffffffffffe043eb"
	#"08 0000000000000000 0010:0000000120000000 000000000000000d 0008:0000000000000000"
	#"09 0000000120000000 0010:0000000000000000 0000000000000000 0008:000000000000000d"

	#"ffffffffffe02a60 ffffffffffe02a60      49f    16         ../.build/kernel/draw_font.o:(.text)"
	#"ffffffffffe02a60 ffffffffffe02a60      187     1                 draw_load_font_ptr"

	rows = [ row.split(" ") for row in inp.split("\n") ]

	frames = []
	for row in rows:
		if len(row) != 5: continue
		num, rbp, ssrbp, rip, csrip = row

		#num = int( num )
		rbp = int( ssrbp.split(":")[1], 16 )
		rip = int( csrip.split(":")[1], 16 )

		frames.append(( rbp, rip ))

		if rbp == 0: break

	locs_syms = []
	lines = [ "Traceback (most recent call last):" ]
	for i, frame in enumerate(reversed(frames)):
		rbp, rip = frame
		sym = "?"
		for addr, symname in syms:
			if addr<=rip: sym=symname
			else: break

		line = f"  {i+1:2d}: sym="
		R = len(lines) + 1
		C0 = len(line)
		line += sym
		C1 = len(line)
		locs_syms.append(( R, C0, C1 ))
		lines.append(line)
		lines.append(f"      rbp={rbp:016x}, rip={rip:016x}")

	lines.append("      (current)")

	textbox_out.insert( tk.END, "\n".join(lines) )
	textbox_out.tag_add( "style-background", "1.0",tk.END )
	for R,C0,C1 in locs_syms:
		textbox_out.tag_add( "style-symbol", f"{R}.{C0}",f"{R}.{C1}" )

textbox_in.bind( "<KeyRelease>", callback )



tk.mainloop()



