# MOSS Input System

The input system of MOSS is somewhat involved and deserves a dedicated explanation.

Basically, input devices are controlled by an input devices controller, and every device is assigned
a device ID, through which you can command or read from that device.



## Unified Input Device Buttons

A major feature is that buttons on input devices are unified.  Basically, this means that when a
button press / release is registered, it is just a number in a space shared with all the other
devices.

The user can define translation tables that map events on one

TODO: below



Input devices are unified in that their button pushes emit a 


are unified 







There are several levels of abstraction and translation.  First, there is the 'key-mapping
positional-number', which roughly encodes the 2D position of a key on an abstract physical keyboard.
Positional-numbers are the way in which an end-user (or at least a human) defines a 'key-mapping'.
A key-mapping lists the positional-numbers and the associated actions or glyphs on them.

The representation that's actually used internally is a 'key-index', which is a 1-byte number that
indexes into tightly-packed tables of information (for example, the table of associated unicode
codepoints, the table of key names, etc.).  They are also used directly as a unique identifier for
the key.  The key-mappings compile down into this compacter representation, for maximum performance.
They also generate information that is used by drivers to directly translate into incoming keyboard
signals into key-indices.



                                 positional-number
								        ^
										|
									    v
scancode ->    keyboard           -> key-index
          kb->scancode_parser
	(according to current scancode set)


keyboard has table of commands (bytecode sequences) 




## Key-Mapping Positional-Number

First, we have the 'key-mapping positional-number'.  This is a number assigned to each key on the
keyboard, assigned according to the physical layout alone.  Its only purpose is to be used as an
intermediary for writing keyboard mappings.

There are several common physical layouts for the main region ([see][10] here and [here][11]), so we
consider an abstract keyboard that has the maximum number of keys for each area.

The mapping is fundamentally arbitrary, but has been chosen so that the positional-number's lower 5
bits give the position horizontally, while the upper 3 bits are the position vertically.  This gives
the bit pattern a spatial meaning, which (might) be more intuitive.

Another obvious alternative would have been to use IBM's key-numbers (see
["Hardware Interface Technical Reference" for "Keyboard 101- and 102-Key" (PDF)][12] from
[resource][13]), which are necessary for [PS/2 keyboard translation][14].  However, unfortunately,
although they reserved some numbers in anticipation of subdividing keys, they didn't reserve enough,
especially e.g. on the bottom row.  They also don't have (so much) a positional meaning.  Another
alternative would be [Linux's key-numbers][15] or [Windows's virtual keys][16], but these don't
correspond to physical keys on a single keyboard.

Visually, MOSS's mapping looks as follows.  Again, this is the main region of keyboard, abstracted
to have maximum number of buttons.  For each key, the typical US-QWERTY result is given for
intuition on top, and the key-number is written below.

	╭─────╮       ╭─────╮╭─────╮╭─────╮╭─────╮   ╭─────╮╭─────╮╭─────╮╭─────╮    ╭─────╮╭─────╮╭─────╮╭─────╮
	│ Esc │       │ F1  ││ F2  ││ F3  ││ F4  │   │ F5  ││ F6  ││ F7  ││ F8  │    │ F9  ││ F10 ││ F11 ││ F12 │
	│(160)│       │(161)││(162)││(163)││(164)│   │(165)││(166)││(167)││(168)│    │(169)││(170)││(171)││(172)│
	╰─────╯       ╰─────╯╰─────╯╰─────╯╰─────╯   ╰─────╯╰─────╯╰─────╯╰─────╯    ╰─────╯╰─────╯╰─────╯╰─────╯

	╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭────────╮
	│ ` ~ ││ 1 ! ││ 2 @ ││ 3 # ││ 4 $ ││ 5 % ││ 6 ^ ││ 7 & ││ 8 * ││ 9 ( ││ 0 ) ││ - _ ││ = + ││     ││Bk-space│
	│(128)││(129)││(130)││(131)││(132)││(133)││(134)││(135)││(136)││(137)││(138)││(139)││(140)││(141)││ (142)  │
	╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰────────╯
	╭────────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮
	│  Tab   ││ q Q ││ w W ││ e E ││ r R ││ t T ││ y Y ││ u U ││ i I ││ o O ││ p P ││ [ { ││ ] } ││     ││ \ | │
	│  (96)  ││(97) ││(98) ││(99) ││(100)││(101)││(102)││(103)││(104)││(105)││(106)││(107)││(108)││(109)││(110)│
	╰────────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯
	╭──────────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭──────────╮
	│ Caps Lck ││ a A ││ s S ││ d D ││ f F ││ g G ││ h H ││ j J ││ k K ││ l L ││ ; : ││ ' " ││     ││  Return  │
	│   (64)   ││(65) ││(66) ││(67) ││(68) ││(69) ││(70) ││(71) ││(72) ││(73) ││(74) ││(75) ││(76) ││   (77)   │
	╰──────────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰──────────╯
	╭──────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭──────────────╮
	│LShift││     ││ z Z ││ x X ││ c C ││ v V ││ b B ││ n N ││ m M ││ , < ││ . > ││ / ? ││     ││   R Shift    │
	│ (32) ││(33) ││(34) ││(35) ││(36) ││(37) ││(38) ││(39) ││(40) ││(41) ││(42) ││(43) ││(44) ││     (45)     │
	╰──────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰──────────────╯
	╭──────────╮╭─────╮╭─────╮╭─────╮╭─────╮╭───────────────────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭──────────╮
	│  L Ctrl  ││Func ││OSKyL││ Alt ││     ││     Spacebar      ││     ││     ││AltGr││OSKyR││Menu ││  R Ctrl  │
	│   (0)    ││ (1) ││ (2) ││ (3) ││ (4) ││        (5)        ││ (6) ││ (7) ││ (8) ││ (9) ││(10) ││   (11)   │
	╰──────────╯╰─────╯╰─────╯╰─────╯╰─────╯╰───────────────────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰──────────╯

The 'other side' of the keyboard is:

	╭─────╮╭─────╮╭─────╮
	│Pt Rq││ScrLk││Ps Bk│
	│(173)││(174)││(175)│
	╰─────╯╰─────╯╰─────╯

	╭─────╮╭─────╮╭─────╮   ╭─────╮╭─────╮╭─────╮╭─────╮
	│ Ins ││Home ││Pg Up│   │NumLk││  /  ││  *  ││  -  │
	│(143)││(144)││(145)│   │(146)││(147)││(148)││(149)│
	╰─────╯╰─────╯╰─────╯   ╰─────╯╰─────╯╰─────╯╰─────╯
	╭─────╮╭─────╮╭─────╮   ╭─────╮╭─────╮╭─────╮╭─────╮
	│ Del ││ End ││Pg Dn│   │7 Hm ││ 8 ↑ ││9 PgU││     │
	│(111)││(112)││(113)│   │(114)││(115)││(116)││     │
	╰─────╯╰─────╯╰─────╯   ╰─────╯╰─────╯╰─────╯│     │
	                        ╭─────╮╭─────╮╭─────╮│     │
	                        │ 4 ← ││  5  ││ 6 → ││  +  │
	                        │(78) ││(79) ││(80) ││(81) │
	                        ╰─────╯╰─────╯╰─────╯╰─────╯
	       ╭─────╮          ╭─────╮╭─────╮╭─────╮╭─────╮
	       │  ↑  │          │1 End││ 2 ↓ ││3 PgD││     │
	       │(46) │          │(47) ││(48) ││(49) ││     │
	       ╰─────╯          ╰─────╯╰─────╯╰─────╯│     │
	╭─────╮╭─────╮╭─────╮   ╭────────────╮╭─────╮│     │
	│  ←  ││  ↓  ││  →  │   │  0    Ins  ││. Del││Enter│
	│(12) ││(13) ││(14) │   │    (15)    ││(16) ││(17) │
	╰─────╯╰─────╯╰─────╯   ╰────────────╯╰─────╯╰─────╯

Some keyboards may also have power and multimedia buttons, which we treat as all on some abstract
"row" (the real arrangement of these buttons varies considerably, and most keyboards don't have them
anyway):

	╭─────╮╭─────╮╭─────╮   ╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮
	│Power││Sleep││Wake │   │Stop ││Play ││Mute ││VolDn││VolUp││SelMd││PvTrk││NxTrk│
	│(192)││(193)││(194)│   │(195)││(196)││(197)││(198)││(199)││(200)││(201)││(202)│
	╰─────╯╰─────╯╰─────╯   ╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯

The "row" continues with some weird junk (maybe we need more weird junk, hard to tell; see e.g.
[here][17]):

	╭─────╮╭─────╮╭─────╮   ╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮
	│Calc ││Email││MyCmp│   │WebBk││WebFw││WbStp││WebF5││WebHm││WbFav││WbSch│
	│(203)││(204)││(205)│   │(206)││(207)││(208)││(209)││(210)││(211)││(212)│
	╰─────╯╰─────╯╰─────╯   ╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯

[10]: https://en.wikipedia.org/wiki/Keyboard_layout#Physical_layouts
[11]: https://en.wikipedia.org/wiki/Keyboard_layout#/media/File:Physical_keyboard_layouts_comparison_ANSI_ISO_KS_ABNT_JIS.png
[12]: https://www.ardent-tool.com/docs/pdf/ibm_hitrc11.pdf
[13]: https://www.ardent-tool.com/
[14]: "../src/keyboard/ps2_keys.py"
[15]: https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h#L65
[16]: https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
[17]: https://www.amazon.com/Microsoft-K50-00100-Natural-Multimedia-Keyboard/dp/B0000AOWVN



## Key-Mapping

Key-mappings are more complicated than they may first seem:

- Many keys do not generate a unicode character at all (for example Ctrl or Home).

- We cannot just assume that every key generates at most one unicode character.  Some keys can
  generate multiple unicode characters (they are (misnamed) 'ligature keys').  For example, on the
  [Cherokee keyboard][20].

- There's no guarantee any generated unicode character is in the basic multilingual plane (the first
  2¹⁶ code points).  Windows and Linux both seem to want to assume that a codepoint is a 16-bit
  word (though Windows at least *can* support more).  However, we want to be able to accept input in
  stuff like the Adlam script, used by probably millions of people.

- There is not a well-defined maximum number of modified (shift, alternate) glyphs or operations per
  key, and it would be too inefficient to define them all for every key anyway.  Although most keys
  do only 1, 2 or maybe 3 functions, others do more.  4 is pretty common, and the
  [Japanese keyboard][21] has keys with 6 states (up to 8, depending on how you count).  The
  [Canadian multilingual][22] keyboard is another interesting example.  Also, these can be affected
  by various locking states (e.g. Caps Lock), sometimes in a way that *partially cancels* prior
  locking states (e.g. in the [Czech keyboard][23]).

- Locking states don't have to be toggles.  E.g. to emulate typewriters, one [can make][24] the
  caps-lock not disable when you hit it again.

- Some functions on some keys are 'dead keys'.  A dead key is a key that says that the next letter
  should have a particular diacritic over it.  Unfortunately, this is [exactly opposite][25] to what
  unicode does, where the diacritic comes after.  The [French keyboard][26] is loaded with them.  In
  practice, we can handle this at the kernel level instead of in the mapping, which can just use the
  appropriate unicode codepoints.

MOSS deals with this complexity by breaking down a key-mapping into distinct categories, which are
handled separately.  Keys maps be 


[20]: https://kbdlayout.info/kbdcher
[21]: https://kbdlayout.info/kbdjpn
[22]: https://en.wikipedia.org/wiki/CSA_keyboard
[23]: https://kbdlayout.info/kbdcz/shiftstates/base
[24]: https://man7.org/linux/man-pages/man5/keymaps.5.html
[25]: https://en.wikipedia.org/wiki/Precomposed_character#Comparing_precomposed_and_decomposed_characters
[26]: https://kbdlayout.info/kbdfrna



## Positional-Number → Key-Index

The positional-number is intended to be used by humans to define key-mappings.  However, we need a
separate concept, key-indices, for the actual internal representation.

The main reason is essentially that the space of key-indices can be tightly packed.  This
immediately makes all tables up to 60% smaller.  This reduces wasted space, making the kernel
lighter and faster (due to less cache missing).  It also makes checking for indexing and invalid
keys simpler.  It even frees up a bit (a key-index is 7-bit, while a positional-number is 8-bit),
which means certain bitfields can get just a little tighter.

Another important reason is that the key-indices for the 'modifier' keys (Ctrl, Alt, etc.) can be
contiguous.  Currently, they are the lower end of the space, meaning we can check if a key is a
modifier simply by checking if it's less than some number.  The keyboard stores a bitmask of the
keys that are pressed; you can get all the modifier bits at once out of this table instantly just
by taking the lower two bytes.

Tables can also be sorted according to descending frequency of use to facilitate better caching.

Finally, it is also future-proof in that almost any aspect of the internal representation can change
completely, without munging existing keyboard layouts (including end-user customizations).  You just
have to regenerate the appropriate tables from them again.




The first and most important job of a keyboard driver is to convert key-press / key-release signals
into an event on a MOSS key-number.

, also allowing for quickly constructing key-numbers from


physical positions.  Modifier keys (Ctrl, Shift, Caps Lock, etc.) are, however,
pulled out into a separate, contiguous block

this makes collecting the entire modifier state from a
driver very fast).  Note that key-numbers 0 and 255 are not needed by this scheme, so are good
'invalid' values.



The modifier keys were included above, but since they have special numbers, they can also
alternately be considered to live in their own, contiguous, abstract row:

	╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮╭─────╮
	│LCtrl││RCtrl││LAlt ││AltGr││LShft││RShft││OSKyL││OSKyL││Func ││CpsLk││NumLk││ScrLk││KnaLk││ Ins ││Mute │
	│[224]││[225]││[226]││[227]││[228]││[229]││[230]││[231]││[232]││[233]││[234]││[235]││[235]││[236]││[237]│
	╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯╰─────╯