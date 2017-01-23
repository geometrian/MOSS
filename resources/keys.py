#http://msdn.microsoft.com/en-us/library/ms894073.aspx
#See there for English and other data

#No need to escape the backslash manually?

table1 = """29	OEM_3	0	`	~	 	 
02	1	0	1	!	 	 
03	2	0	2	@	–1	0000
04	3	0	3	#	 	 
05	4	0	4	$	 	 
06	5	0	5	%	 	 
07	6	0	6	^	–1	001e
08	7	0	7	&	 	 
09	8	0	8	*	 	 
0a	9	0	9	(	 	 
0b	0	0	0	)	 	 
0c	OEM_MINUS	0	-	_	–1	001f
0d	OEM_PLUS	0	=	+	 	 
10	Q	1	q	Q	 	 
11	W	1	w	W	 	 
12	E	1	e	E	 	 
13	R	1	r	R	 	 
14	T	1	t	T	 	 
15	Y	1	y	Y	 	 
16	U	1	u	U	 	 
17	I	1	i	I	 	 
18	O	1	o	O	 	 
19	P	1	p	P	 	 
1a	OEM_4	0	[	{	001b	 
1b	OEM_6	0	]	}	001d	 
2b	OEM_5	0	\	|	001c	 
1e	A	1	a	A	 	 
1f	S	1	s	S	 	 
20	D	1	d	D	 	 
21	F	1	f	F	 	 
22	G	1	g	G	 	 
23	H	1	h	H	 	 
24	J	1	j	J	 	 
25	K	1	k	K	 	 
26	L	1	l	L	 	 
27	OEM_1	0	;	:	 	 
28	OEM_7	0	'	"	 	 
2c	Z	1	z	Z	 	 
2d	X	1	x	X	 	 
2e	C	1	c	C	 	 
2f	V	1	v	V	 	 
30	B	1	b	B	 	 
31	N	1	n	N	 	 
32	M	1	m	M	 	 
33	OEM_COMMA	0	,	<	 	 
34	OEM_PERIOD	0	.	>	 	 
35	OEM_2	0	/	?	 	 
56	OEM_102	0	\	|	001c	 
53	DECIMAL	0	.	.	 	 """

table2 = """01	ESC
0e	BACKSPACE
0f	TAB
1c	ENTER
1d	CTRL
2a	SHIFT
36	Right SHIFT
37	NUM *
38	ALT
39	SPACE
3a	CAPS LOCK
3b	F1
3c	F2
3d	F3
3e	F4
3f	F5
40	F6
41	F7
42	F8
43	F9
44	F10
45	PAUSE
46	SCROLL LOCK
47	Num 7
48	Num 8
49	Num 9
4a	Num -
4b	Num 4
4c	Num 5
4d	Num 6
4e	Num +
4f	Num 1
50	Num 2
51	Num 3
52	Num 0
53	Num Del
54	Sys Req
57	F11
58	F12
7c	F13
7d	F14
7e	F15
7f	F16
80	F17
81	F18
82	F19
83	F20
84	F21
85	F22
86	F23
87	F24"""

data = []
def add(sc, vk,vk_shift, ch,ch_shift):
    if int("0x"+sc,16) >= 0x80: return #we need that bit for breaking!
    if vk == "DECIMAL": return #TODO: conflicts with other 0x53!
    def process_vk(vk):
        if vk == "ESC": return "ESCAPE"
        vk2 = ""
        for c in vk:
            if c == " ": vk2 += "_"
            else: vk2 += c
        vk3 = vk2.upper().replace("NUM_","KP_")
        if vk3 == "KP_*": return "KP_TIMES"
        if vk3 == "KP_-": return "KP_MINUS"
        if vk3 == "KP_+": return "KP_PLUS"
        if vk3.startswith("LEFT_"): vk3=vk3[5:]+"_L"
        if vk3.startswith("RIGHT_"): vk3=vk3[6:]+"_R"
        if vk3 == "SHIFT": return "SHIFT_L"
        if vk3 == "SCROLL_LOCK": return "LOCK_SCROLL"
        if vk3 == "CAPS_LOCK": return "LOCK_CAPS"
        return vk3
    def process_ch(ch):
        if ch == "\\": return "\\\\"
        if ch == "\"": return "\\\""
        if ch == "SPACE": return " "
        if ch == "TAB": return "\\t"
        if ch == "ESC": return "ESCAPE"
        if ch == "ENTER": return "\\n"
        return ch
    ch = process_ch(ch)
    ch_shift = process_ch(ch_shift)
    vk = process_vk(vk)
    vk_shift = process_vk(vk_shift)
    data.append([sc,[vk,vk_shift,ch,ch_shift]])
for line in table1.split("\n"):
    sc,vk,cap,ch,ch_shift,ctrl,ctrl_shift = line.split("\t")
    add(sc, vk,vk, ch,ch_shift)
for line in table2.split("\n"):
    sc,ch = line.split("\t")
    add(sc, ch,"INVALID", ch,"INVALID")
data.sort()

longest = [0,0,0,0]
for sc,key_record in data:
    for i in range(4):
        if len(key_record[i])>longest[i]: longest[i]=len(key_record[i])

for sc,key_record in data:
    spacing = [(longest[i]-len(key_record[i]))*" " for i in range(4)]
    print("\tMACRO("+spacing[0]+key_record[0]+","+spacing[1]+key_record[1]+", "+spacing[2]+"\""+key_record[2]+"\","+spacing[3]+"\""+key_record[3]+"\", 0x"+sc+")\\")


























