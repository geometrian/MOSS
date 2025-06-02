from inpdev_defs import InpDev_Btn

from ps2_scancodes import sc_set1, sc_set2, sc_set3



class Key:
	def __init__( self, name ):
		self.name = name
		self.ibm_num = None

		self.inpdev_btn = InpDev_Btn.get_by_name( name )

		self.handled = [ False, False, False ]

		self.sc_down = [ None, None, None ]
		self.sc_up   = [ None, None, None ]

	def is_simple( self, ind ):
		if type(self.sc_down[ind]) != type((1,2,3)): return False
		if type(self.sc_up  [ind]) != type((1,2,3)): return False
		return True

	def get_sort_key(self):
		first = self.ibm_num
		if first == None: first=999
		return ( first, self.inpdev_btn.value )

	def print( self, iset ):
		sc_down = str(self.sc_down[iset]).replace(" ","")
		sc_up   = str(self.sc_up  [iset]).replace(" ","")
		print(f"{self.name}:[   down={sc_down},   up={sc_up}   ]")



# Load scancodes, checking we don't have duplicates

known_names = [ [], [], [] ]
for ibm_num, sc, names in sc_set1:
	assert names[0] not in known_names[0]; known_names[0].append(names[0])
for ibm_num, sc, names in sc_set2:
	assert names[0] not in known_names[1]; known_names[1].append(names[0])
for ibm_num, sc, names in sc_set3:
	assert names[0] not in known_names[2]; known_names[2].append(names[0])
scs_names = set( known_names[0] + known_names[1] + known_names[2] )

keys = [ Key(name) for name in scs_names ]

scs_map = {}
for key in keys:
	scs_map[key.name] = key

def parse_sc_seq( sc_seq ):
	if sc_seq == None: return sc_seq
	return tuple([ int(s,16) for s in sc_seq.split(" ") ])
def parse_sc_downup( sc ):
	if type(sc) == type({}):
		down={}; up={}
		for cond, pair in sc.items():
			down[cond] = parse_sc_seq(pair[0])
			up  [cond] = parse_sc_seq(pair[1])
	else:
		down = parse_sc_seq(sc[0])
		up   = parse_sc_seq(sc[1])
	return ( down, up )

for ibm_num, sc, names in sc_set1:
	key = scs_map[names[0]]
	key.ibm_num = ibm_num
	key.sc_down[0], key.sc_up[0] = parse_sc_downup(sc)

for ibm_num, sc, names in sc_set2:
	key = scs_map[names[0]]
	key.ibm_num = ibm_num
	key.sc_down[1], key.sc_up[1] = parse_sc_downup(sc)

for ibm_num, sc, names in sc_set3:
	key = scs_map[names[0]]
	key.ibm_num = ibm_num
	key.sc_down[2], key.sc_up[2] = parse_sc_downup(sc)

keys.sort( key=lambda k:k.get_sort_key() )

#for key in keys: key.print(0)



# Check the scancodes for a property; it is true for sets 2 and 3, except for keys 124 and 126
for key in keys:
	if key.ibm_num==124 or key.ibm_num==126:
		continue

	for iset in ( 1, 2 ):
		if not key.is_simple(iset):
			continue # not implemented

		sc_down = key.sc_down[iset]
		sc_up   = key.sc_up  [iset]

		if sc_down[0] == 0xe0:
			assert sc_up == (0xe0,0xf0) + sc_down[1:]
		else:
			assert sc_up == (0xf0,) + sc_down



# Tables to parse 1-byte codes

INV = -1

table0a = [ INV for k in range(256) ] # 1-byte make in set 1
table0b = [ INV for k in range(256) ] # 1-byte break in set 1
table0c = [ INV for k in range(256) ] # 2-byte make (prefix with 0xe0) in set 1
table0d = [ INV for k in range(256) ] # 2-byte break (prefix with 0xe0) in set 1

table1a  = [ INV for k in range(256) ] # 1-byte make (prefix 0xf0 to get break too) in set 2
table1b  = [ INV for k in range(256) ] # 2-byte make (prefix 0x20, prefix 0xe0 0xf0 to get break too) in set 2

table2  = [ INV for k in range(256) ] # 1-byte make (prefix 0xf0 to get break too) in set 3

def set1_add_codes( key, down,up ):
	if len(down)==1 and len(up)==1:
		assert table0a[down[0]]==INV and table0b[up[0]]==INV
		table0a[down[0]] = key.inpdev_btn.value
		table0b[up  [0]] = key.inpdev_btn.value
		return True
	elif len(down)==2 and len(up)==2 and down[0]==0xe0 and up[0]==0xe0:
		assert table0c[down[1]]==INV and table0d[up[1]]==INV
		table0c[down[1]] = key.inpdev_btn.value
		table0d[up  [1]] = key.inpdev_btn.value
		return True
	return False

def set2_add_codes( key, down,up ):
	if len(down)==1 and up==(0xf0,)+down:
		assert table1a[down[0]] == INV
		table1a[down[0]] = key.inpdev_btn.value
		return True
	elif len(down)==2 and len(up)==3 and down[0]==0xe0 and up[0]==0xe0 and up[1]==0xf0:
		assert table1b[down[1]] == INV
		table1b[down[1]] = key.inpdev_btn.value
		return True
	return False

def set3_add_codes( key, down,up ):
	if len(down)==1 and up==(0xf0,)+down:
		assert table2[down[0]] == INV
		table2[down[0]] = key.inpdev_btn.value
		return True
	return False

for key in keys:
	down=key.sc_down[0]; up=key.sc_up[0]
	if key.is_simple(0):
		key.handled[0] = set1_add_codes( key, down,up )
	else:
		key.handled[0] = True
		for cond in set( list(down.keys()) + list(up.keys()) ):
			if cond in ( "101-only", "102-only" ): continue
			if cond=="Ctrl-or-Shift" and key.name=="Print Screen": continue # is a prefix of other; see C file
			key.handled[0] &= set1_add_codes( key, down[cond],up[cond] )

	down=key.sc_down[1]; up=key.sc_up[1]
	if key.is_simple(1):
		key.handled[1] = set2_add_codes( key, down,up )
	else:
		key.handled[1] = True
		for cond in set( list(down.keys()) + list(up.keys()) ):
			if cond in ( "101-only", "102-only" ): continue
			if cond=="Ctrl-or-Shift" and key.name=="Print Screen": continue
			key.handled[1] &= set2_add_codes( key, down[cond],up[cond] )

	down=key.sc_down[2]; up=key.sc_up[2]
	if down!=None and up!=None:
		assert key.is_simple(2)
		key.handled[2] = set3_add_codes( key, down,up )

for key in keys:
	if key.ibm_num == None:
		continue
	if not key.handled[0]:
		assert (key.ibm_num>=75 and key.ibm_num<=89) or key.ibm_num in (29,42,62,64,95,108,124,126)
	if not key.handled[1]:
		assert (key.ibm_num>=75 and key.ibm_num<=89) or key.ibm_num in (29,42,62,64,95,108,124,126)
	assert key.handled[2]

#for num in unhandled[1]: scancodes[num].print(1)

#print_arr(table2)
