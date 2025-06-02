"""
There are three scancode sets for PS/2.

The ultimate source for this is the "Hardware Interface Technical Reference" for "Keyboard 101- and
102-Key" (see [1], from parent site [2] with lots of great info).

I had to parse this document myself, because everyone else did it badly.  The best I found was [3],
but this still has a rather critical error---the pause key's break code doesn't exist.  Other
sources aren't better.  E.g. [4] is commonly cited, but has issues---for example, the scancode for
'i' in set 3 has a copy-paste or OCR error.  Also the names and formatting are clunky.  [5] is
basically impossibly messy, though has some neat links.

[1]: https://www.ardent-tool.com/docs/pdf/ibm_hitrc11.pdf
     Scan codes pg. 27--36, (31--40 in PDF)
     Key numbers pg. 2f (6f)
[2]: https://www.ardent-tool.com/docs/pdfs.html
[3]: https://www.win.tue.nl/~aeb/linux/kbd/scancodes-10.html#ss10.6
[4]: https://web.archive.org/web/20180204214719/http://www.computer-engineering.org/ps2keyboard/scancodes1.html
     https://web.archive.org/web/20180202062621/http://www.computer-engineering.org/ps2keyboard/scancodes2.html
     https://web.archive.org/web/20180208084611/http://www.computer-engineering.org/ps2keyboard/scancodes3.html
[5]: http://www.quadibloc.com/comp/scan.htm

More stuff:
	https://www.win.tue.nl/~aeb/linux/kbd/scancodes-10.html
	http://www.brokenthorn.com/Resources/OSDevScanCodes.html
"""

_tmp = [
	# Num    Set 1 Make/Break     Set 2 Make/Break        Set3 Make/Break   US-QWERTY Name
	(   1,   ("29"   ,"a9"   ),   ("0e"   ,"f0 0e"   ),   ("0e","f0 0e"),   ("`"           ,"~" ) ),
	(   2,   ("02"   ,"82"   ),   ("16"   ,"f0 16"   ),   ("16","f0 16"),   ("1"           ,"!" ) ),
	(   3,   ("03"   ,"83"   ),   ("1e"   ,"f0 1e"   ),   ("1e","f0 1e"),   ("2"           ,"@" ) ),
	(   4,   ("04"   ,"84"   ),   ("26"   ,"f0 26"   ),   ("26","f0 26"),   ("3"           ,"#" ) ),
	(   5,   ("05"   ,"85"   ),   ("25"   ,"f0 25"   ),   ("25","f0 25"),   ("4"           ,"$" ) ),
	(   6,   ("06"   ,"86"   ),   ("2e"   ,"f0 2e"   ),   ("2e","f0 2e"),   ("5"           ,"%" ) ),
	(   7,   ("07"   ,"87"   ),   ("36"   ,"f0 36"   ),   ("36","f0 36"),   ("6"           ,"^" ) ),
	(   8,   ("08"   ,"88"   ),   ("3d"   ,"f0 3d"   ),   ("3d","f0 3d"),   ("7"           ,"&" ) ),
	(   9,   ("09"   ,"89"   ),   ("3e"   ,"f0 3e"   ),   ("3e","f0 3e"),   ("8"           ,"*" ) ),
	(  10,   ("0a"   ,"8a"   ),   ("46"   ,"f0 46"   ),   ("46","f0 46"),   ("9"           ,"(" ) ),
	(  11,   ("0b"   ,"8b"   ),   ("45"   ,"f0 45"   ),   ("45","f0 45"),   ("0"           ,")" ) ),
	(  12,   ("0c"   ,"8c"   ),   ("4e"   ,"f0 4e"   ),   ("4e","f0 4e"),   ("-"           ,"_" ) ),
	(  13,   ("0d"   ,"8d"   ),   ("55"   ,"f0 55"   ),   ("55","f0 55"),   ("="           ,"+" ) ),
	# (14 reserved)
	(  15,   ("0e"   ,"8e"   ),   ("66"   ,"f0 66"   ),   ("66","f0 66"),   ("Backspace"   ,None) ),
	(  16,   ("0f"   ,"8f"   ),   ("0d"   ,"f0 0d"   ),   ("0d","f0 0d"),   ("Tab"         ,None) ),
	(  17,   ("10"   ,"90"   ),   ("15"   ,"f0 15"   ),   ("15","f0 15"),   ("q"           ,"Q" ) ),
	(  18,   ("11"   ,"91"   ),   ("1d"   ,"f0 1d"   ),   ("1d","f0 1d"),   ("w"           ,"W" ) ),
	(  19,   ("12"   ,"92"   ),   ("24"   ,"f0 24"   ),   ("24","f0 24"),   ("e"           ,"E" ) ),
	(  20,   ("13"   ,"93"   ),   ("2d"   ,"f0 2d"   ),   ("2d","f0 2d"),   ("r"           ,"R" ) ),
	(  21,   ("14"   ,"94"   ),   ("2c"   ,"f0 2c"   ),   ("2c","f0 2c"),   ("t"           ,"T" ) ),
	(  22,   ("15"   ,"95"   ),   ("35"   ,"f0 35"   ),   ("35","f0 35"),   ("y"           ,"Y" ) ),
	(  23,   ("16"   ,"96"   ),   ("3c"   ,"f0 3c"   ),   ("3c","f0 3c"),   ("u"           ,"U" ) ),
	(  24,   ("17"   ,"97"   ),   ("43"   ,"f0 43"   ),   ("43","f0 43"),   ("i"           ,"I" ) ),
	(  25,   ("18"   ,"98"   ),   ("44"   ,"f0 44"   ),   ("44","f0 44"),   ("o"           ,"O" ) ),
	(  26,   ("19"   ,"99"   ),   ("4d"   ,"f0 4d"   ),   ("4d","f0 4d"),   ("p"           ,"P" ) ),
	(  27,   ("1a"   ,"9a"   ),   ("54"   ,"f0 54"   ),   ("54","f0 54"),   ("["           ,"{" ) ),
	(  28,   ("1b"   ,"9b"   ),   ("5b"   ,"f0 5b"   ),   ("5b","f0 5b"),   ("]"           ,"}" ) ),
	# (29 alternates with 42 according to the keyboard physical layout (see below))
	(  30,   ("3a"   ,"ba"   ),   ("58"   ,"f0 58"   ),   ("14","f0 14"),   ("Caps Lock"   ,None) ),
	(  31,   ("1e"   ,"9e"   ),   ("1c"   ,"f0 1c"   ),   ("1c","f0 1c"),   ("a"           ,"A" ) ),
	(  32,   ("1f"   ,"9f"   ),   ("1b"   ,"f0 1b"   ),   ("1b","f0 1b"),   ("s"           ,"S" ) ),
	(  33,   ("20"   ,"a0"   ),   ("23"   ,"f0 23"   ),   ("23","f0 23"),   ("d"           ,"D" ) ),
	(  34,   ("21"   ,"a1"   ),   ("2b"   ,"f0 2b"   ),   ("2b","f0 2b"),   ("f"           ,"F" ) ),
	(  35,   ("22"   ,"a2"   ),   ("34"   ,"f0 34"   ),   ("34","f0 34"),   ("g"           ,"G" ) ),
	(  36,   ("23"   ,"a3"   ),   ("33"   ,"f0 33"   ),   ("33","f0 33"),   ("h"           ,"H" ) ),
	(  37,   ("24"   ,"a4"   ),   ("3b"   ,"f0 3b"   ),   ("3b","f0 3b"),   ("j"           ,"J" ) ),
	(  38,   ("25"   ,"a5"   ),   ("42"   ,"f0 42"   ),   ("42","f0 42"),   ("k"           ,"K" ) ),
	(  39,   ("26"   ,"a6"   ),   ("4b"   ,"f0 4b"   ),   ("4b","f0 4b"),   ("l"           ,"L" ) ),
	(  40,   ("27"   ,"a7"   ),   ("4c"   ,"f0 4c"   ),   ("4c","f0 4c"),   (";"           ,":" ) ),
	(  41,   ("28"   ,"a8"   ),   ("52"   ,"f0 52"   ),   ("52","f0 52"),   ("'"           ,"\"") ),
	# (42 alternates with 29 according to the keyboard physical layout (see below))
	(  43,   ("1c"   ,"9c"   ),   ("5a"   ,"f0 5a"   ),   ("5a","f0 5a"),   ("Return"      ,None) ),
	(  44,   ("2a"   ,"aa"   ),   ("12"   ,"f0 12"   ),   ("12","f0 12"),   ("Left Shift"  ,None) ),
	(  45,   ("56"   ,"d6"   ),   ("61"   ,"f0 61"   ),   ("13","f0 13"),   ("Other33"     ,None) ), # E.g. U.K. '\\'/'|'
	(  46,   ("2c"   ,"ac"   ),   ("1a"   ,"f0 1a"   ),   ("1a","f0 1a"),   ("z"           ,"Z" ) ),
	(  47,   ("2d"   ,"ad"   ),   ("22"   ,"f0 22"   ),   ("22","f0 22"),   ("x"           ,"X" ) ),
	(  48,   ("2e"   ,"ae"   ),   ("21"   ,"f0 21"   ),   ("21","f0 21"),   ("c"           ,"C" ) ),
	(  49,   ("2f"   ,"af"   ),   ("2a"   ,"f0 2a"   ),   ("2a","f0 2a"),   ("v"           ,"V" ) ),
	(  50,   ("30"   ,"b0"   ),   ("32"   ,"f0 32"   ),   ("32","f0 32"),   ("b"           ,"B" ) ),
	(  51,   ("31"   ,"b1"   ),   ("31"   ,"f0 31"   ),   ("31","f0 31"),   ("n"           ,"N" ) ),
	(  52,   ("32"   ,"b2"   ),   ("3a"   ,"f0 3a"   ),   ("3a","f0 3a"),   ("m"           ,"M" ) ),
	(  53,   ("33"   ,"b3"   ),   ("41"   ,"f0 41"   ),   ("41","f0 41"),   (","           ,"<" ) ),
	(  54,   ("34"   ,"b4"   ),   ("49"   ,"f0 49"   ),   ("49","f0 49"),   ("."           ,">" ) ),
	(  55,   ("35"   ,"b5"   ),   ("4a"   ,"f0 4a"   ),   ("4a","f0 4a"),   ("/"           ,"?" ) ),
	# (56 reserved)
	(  57,   ("36"   ,"b6"   ),   ("59"   ,"f0 59"   ),   ("59","f0 59"),   ("Right Shift" ,None) ),
	(  58,   ("1d"   ,"9d"   ),   ("14"   ,"f0 14"   ),   ("11","f0 11"),   ("Left Ctrl"   ,None) ),
	# (59 reserved)
	(  60,   ("38"   ,"b8"   ),   ("11"   ,"f0 11"   ),   ("19","f0 19"),   ("Left Alt"    ,None) ),
	(  61,   ("39"   ,"b9"   ),   ("29"   ,"f0 29"   ),   ("29","f0 29"),   ("Spacebar"    ,None) ),
	(  62,   ("e0 38","e0 b8"),   ("e0 11","e0 f0 11"),   ("39","f0 39"),   ("Alt Gr"      ,None) ),
	# (63 reserved)
	(  64,   ("e0 1d","e0 9d"),   ("e0 14","e0 f0 14"),   ("58","f0 58"),   ("Right Ctrl"  ,None) ),
	# (65--74 reserved)
	# (75--89 have different forms depending on the states of Shift and Num Lk (see below))
	(  90,   ("45"   ,"c5"   ),   ("77"   ,"f0 77"   ),   ("76","f0 76"),   ("Num Lock"    ,None) ),
	(  91,   ("47"   ,"c7"   ),   ("6c"   ,"f0 6c"   ),   ("6c","f0 6c"),   ("Numpad 7"    ,None) ),
	(  92,   ("4b"   ,"cb"   ),   ("6b"   ,"f0 6b"   ),   ("6b","f0 6b"),   ("Numpad 4"    ,None) ),
	(  93,   ("4f"   ,"cf"   ),   ("69"   ,"f0 69"   ),   ("69","f0 69"),   ("Numpad 1"    ,None) ),
	# (94 reserved)
	# (95 has different forms depending on the states of Shift (see below))
	(  96,   ("48"   ,"c8"   ),   ("75"   ,"f0 75"   ),   ("75","f0 75"),   ("Numpad 8"    ,None) ),
	(  97,   ("4c"   ,"cc"   ),   ("73"   ,"f0 73"   ),   ("73","f0 73"),   ("Numpad 5"    ,None) ),
	(  98,   ("50"   ,"d0"   ),   ("72"   ,"f0 72"   ),   ("72","f0 72"),   ("Numpad 2"    ,None) ),
	(  99,   ("52"   ,"d2"   ),   ("70"   ,"f0 70"   ),   ("70","f0 70"),   ("Numpad 0"    ,None) ),
	( 100,   ("37"   ,"b7"   ),   ("7c"   ,"f0 7c"   ),   ("7e","f0 7e"),   ("Numpad *"    ,None) ),
	( 101,   ("49"   ,"c9"   ),   ("7d"   ,"f0 7d"   ),   ("7d","f0 7d"),   ("Numpad 9"    ,None) ),
	( 102,   ("4d"   ,"cd"   ),   ("74"   ,"f0 74"   ),   ("74","f0 74"),   ("Numpad 6"    ,None) ),
	( 103,   ("51"   ,"d1"   ),   ("7a"   ,"f0 7a"   ),   ("7a","f0 7a"),   ("Numpad 3"    ,None) ),
	( 104,   ("53"   ,"d3"   ),   ("71"   ,"f0 71"   ),   ("71","f0 71"),   ("Numpad ."    ,None) ),
	( 105,   ("4a"   ,"ca"   ),   ("7b"   ,"f0 7b"   ),   ("84","f0 84"),   ("Numpad -"    ,None) ),
	( 106,   ("4e"   ,"ce"   ),   ("79"   ,"f0 79"   ),   ("7c","f0 7c"),   ("Numpad +"    ,None) ),
	# (107 reserved)
	( 108,   ("e0 1c","e0 9c"),   ("e0 5a","e0 f0 5a"),   ("79","f0 79"),   ("Numpad Enter",None) ),
	# (109 reserved)
	( 110,   ("01"   ,"81"   ),   ("76"   ,"f0 76"   ),   ("08","f0 08"),   ("Escape"      ,None) ),
	# (111 reserved)
	( 112,   ("3b"   ,"bb"   ),   ("05"   ,"f0 05"   ),   ("07","f0 07"),   ("F1"          ,None) ),
	( 113,   ("3c"   ,"bc"   ),   ("06"   ,"f0 06"   ),   ("0f","f0 0f"),   ("F2"          ,None) ),
	( 114,   ("3d"   ,"bd"   ),   ("04"   ,"f0 04"   ),   ("17","f0 17"),   ("F3"          ,None) ),
	( 115,   ("3e"   ,"be"   ),   ("0c"   ,"f0 0c"   ),   ("1f","f0 1f"),   ("F4"          ,None) ),
	( 116,   ("3f"   ,"bf"   ),   ("03"   ,"f0 03"   ),   ("27","f0 27"),   ("F5"          ,None) ),
	( 117,   ("40"   ,"c0"   ),   ("0b"   ,"f0 0b"   ),   ("2f","f0 2f"),   ("F6"          ,None) ),
	( 118,   ("41"   ,"c1"   ),   ("83"   ,"f0 83"   ),   ("37","f0 37"),   ("F7"          ,None) ),
	( 119,   ("42"   ,"c2"   ),   ("0a"   ,"f0 0a"   ),   ("3f","f0 3f"),   ("F8"          ,None) ),
	( 120,   ("43"   ,"c3"   ),   ("01"   ,"f0 01"   ),   ("47","f0 47"),   ("F9"          ,None) ),
	( 121,   ("44"   ,"c4"   ),   ("09"   ,"f0 09"   ),   ("4f","f0 4f"),   ("F10"         ,None) ),
	( 122,   ("57"   ,"d7"   ),   ("78"   ,"f0 78"   ),   ("56","f0 56"),   ("F11"         ,None) ),
	( 123,   ("58"   ,"d8"   ),   ("07"   ,"f0 07"   ),   ("5e","f0 5e"),   ("F12"         ,None) ),
	# (124 has different forms depending on the states of Ctrl, Shift, and Alt (see below))
	( 125,   ("46"   ,"c6"   ),   ("7e"   ,"f0 7e"   ),   ("5f","f0 5f"),   ("Scroll Lock" ,None) )
	# (126 has different forms depending on the states of Ctrl (see below))
]
sc_set1 = []
sc_set2 = []
sc_set3 = []
for num, set1,set2,set3, names in _tmp:
	sc_set1.append(( num, set1, names ))
	sc_set2.append(( num, set2, names ))
	sc_set3.append(( num, set3, names ))



# Keys 29 and 42
# On a 101-key keyboard (US-style), 42 doesn't exist.  On a 102-key keyboard, 29 doesn't exist.  In
# sets 1 and 2, they have the same scancodes, though they are distinct in set 3

sc_set1.append(( 29,   { "101-only":("2b","ab"   ) },   ("\\"     ,"|" ) ))
sc_set1.append(( 42,   { "102-only":("2b","ab"   ) },   ("Other76",None) )) # E.g. U.K. '#'/'~'

sc_set2.append(( 29,   { "101-only":("5d","f0 5d") },   ("\\"     ,"|" ) ))
sc_set2.append(( 42,   { "102-only":("5d","f0 5d") },   ("Other76",None) ))

sc_set3.append(( 29,                ("5c","f0 5c")  ,   ("\\"     ,"|" ) ))
sc_set3.append(( 42,                ("53","f0 53")  ,   ("Other76",None) ))



# Keys 75--89, 95
# In sets 1 and 2, keys 75--89 prefix / suffix their keys according to Shift and NumLk, while in set
# 3 nothing changes.  There are notes in the spec. which I interpret (and experiment to support) it
# working like this:
#
#	if which_set == 1:
#		if (LShift or RShift) and NumLk: pass
#		elif NumLk:
#			make_code  = "e0 2a" + make_code
#			break_code = break_code + "e0 aa"
#		else:
#			if RShift:
#				make_code  = "e0 b6" + make_code
#			if LShift:
#				make_code  = "e0 aa" + make_code
#				break_code = break_code + "e0 2a"
#			if RShift:
#				break_code = break_code + "e0 36"
#	elif which_set == 2:
#		if (LShift or RShift) and NumLk: pass
#		elif NumLk:
#			make_code  = "e0 12" + make_code
#			break_code = break_code + "e0 f0 12"
#		else:
#			if RShift:
#				make_code  = "e0 f0 59" + make_code
#			if LShift:
#				make_code  = "e0 f0 12" + make_code
#				break_code = break_code + "e0 12"
#			if RShift:
#				break_code = break_code + "e0 59"
#	elif which_set == 3:
#		pass
#
# Key 95 works the same way, except without being affected by Num Lock.

_tmp = [
	# Num   Set 1 Make/Break     Set 2 Make/Break        Set3 Make/Break   US-QWERTY Name
	( 75,   ("e0 52","e0 d2"),   ("e0 70","e0 f0 70"),   ("67","f0 67"),   ("Insert"     ,None) ),
	( 76,   ("e0 53","e0 d3"),   ("e0 71","e0 f0 71"),   ("64","f0 64"),   ("Delete"     ,None) ),
	# (77--78 reserved)
	( 79,   ("e0 4b","e0 cb"),   ("e0 6b","e0 f0 6b"),   ("61","f0 61"),   ("Left Arrow" ,None) ),
	( 80,   ("e0 47","e0 c7"),   ("e0 6c","e0 f0 6c"),   ("6e","f0 6e"),   ("Home"       ,None) ),
	( 81,   ("e0 4f","e0 cf"),   ("e0 69","e0 f0 69"),   ("65","f0 65"),   ("End"        ,None) ),
	# (82 reserved)
	( 83,   ("e0 48","e0 c8"),   ("e0 75","e0 f0 75"),   ("63","f0 63"),   ("Up Arrow"   ,None) ),
	( 84,   ("e0 50","e0 d0"),   ("e0 72","e0 f0 72"),   ("60","f0 60"),   ("Down Arrow" ,None) ),
	( 85,   ("e0 49","e0 c9"),   ("e0 7d","e0 f0 7d"),   ("6f","f0 6f"),   ("Page Up"    ,None) ),
	( 86,   ("e0 51","e0 d1"),   ("e0 7a","e0 f0 7a"),   ("6d","f0 6d"),   ("Page Down"  ,None) ),
	# (87--88 reserved)
	( 89,   ("e0 4d","e0 cd"),   ("e0 74","e0 f0 74"),   ("6a","f0 6a"),   ("Right Arrow",None) )
]
for num, set1,set2,set3, names in _tmp:
	sc_set1.append(( num, { "NumLk-Shift-Prefix-Suffix":set1 }, names ))
	sc_set2.append(( num, { "NumLk-Shift-Prefix-Suffix":set2 }, names ))
	sc_set3.append(( num,                               set3  , names ))

sc_set1.append(( 95, { "Shift-Prefix-Suffix":("e0 35","e0 b5"   ) }, ("Numpad /",None) ))
sc_set2.append(( 95, { "Shift-Prefix-Suffix":("e0 4a","e0 f0 4a") }, ("Numpad /",None) ))
sc_set3.append(( 95,                         ("77"   ,"f0 77"   )  , ("Numpad /",None) ))



# Key 124
# In sets 1 and 2, this modifies by a Ctrl or Shift, or by an Alt.  In set 3, it doesn't change.

sc_set1.append(( 124, {
	"Base"         :("e0 2a e0 37","e0 b7 e0 aa"      ),
	"Ctrl-or-Shift":("e0 37"      ,"e0 b7"            ),
	"Alt"          :("54"         ,"d4"               )
}, ("Print Screen",None) ))

sc_set2.append(( 124, {
	"Base"         :("e0 12 e0 7c","e0 f0 7c e0 f0 12"),
	"Ctrl-or-Shift":("e0 7c"      ,"e0 f0 7c"         ),
	"Alt"          :("84"         ,"f0 84"            )
}, ("Print Screen",None) ))

sc_set3.append(( 124, ("57","f0 57"), ("Print Screen",None) ))



# Key 126
# In sets 1 and 2, this modifies by a Ctrl.  Note that sets 1 and 2 don't have break codes.  In set
# 3, it doesn't change, and there is a break code.

sc_set1.append(( 126, {
	"Base":("e1 1d 45 e1 9d c5"      ,None),
	"Ctrl":("e0 46 e0 c6"            ,None)
}, ("Pause / Break",None) ))

sc_set2.append(( 126, {
	"Base":("e1 14 77 e1 f0 14 f0 77",None),
	"Ctrl":("e0 7e e0 f0 7e"         ,None)
}, ("Pause / Break",None) ))

sc_set3.append(( 126, ("62","f0 62"), ("Pause / Break",None) ))



# Additional keys have been added over the years by non-canonical sources.  These were discovered
# experimentally.

sc_set2.append(( None,   ("e0 1f","e0 f0 1f"),   ("Left OS Key",None) ))
sc_set2.append(( None,   ("e0 2f","e0 f0 2f"),   ("Menu"       ,None) ))



# Adam Chapweske had lists of scancodes, mentioned above.  Although these contain errors as noted,
# they have some additional codes we can support.  He probably discovered those experimentally, or
# got them out of some crufty Microsoft documentation.
#	https://web.archive.org/web/20180204214719/http://www.computer-engineering.org/ps2keyboard/scancodes1.html
#	https://web.archive.org/web/20180202062621/http://www.computer-engineering.org/ps2keyboard/scancodes2.html
#	https://web.archive.org/web/20180208084611/http://www.computer-engineering.org/ps2keyboard/scancodes3.html

sc_set1.append(( None,   ("e0 5b","e0 db"   ),   ("Left OS Key",None) )) # "L GUI"
#sc_set2.append(( None,   ("e0 1f","e0 f0 1f"),   ("Left OS Key",None) )) # (already found above)

# TODO: where is this on keyboard?
#sc_set1.append(( None,   ("e0 5c","e0 dc"   ),   (?,None) )) # "R GUI"
#sc_set2.append(( None,   ("e0 27","e0 f0 27"),   (?,None) )) # "R GUI"
#sc_set3.append(( None,   ("8c"   ,"f0 8c"   ),   (?,None) )) # "R WIN"

_tmp = [
	( ("e0 5e","e0 de"),   ("e0 37","e0 f0 37"),   "Power"          ),
	( ("e0 5f","e0 df"),   ("e0 3f","e0 f0 3f"),   "Sleep"          ),
	( ("e0 63","e0 e3"),   ("e0 5e","e0 f0 5e"),   "Wake"           ),

	( ("e0 19","e0 99"),   ("e0 4d","e0 f0 4d"),   "Next Track"     ),
	( ("e0 10","e0 90"),   ("e0 15","e0 f0 15"),   "Previous Track" ),
	( ("e0 24","e0 a4"),   ("e0 3b","e0 f0 3b"),   "Stop"           ),
	( ("e0 22","e0 a2"),   ("e0 34","e0 f0 34"),   "Play / Pause"   ),
	( ("e0 20","e0 a0"),   ("e0 23","e0 f0 23"),   "Mute"           ),
	( ("e0 30","e0 b0"),   ("e0 32","e0 f0 32"),   "Volume Up"      ),
	( ("e0 2e","e0 ae"),   ("e0 21","e0 f0 21"),   "Volume Down"    ),
	( ("e0 6d","e0 ed"),   ("e0 50","e0 f0 50"),   "Media Select"   ),
	( ("e0 6c","e0 ec"),   ("e0 48","e0 f0 48"),   "E-Mail"         ),
	( ("e0 21","e0 a1"),   ("e0 2b","e0 f0 2b"),   "Calculator"     ),
	( ("e0 6b","e0 eb"),   ("e0 40","e0 f0 40"),   "My Computer"    ),
	( ("e0 65","e0 e5"),   ("e0 10","e0 f0 10"),   "www Search"     ),
	( ("e0 32","e0 b2"),   ("e0 3a","e0 f0 3a"),   "www Home"       ),
	( ("e0 6a","e0 ea"),   ("e0 38","e0 f0 38"),   "www Back"       ),
	( ("e0 69","e0 e9"),   ("e0 30","e0 f0 30"),   "www Forward"    ),
	( ("e0 68","e0 e8"),   ("e0 28","e0 f0 28"),   "www Stop"       ),
	( ("e0 67","e0 e7"),   ("e0 20","e0 f0 20"),   "www Refresh"    ),
	( ("e0 66","e0 e6"),   ("e0 18","e0 f0 18"),   "www Favorites"  ),
]
for set1,set2, name in _tmp:
	sc_set1.append(( None, set1, (name,None) ))
	sc_set2.append(( None, set2, (name,None) ))

sc_set1.append(( None,   ("e0 5d","e0 dd"   ),   ("Menu",None) )) # "APPS"
#sc_set2.append(( None,   ("e0 2f","e0 f0 2f"),   ("Menu",None) )) # (already found above)
sc_set3.append(( None,   ("8d"   ,"f0 8d"   ),   ("Menu",None) ))
