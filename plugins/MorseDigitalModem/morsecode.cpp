//GPL license and attributions are in gpl.h and terms are included in this file by reference
#include "gpl.h"
#include "morsecode.h"

//Main table used to generate fast lookup tables
//DotDash must be unique for each entry
//7 Elements max
MorseSymbol MorseCode::m_morseTable[] = {
    //Prosigns, no interletter spacing http://en.wikipedia.org/wiki/Prosigns_for_Morse_Code
    //Special abreviations http://en.wikipedia.org/wiki/Morse_code_abbreviations
    //Supposed to have inter-letter spacing, but often sent as single 'letter'
    //Non English extensions (TBD from Wikipedia)

	//Todo: Support for 8 elelement Morse like (ER)
	//ASCII codes must be unique, no duplicates allowed in table

	//Ascii  Display   Dot/Dash	Token
	{0x21,	"!",	"-.-.--",	0	}, // exclamation
	{0x22,   "<CT>",   "-·-·-",	0	}, // '"' Attention or Start copying (same as KA)
	{0x23,   "<NJ>",   "-··---",	0	},  // '#' Shift to Wabun code
	{0x24,	"$",	"...-..-",	0	},
	{0x25,	"<SK>",   "...-.-",	0	}, // '%' End of QSO or End of Work
	{0x26,	"<INT>",  "..-.-",	0	}, // '&'
	{0x27,	"'",	".----.",	0	}, // apostrophe
	{0x28,	"(",	"-.--.",	0	}, // open paren
	{0x29,	")",	"-.--.-",	0	}, // close paren
	{0x2a,   "<SN>",   "···-·",	0	}, //Understood
	{0x2b,	"<KN>",   "-.--.",	0	}, // '+' Invitation to a specific station to transmit, compared to K which is general
	{0x2c,	",",	"--..--",	0	}, // comma
	{0x2d,	"-",	"-....-",	0	}, // dash
	{0x2e,	".",	".-.-.-",	0	}, // period
	{0x2f,	"/",	"-..-.",	0	}, // forward slash

	{0x30,	"0",	"-----",	0	},
	{0x31,	"1",	".----",	0	},
	{0x32,	"2",	"..---",	0	},
	{0x33,	"3",	"...--",	0	},
	{0x34,	"4",	"....-",	0	},
	{0x35,	"5",	".....",	0	},
	{0x36,	"6",	"-....",	0	},
	{0x37,	"7",	"--...",	0	},
	{0x38,	"8",	"---..",	0	},
	{0x39,	"9",	"----.",	0	},
	{0x3a,	":",	"---...",	0	}, // colon
	{0x3b,	";",	"-.-.-.",	0	}, // semi colon
	{0x3c,	"<AS>",   ".-...",	0	}, // '<' Please Wait or '&'
	{0x3d,	"<BT>\n",   "-...-",	0	}, // '='  2 LF or new paragraph
	{0x3e,	"<AR>",   ".-.-.",	0	}, // '>' End of Message, sometimes shown as '+'
	{0x3f,	"?",	"..--..",	0	}, // question
	{0x40,	"@",	".--.-.",	0	}, // At sign (Added to ITU standard in 2004)

	//ASCII letters (always converted to UC for typing, 'a' same as 'A'
	{0x41,	"A",	".-",	0	},
	{0x42,	"B",	"-...",	0	},
	{0x43,	"C",	"-.-.",	0	},
	{0x44,	"D",	"-..",	0	},
	{0x45,	"E",	".",	0	},
	{0x46,	"F",	"..-.",	0	},
	{0x47,	"G",	"--.",	0	},
	{0x48,	"H",	"....",	0	},
	{0x49,	"I",	"..",	0	},
	{0x4a,	"J",	".---",	0	},
	{0x4b,	"K",	"-.-",	0	}, //Also <K>
	{0x4c,	"L",	".-..",	0	},
	{0x4d,	"M",	"--",	0	},
	{0x4e,	"N",	"-.",	0	},
	{0x4f,	"O",	"---",	0	},
	{0x50,	"P",	".--.",	0	},
	{0x51,	"Q",	"--.-",	0	},
	{0x52,	"R",	".-.",	0	},
	{0x53,	"S",	"...",	0	},
	{0x54,	"T",	"-",	0	},
	{0x55,	"U",	"..-",	0	},
	{0x56,	"V",	"...-",	0	},
	{0x57,	"W",	".--",	0	},
	{0x58,	"X",	"-..-",	0	},
	{0x59,	"Y",	"-.--",	0	},
	{0x5a,	"Z",	"--..",	0	},
	{0x5b,   "<BK>",   "-...-.-",	0}, // '[' Break (Bk)
	{0x5c,	"\\",	".-..-.",	0	}, // '\' back slash
	//{0x5d,   "<HH>",   "·······.",	0},  // ']' Error (8 elements, too long)
	//{0x5e,   "<CL>",   "-·-··-··",	0}, // '^' Going off air (8 elements, too long)
	{0x5f,	"_",	"..--.-",	0	}, // '_' underscore

	{0x7b,	"<HM>",   "....--",	0	}, // '{'
	//{0x7c,   "<SOS>",  "···---···",	0}, // '|' SOS (Too long)
	{0x7d,	"<VE>",   "...-.",	0	}, // '}'
	{0x7e,	"<AA>",   ".-.-",	0	}, // '~' CR/LF

	//International characters
	//Todo: update display so characters are readable in morse window

	//WARNING: There are duplicate morse codes which must be handled
	//Lookup by ascii will work, but init() will skip duplicate tokens to lookup isn't broken
	//Typing the ascii code will send the right morse
	//But decoding will print the first symbol found

	//These map to extended ascii
	{0x80,	"Ç",	"-.-..",	0	}, //C cedila ISO 8859-1
	{0x85,	"À",	".--.-",	0	}, //A grave
	{0x8a,	"È",	".-..-",	0	}, //E grave ISO 8859-1
	{0x8e,	"Ä",	".-.-",		0	}, //A diaeresis ISO 8859-1
	{0x8f,	"Ȧ",	".--.-",	0	}, //A dot ISO 8859-1
	{0x90,	"É",	"..-..",	0	}, //E acute ISO 8859-1
	{0x92,	"Æ",	".-.-",		0	}, //AE
	{0x99,	"Ö",	"---.",		0	}, //O diaeresis ISO 8859-1
	{0x9a,	"Ü",	"..--",		0	}, //U diaeresis ISO 8859-1
	{0xa2,	"Ó",	"---.",		0	}, //O acute
	{0xa5,	"Ñ",	"--.--",	0	}, //N tilde ISO 8859-1

	//These don't map to any ascii, just assign sequential codes for now until we can figure out typeing codes
	{0xb0,	"Ą",	".-.-",		0	}, //A cedila
	{0xb1,	"Ć",	"-.-..",	0	}, //C acute
	{0xb2,	"Ĉ",	"-.-..",	0	}, //C circumflex
	{0xb3,	"(Ch)",	"----",		0	}, //Ch not printable?
	{0xb4,	"Đ",	"..-..",	0	}, //D with stroke
	{0xb5,	"Ð",	"..--.",	0	}, //Eth
	{0xb6,	"Ȩ",	"..-..",	0	}, //E cedila
	{0xb7,	"Ĝ",	"--.-.",	0	}, //G circumflex
	{0xb8,	"Ĥ",	"----",		0	}, //H circumflex
	{0xb9,	"Ĵ",	".---.",	0	}, //J circumflex
	{0xba,	"Ł",	".-..-",	0	}, //L stroke
	{0xbb,	"Ń",	"--.--",	0	}, //N acute
	{0xbc,	"Ø",	"---.",		0	}, //O stroke
	{0xbd,	"Ś",	"...-...",	0	}, //S acute
	{0xbe,	"Ŝ",	"...-.",	0	}, //S circumflex, also prosign for understood
	{0xbf,	"Š",	"----",		0	}, //S caron ISO 8859-2
	{0xc0,	"Ᵽ",	".--..",	0	}, //P stroke?
	{0xc1,	"Ǔ",	"..--",		0	}, //U caron
	{0xc2,	"Ź",	"--..-.",	0	}, //Z acute
	{0xc3,	"Ż",	"--..-",	0	}, //Z dot ISO 8859-2


	{0,		NULL,	NULL,		0	} //Must terminate table
};

/*
  Abbreviations, assume letter space between char or not?
AA	All after (used after question mark to request a repetition)
AB	All before (similarly)
ARRL	American Radio Relay League
ABT	About
ADR	Address
AGN	Again
ANT	Antenna
ARND	Around
BCI	Broadcast interference
BK	Break (to pause transmission of a message, say)
BN	All between
BTR	Better
BUG	Semiautomatic mechanical key
BURO	Bureau (usually used in the phrase PLS QSL VIA BURO, "Please send QSL card via my local/national QSL bureau")
B4	Before
C	Yes; correct
CBA	Callbook address
CFM	Confirm
CK	Check
CL	Clear (I am closing my station)
CLG	Calling
CQ	Calling any station
CQD	Original International Distress Call, fell out of use before 1915
CS	Callsign
CTL	Control
CUD	Could
CUL	See you later
CUZ	Because
CW	Continuous wave (i.e., radiotelegraph)
CX	Conditions
DE	From (or "this is")
DN	Down
DR	Dear
DSW	Goodbye (Russian: до свидания [Do svidanya])
DX	Distance (sometimes refers to long distance contact), foreign countries
EMRG	Emergency
ENUF	Enough
ES	And
FB	Fine business (Analogous to "OK")
FCC	Federal Communications Commission
FER	For
FM	From
FREQ	Frequency
FWD	Forward
GA	Good afternoon or Go ahead (depending on context)
GE	Good evening
GG	Going
GL	Good luck
GM	Good morning
GN	Good night
GND	Ground (ground potential)
GUD	Good
GX	Ground
HEE	Humour intended (often repeated, e.g. HEE HEE)
HI	Humour intended (possibly derived from HEE)
HR	Here, hear
HV	Have
HW	How
II	I say again
IMP	Impedance
K	Over
KN	Over; only the station named should respond (e.g. W7PTH DE W1AW KN)
LID	Poor operator
MILS	Milliamperes
MNI	Many
MSG	Message
N	No; nine
NIL	Nothing
NM	Name
NR	Number
NW	Now
NX	Noise; noisy
OB	Old boy
OC	Old chap
OM	Old man (any male amateur radio operator is an OM regardless of age)
OO	Official observer
OP	Operator
OT	Old timer
OTC	Old timers club (ARRL-sponsored organization for radio amateurs first licensed 20 or more years ago)
OOTC	Old old timers club (organization for those whose first two-way radio contact occurred 40 or more years ago; separate from OTC and ARRL)
PLS	Please
PSE	Please
PWR	Power
PX	Prefix
QCWA	Quarter Century Wireless Association (organization for radio amateurs who have been licensed 25 or more years)
R	Are; received as transmitted (origin of "Roger"), or decimal point (depending on context)
RCVR	Receiver (radio)
RFI	Radio Frequency Interference
RIG	Radio apparatus
RPT	Repeat or report (depending on context)
RPRT	Report
RST	Signal report format (Readability-Signal Strength-Tone)
RTTY	Radioteletype
RX	Receiver, radio
SAE	Self-addressed envelope
SASE	Self-addressed, stamped envelope
SED	Said
SEZ	Says
SFR	So far (proword)
SIG	Signal or signature
SIGS	Signals
SK	Out (proword), end of contact
SK	Silent Key (a deceased radio amateur)
SKED	Schedule
SMS	Short message service
SN	Soon
SNR	Signal-to-noise ratio
SRI	Sorry
SSB	Single sideband
STN	Station
T	Zero (usually an elongated dah)
TEMP	Temperature
TFC	Traffic
TKS	Thanks
TMW	Tomorrow
TNX	Thanks
TT	That
TU	Thank you
TVI	Television interference
TX	Transmit, transmitter
TXT	Text
U	You
UR	Your or You're (depending on context)
URS	Yours
VX	Voice; phone
VY	Very
W	Watts
WA	Word after
WB	Word before
WC	Wilco
WDS	Words
WID	With
WKD	Worked
WKG	Working
WL	Will
WUD	Would
WTC	Whats the craic? (Irish Language: [Conas atá tú?])
WX	Weather
XCVR	Transceiver
XMTR	Transmitter
XYL	Wife (ex-YL)
YF	Wife
YL	Young lady (originally an unmarried female operator, now used for any female)
ZX	Zero beat
73	Best regards
88	Love and kisses
[edit]

*/

/**
 * cw_tokenize_representation()
 *
 * Return a token value, in the range 2-255, for a lookup table representation.
 * The routine returns 0 if no valid token could be made from the string.  To
 * avoid casting the value a lot in the caller (we want to use it as an array
 * index), we actually return an unsigned int.
 *
 * This token algorithm is designed ONLY for valid CW representations; that is,
 * strings composed of only '.' and '-', and in this case, strings shorter than
 * eight characters.  The algorithm simply turns the representation into a
 * 'bitmask', based on occurrences of '.' and '-'.  The first bit set in the
 * mask indicates the start of data (hence the 7-character limit).  This mask
 * is viewable as an integer in the range 2 (".") to 255 ("-------"), and can
 * be used as an index into a fast lookup array.
 */
quint8 MorseCode::tokenizeSymbol(const char *symbol)
{
    unsigned int token;	/* Return token value */
    const char *sptr;	/* Pointer through string */

    /*
     * Our algorithm can handle only 6 characters of representation.
     * And we insist on there being at least one character, too.
     */
	if (strlen(symbol) > 6 || strlen(symbol) < 1)
        return 0;

    /*
     * Build up the token value based on the dots and dashes.  Start the
     * token at 1 - the sentinel (start) bit.
     */
	for (sptr = symbol, token = 1; *sptr != 0; sptr++) {
        /*
         * Left-shift the sentinel (start) bit.
         */
        token <<= 1;

        /*
         * If the next element is a dash, OR in another bit.  If it is
         * not a dash or a dot, then there is an error in the repres-
         * entation string.
         */
		if (*sptr == c_dashChar)
            token |= 1;
		else if (*sptr != c_dotChar)
            return 0;
    }

    /* Return the value resulting from our tokenization of the string. */
    return token;
}

/* ---------------------------------------------------------------------- */

MorseCode::MorseCode() {
	init();
}

MorseCode::~MorseCode() {
}

void MorseCode::init()
{
	MorseSymbol *symbol;
	quint8 token;

    // Clear the RX & TX tables
	for (quint32 i = 0; i < c_morseTableSize; i++) {
		m_rxLookup[i] = 0; //Flag invalid entry
		m_txLookup[i] = 0;
    }

	// For each main table entry, create a pointer to symbol in rxLookup in token order
	//Then we can do fast lookup by morse token in rxLookup
	for (symbol = m_morseTable; symbol->ascii != 0; symbol++) {
		if ((symbol->ascii == '(') && !c_useParen)
			continue;
		if ((symbol->ascii == '<') && c_useParen)
			continue;
		//tokens start with a 1 bit followed by 1's for dash and 0's for dots
		token = tokenizeSymbol(symbol->dotDash);
		if (token != 0) {
			symbol->token = token;
			//In token order
			if (m_rxLookup[token] != 0) {
				//We already have an entry, skip this one and output warning
				qWarning()<<"Duplicate token in morse table for -"<<symbol->ascii<<" "<<symbol->display;
			} else {
				m_rxLookup[token] = symbol;
			}
			//In character order
			if (m_txLookup[symbol->ascii] != 0) {
				//We already have an entry, skip this one and output warning
				qWarning()<<"Duplicate ascii in morse table for -"<<symbol->ascii<<" "<<symbol->display;
			} else {
				m_txLookup[symbol->ascii] = symbol;
			}
		}
    }
}

//Return CW_TABLE element so caller can access anything from it, not just display
MorseSymbol *MorseCode::rxLookup(char *r)
{
    int			token;
	MorseSymbol *cw;

	if ((token = tokenizeSymbol(r)) == 0)
        return NULL;

	if ((cw = m_rxLookup[token]) == NULL)
        return NULL;

    return cw;
}

quint8 MorseCode::txLookup(char c)
{
	return m_txLookup[toupper(c)]->token;
}

MorseSymbol *MorseCode::tableLookup(quint32 index)
{
	if (index >= c_morseTableSize)
		return NULL;
	MorseSymbol *symbol = &m_morseTable[index];
	return symbol;
}

const char *MorseCode::txPrint(char c)
{
	MorseSymbol *symbol = m_txLookup[toupper(c)];
	if (symbol != NULL)
		return symbol->display;
    else
		return "";
}

quint32 MorseCode::wpmToTcwMs(quint32 wpm)
{
	//quint32 tcw = 60.0 / (wpm * 50) * 1000;
	quint32 tcw = c_mSecDotMagic / wpm;
	return tcw;
}

quint32 MorseCode::tcwMsToWpm(quint32 msTcw)
{
	quint32 wpm = c_mSecDotMagic / msTcw;
	return wpm;
}

// Usec = DM / WPM
// DM = WPM * Usec
// WPM = DM / Usec
quint32 MorseCode::wpmToTcwUsec(quint32 wpm)
{
	return c_uSecDotMagic / wpm;
}

quint32  MorseCode::tcwUsecToWpm(quint32 tcwUsec)
{
	return c_uSecDotMagic / tcwUsec;
}




