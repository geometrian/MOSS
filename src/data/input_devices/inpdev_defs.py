class InpDev_Btn:
	def __init__( self, nums, macro_token, name ):
		self.name = name
		self.macro_token = macro_token

		self.value = nums[0]

		self.ibm_key_num = nums[1]

	@staticmethod
	def get_by_name( name ):
		return InpDev_Btn.btns_map[ name ]

InpDev_Btn.btns = [ InpDev_Btn(*params) for params in [

	# (MOSS_num,IBM_key_num), macro_token, human name (referred to US-QWERTY)

	( (  0,  58), "LeftCtrl"     , "Left Ctrl"      ),
	( (  1,  60), "Func"         , "Function"       ),
	( (  2,None), "LeftOSKey"    , "Left OS Key"    ),
	( (  3,None), "LeftAlt"      , "Left Alt"       ),
	( (  4,None), "Other4"       , "Other4"         ),
	( (  5,  61), "Spacebar"     , "Spacebar"       ),
	( (  6,None), "Other6"       , "Other6"         ),
	( (  7,None), "Other7"       , "Other7"         ),
	( (  8,  62), "AltGr"        , "Alt Gr"         ),
	( (  9,None), "RightOSKey"   , "Right OS Key"   ),
	( ( 10,None), "Menu"         , "Menu"           ),
	( ( 11,  64), "RightCtrl"    , "Right Ctrl"     ),
	( ( 12,  79), "LeftArrow"    , "Left Arrow"     ),
	( ( 13,  84), "DownArrow"    , "Down Arrow"     ),
	( ( 14,  89), "RightArrow"   , "Right Arrow"    ),
	( ( 15,  99), "Numpad0"      , "Numpad 0"       ),
	( ( 16, 104), "NumpadDot"    , "Numpad ."       ),
	( ( 17, 108), "NumpadEnter"  , "Numpad Enter"   ),

	( ( 32,  44), "LeftShift"    , "Left Shift"     ),
	( ( 33,  45), "Other33"      , "Other33"        ),
	( ( 34,  46), "z"            , "z"              ),
	( ( 35,  47), "x"            , "x"              ),
	( ( 36,  48), "c"            , "c"              ),
	( ( 37,  49), "v"            , "v"              ),
	( ( 38,  50), "b"            , "b"              ),
	( ( 39,  51), "n"            , "n"              ),
	( ( 40,  52), "m"            , "m"              ),
	( ( 41,  53), "Comma"        , ","              ),
	( ( 42,  54), "Dot"          , "."              ),
	( ( 43,  55), "Slash"        , "/"              ),
	( ( 44,None), "Other44"      , "Other44"        ),
	( ( 45,  57), "RightShift"   , "Right Shift"    ),
	( ( 46,  83), "UpArrow"      , "Up Arrow"       ),
	( ( 47,  93), "Numpad1"      , "Numpad 1"       ),
	( ( 48,  98), "Numpad2"      , "Numpad 2"       ),
	( ( 49, 103), "Numpad3"      , "Numpad 3"       ),

	( ( 64,  30), "CapsLock"     , "Caps Lock"      ),
	( ( 65,  31), "a"            , "a"              ),
	( ( 66,  32), "s"            , "s"              ),
	( ( 67,  33), "d"            , "d"              ),
	( ( 68,  34), "f"            , "f"              ),
	( ( 69,  35), "g"            , "g"              ),
	( ( 70,  36), "h"            , "h"              ),
	( ( 71,  37), "j"            , "j"              ),
	( ( 72,  38), "k"            , "k"              ),
	( ( 73,  39), "l"            , "l"              ),
	( ( 74,  40), "Semicolon"    , ";"              ),
	( ( 75,  41), "SingleQuote"  , "'"              ),
	( ( 76,  42), "Other76"      , "Other76"        ),
	( ( 77,  43), "Return"       , "Return"         ),
	( ( 78,  92), "Numpad4"      , "Numpad 4"       ),
	( ( 79,  97), "Numpad5"      , "Numpad 5"       ),
	( ( 80, 102), "Numpad6"      , "Numpad 6"       ),
	( ( 81, 106), "NumpadPlus"   , "Numpad +"       ),

	( ( 96,  16), "Tab"          , "Tab"            ),
	( ( 97,  17), "q"            , "q"              ),
	( ( 98,  18), "w"            , "w"              ),
	( ( 99,  19), "e"            , "e"              ),
	( (100,  20), "r"            , "r"              ),
	( (101,  21), "t"            , "t"              ),
	( (102,  22), "y"            , "y"              ),
	( (103,  23), "u"            , "u"              ),
	( (104,  24), "i"            , "i"              ),
	( (105,  25), "o"            , "o"              ),
	( (106,  26), "p"            , "p"              ),
	( (107,  27), "LeftBracket"  , "["              ),
	( (108,  28), "RightBracket" , "]"              ),
	( (109,None), "Other109"     , "Other109"       ),
	( (110,  29), "Backslash"    , "\\"             ),
	( (111,  76), "Delete"       , "Delete"         ),
	( (112,  81), "End"          , "End"            ),
	( (113,  88), "PageDown"     , "Page Down"      ),
	( (114,  91), "Numpad7"      , "Numpad 7"       ),
	( (115,  96), "Numpad8"      , "Numpad 8"       ),
	( (116, 101), "Numpad9"      , "Numpad 9"       ),

	( (128,   1), "Backtick"     , "`"              ),
	( (129,   2), "1"            , "1"              ),
	( (130,   3), "2"            , "2"              ),
	( (131,   4), "3"            , "3"              ),
	( (132,   5), "4"            , "4"              ),
	( (133,   6), "5"            , "5"              ),
	( (134,   7), "6"            , "6"              ),
	( (135,   8), "7"            , "7"              ),
	( (136,   9), "8"            , "8"              ),
	( (137,  10), "9"            , "9"              ),
	( (138,  11), "0"            , "0"              ),
	( (139,  12), "Minus"        , "-"              ),
	( (140,  13), "Equals"       , "="              ),
	( (141,None), "Other141"     , "Other141"       ),
	( (142,  15), "Backspace"    , "Backspace"      ),
	( (143,  75), "Insert"       , "Insert"         ),
	( (144,  80), "Home"         , "Home"           ),
	( (145,  85), "PageUp"       , "Page Up"        ),
	( (146,  90), "NumLock"      , "Num Lock"       ),
	( (147,  95), "NumpadSlash"  , "Numpad /"       ),
	( (148, 100), "NumpadStar"   , "Numpad *"       ),
	( (149, 105), "NumpadMinus"  , "Numpad -"       ),

	( (160, 110), "Escape"       , "Escape"         ),
	( (161, 112), "F1"           , "F1"             ),
	( (162, 113), "F2"           , "F2"             ),
	( (163, 114), "F3"           , "F3"             ),
	( (164, 115), "F4"           , "F4"             ),
	( (165, 116), "F5"           , "F5"             ),
	( (166, 117), "F6"           , "F6"             ),
	( (167, 118), "F7"           , "F7"             ),
	( (168, 119), "F8"           , "F8"             ),
	( (169, 120), "F9"           , "F9"             ),
	( (170, 121), "F10"          , "F10"            ),
	( (171, 122), "F11"          , "F11"            ),
	( (172, 123), "F12"          , "F12"            ),
	( (173, 124), "PrintScreen"  , "Print Screen"   ),
	( (174, 125), "ScrollLock"   , "Scroll Lock"    ),
	( (175, 126), "PauseBreak"   , "Pause / Break"  ),

	( (192,None), "Power"        , "Power"          ),
	( (193,None), "Sleep"        , "Sleep"          ),
	( (194,None), "Wake"         , "Wake"           ),
	( (195,None), "Stop"         , "Stop"           ),
	( (196,None), "PlayPause"    , "Play / Pause"   ),
	( (197,None), "Mute"         , "Mute"           ),
	( (198,None), "VolumeDown"   , "Volume Down"    ),
	( (199,None), "VolumeUp"     , "Volume Up"      ),
	( (200,None), "SelectMedia"  , "Media Select"   ),
	( (201,None), "PreviousTrack", "Previous Track" ),
	( (202,None), "NextTrack"    , "Next Track"     ),
	( (203,None), "Calculator"   , "Calculator"     ),
	( (204,None), "Email"        , "E-Mail"         ),
	( (205,None), "MyComputer"   , "My Computer"    ),
	( (206,None), "WebBackward"  , "www Back"       ),
	( (207,None), "WebForward"   , "www Forward"    ),
	( (208,None), "WebStop"      , "www Stop"       ),
	( (209,None), "WebRefresh"   , "www Refresh"    ),
	( (210,None), "WebHome"      , "www Home"       ),
	( (211,None), "WebFavorites" , "www Favorites"  ),
	( (212,None), "WebSearch"    , "www Search"     ),

	( (224,None), "MouseLeft"    , "Mouse Left"     ),
	( (225,None), "MouseMiddle"  , "Mouse Middle"   ),
	( (226,None), "MouseRight"   , "Mouse Right"    ),
	( (227,None), "MouseButton4" , "Mouse Button 4" ),
	( (228,None), "MouseButton5" , "Mouse Button 5" ),

	( (255,None), "Invalid"      , "Invalid"        )
] ]

InpDev_Btn.btns_map = {
	btn.name : btn
	for btn in InpDev_Btn.btns
}



inpdev_ctrl_cmds = [
	"KeyPress_And_EmitPayloadAsUTF16",
	"KeyPress_And_SavePayloadAsUTF16",

	"KeyPress_And_KeyLockToggle",
	"KeyPress_And_KeyLockSet",
	"KeyPress_And_KeyLockClear",

	"KeyRelease",

	"PowerSleep",
	"PowerWake",
]
