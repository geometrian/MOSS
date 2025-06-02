#pragma once

#include "../stdafx.h"



/* ######################################################
 * Auto-generated file; change the Python script instead!
 * ###################################################### */



// Input Device Controller Buttons

#define INPUTDEV_BTNS_APPLY(MACRO)\
	MACRO( LeftCtrl     ,   0, "Left Ctrl"      )\
	MACRO( Func         ,   1, "Function"       )\
	MACRO( LeftOSKey    ,   2, "Left OS Key"    )\
	MACRO( LeftAlt      ,   3, "Left Alt"       )\
	MACRO( Other4       ,   4, "Other4"         )\
	MACRO( Spacebar     ,   5, "Spacebar"       )\
	MACRO( Other6       ,   6, "Other6"         )\
	MACRO( Other7       ,   7, "Other7"         )\
	MACRO( AltGr        ,   8, "Alt Gr"         )\
	MACRO( RightOSKey   ,   9, "Right OS Key"   )\
	MACRO( Menu         ,  10, "Menu"           )\
	MACRO( RightCtrl    ,  11, "Right Ctrl"     )\
	MACRO( LeftArrow    ,  12, "Left Arrow"     )\
	MACRO( DownArrow    ,  13, "Down Arrow"     )\
	MACRO( RightArrow   ,  14, "Right Arrow"    )\
	MACRO( Numpad0      ,  15, "Numpad 0"       )\
	MACRO( NumpadDot    ,  16, "Numpad ."       )\
	MACRO( NumpadEnter  ,  17, "Numpad Enter"   )\
	\
	MACRO( LeftShift    ,  32, "Left Shift"     )\
	MACRO( Other33      ,  33, "Other33"        )\
	MACRO( z            ,  34, "z"              )\
	MACRO( x            ,  35, "x"              )\
	MACRO( c            ,  36, "c"              )\
	MACRO( v            ,  37, "v"              )\
	MACRO( b            ,  38, "b"              )\
	MACRO( n            ,  39, "n"              )\
	MACRO( m            ,  40, "m"              )\
	MACRO( Comma        ,  41, ","              )\
	MACRO( Dot          ,  42, "."              )\
	MACRO( Slash        ,  43, "/"              )\
	MACRO( Other44      ,  44, "Other44"        )\
	MACRO( RightShift   ,  45, "Right Shift"    )\
	MACRO( UpArrow      ,  46, "Up Arrow"       )\
	MACRO( Numpad1      ,  47, "Numpad 1"       )\
	MACRO( Numpad2      ,  48, "Numpad 2"       )\
	MACRO( Numpad3      ,  49, "Numpad 3"       )\
	\
	MACRO( CapsLock     ,  64, "Caps Lock"      )\
	MACRO( a            ,  65, "a"              )\
	MACRO( s            ,  66, "s"              )\
	MACRO( d            ,  67, "d"              )\
	MACRO( f            ,  68, "f"              )\
	MACRO( g            ,  69, "g"              )\
	MACRO( h            ,  70, "h"              )\
	MACRO( j            ,  71, "j"              )\
	MACRO( k            ,  72, "k"              )\
	MACRO( l            ,  73, "l"              )\
	MACRO( Semicolon    ,  74, ";"              )\
	MACRO( SingleQuote  ,  75, "'"              )\
	MACRO( Other76      ,  76, "Other76"        )\
	MACRO( Return       ,  77, "Return"         )\
	MACRO( Numpad4      ,  78, "Numpad 4"       )\
	MACRO( Numpad5      ,  79, "Numpad 5"       )\
	MACRO( Numpad6      ,  80, "Numpad 6"       )\
	MACRO( NumpadPlus   ,  81, "Numpad +"       )\
	\
	MACRO( Tab          ,  96, "Tab"            )\
	MACRO( q            ,  97, "q"              )\
	MACRO( w            ,  98, "w"              )\
	MACRO( e            ,  99, "e"              )\
	MACRO( r            , 100, "r"              )\
	MACRO( t            , 101, "t"              )\
	MACRO( y            , 102, "y"              )\
	MACRO( u            , 103, "u"              )\
	MACRO( i            , 104, "i"              )\
	MACRO( o            , 105, "o"              )\
	MACRO( p            , 106, "p"              )\
	MACRO( LeftBracket  , 107, "["              )\
	MACRO( RightBracket , 108, "]"              )\
	MACRO( Other109     , 109, "Other109"       )\
	MACRO( Backslash    , 110, "\\"             )\
	MACRO( Delete       , 111, "Delete"         )\
	MACRO( End          , 112, "End"            )\
	MACRO( PageDown     , 113, "Page Down"      )\
	MACRO( Numpad7      , 114, "Numpad 7"       )\
	MACRO( Numpad8      , 115, "Numpad 8"       )\
	MACRO( Numpad9      , 116, "Numpad 9"       )\
	\
	MACRO( Backtick     , 128, "`"              )\
	MACRO( 1            , 129, "1"              )\
	MACRO( 2            , 130, "2"              )\
	MACRO( 3            , 131, "3"              )\
	MACRO( 4            , 132, "4"              )\
	MACRO( 5            , 133, "5"              )\
	MACRO( 6            , 134, "6"              )\
	MACRO( 7            , 135, "7"              )\
	MACRO( 8            , 136, "8"              )\
	MACRO( 9            , 137, "9"              )\
	MACRO( 0            , 138, "0"              )\
	MACRO( Minus        , 139, "-"              )\
	MACRO( Equals       , 140, "="              )\
	MACRO( Other141     , 141, "Other141"       )\
	MACRO( Backspace    , 142, "Backspace"      )\
	MACRO( Insert       , 143, "Insert"         )\
	MACRO( Home         , 144, "Home"           )\
	MACRO( PageUp       , 145, "Page Up"        )\
	MACRO( NumLock      , 146, "Num Lock"       )\
	MACRO( NumpadSlash  , 147, "Numpad /"       )\
	MACRO( NumpadStar   , 148, "Numpad *"       )\
	MACRO( NumpadMinus  , 149, "Numpad -"       )\
	\
	MACRO( Escape       , 160, "Escape"         )\
	MACRO( F1           , 161, "F1"             )\
	MACRO( F2           , 162, "F2"             )\
	MACRO( F3           , 163, "F3"             )\
	MACRO( F4           , 164, "F4"             )\
	MACRO( F5           , 165, "F5"             )\
	MACRO( F6           , 166, "F6"             )\
	MACRO( F7           , 167, "F7"             )\
	MACRO( F8           , 168, "F8"             )\
	MACRO( F9           , 169, "F9"             )\
	MACRO( F10          , 170, "F10"            )\
	MACRO( F11          , 171, "F11"            )\
	MACRO( F12          , 172, "F12"            )\
	MACRO( PrintScreen  , 173, "Print Screen"   )\
	MACRO( ScrollLock   , 174, "Scroll Lock"    )\
	MACRO( PauseBreak   , 175, "Pause / Break"  )\
	\
	MACRO( Power        , 192, "Power"          )\
	MACRO( Sleep        , 193, "Sleep"          )\
	MACRO( Wake         , 194, "Wake"           )\
	MACRO( Stop         , 195, "Stop"           )\
	MACRO( PlayPause    , 196, "Play / Pause"   )\
	MACRO( Mute         , 197, "Mute"           )\
	MACRO( VolumeDown   , 198, "Volume Down"    )\
	MACRO( VolumeUp     , 199, "Volume Up"      )\
	MACRO( SelectMedia  , 200, "Media Select"   )\
	MACRO( PreviousTrack, 201, "Previous Track" )\
	MACRO( NextTrack    , 202, "Next Track"     )\
	MACRO( Calculator   , 203, "Calculator"     )\
	MACRO( Email        , 204, "E-Mail"         )\
	MACRO( MyComputer   , 205, "My Computer"    )\
	MACRO( WebBackward  , 206, "www Back"       )\
	MACRO( WebForward   , 207, "www Forward"    )\
	MACRO( WebStop      , 208, "www Stop"       )\
	MACRO( WebRefresh   , 209, "www Refresh"    )\
	MACRO( WebHome      , 210, "www Home"       )\
	MACRO( WebFavorites , 211, "www Favorites"  )\
	MACRO( WebSearch    , 212, "www Search"     )\
	\
	MACRO( MouseLeft    , 224, "Mouse Left"     )\
	MACRO( MouseMiddle  , 225, "Mouse Middle"   )\
	MACRO( MouseRight   , 226, "Mouse Right"    )\
	MACRO( MouseButton4 , 227, "Mouse Button 4" )\
	MACRO( MouseButton5 , 228, "Mouse Button 5" )\
	\
	MACRO( Invalid      , 255, "Invalid"        )

enum
{
	#define MOSS_DEF( TOKEN, IND, NAME )\
		INPDEV_BTN_##TOKEN = IND,
	INPUTDEV_BTNS_APPLY( MOSS_DEF )
	#undef MOSS_DEF
};
typedef uint8_t INPDEV_BTN;

MOSS_ND char const* inpdev_btn_to_str( INPDEV_BTN inpdev_btn );



// Input Device Controller bytecode

enum
{
	INPDEV_CMD_KeyPress_And_EmitPayloadAsUTF16,
	INPDEV_CMD_KeyPress_And_SavePayloadAsUTF16,
	INPDEV_CMD_KeyPress_And_KeyLockToggle,
	INPDEV_CMD_KeyPress_And_KeyLockSet,
	INPDEV_CMD_KeyPress_And_KeyLockClear,
	INPDEV_CMD_KeyRelease,
	INPDEV_CMD_PowerSleep,
	INPDEV_CMD_PowerWake,
};
typedef uint8_t INPDEV_CMD;
