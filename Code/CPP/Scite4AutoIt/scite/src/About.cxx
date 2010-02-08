
#include "About.h"

// SciTE - Scintilla based Text Editor
/** @file SciTEBase.cxx
 ** Platform independent base class of editor.
 **/
// Copyright 1998-2007 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <time.h>

#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif

#include <string>
#include <map>

#include "Platform.h"

#if PLAT_WIN
#ifdef _MSC_VER
// windows.h, et al, use a lot of nameless struct/unions - can't fix it, so allow it
#pragma warning(disable: 4201)
#endif
#include <windows.h>
#ifdef _MSC_VER
// okay, that's done, don't allow it in our code
#pragma warning(default: 4201)
#endif
#include <commctrl.h>

#ifdef _MSC_VER
#include <direct.h>
#endif
#endif

#include "SciTE.h"
#include "PropSet.h"
#include "SString.h"
#include "StringList.h"
//#include "Accessor.h"
//#include "WindowAccessor.h"
#include "Scintilla.h"
#include "ScintillaWidget.h"
#include "SciLexer.h"
#include "Extender.h"
#include "FilePath.h"
#include "PropSetFile.h"
#include "Mutex.h"
#include "JobQueue.h"
#include "SciTEBase.h"


// 捐助者名称(UTF-8编码)
const char *contributors[] = {
            "Atsuo Ishimoto",
            "Mark Hammond",
            "Francois Le Coguiec",
            "Dale Nagata",
            "Ralf Reinhardt",
            "Philippe Lhoste",
            "Andrew McKinlay",
            "Stephan R. A. Deibel",
            "Hans Eckardt",
            "Vassili Bourdo",
            "Maksim Lin",
            "Robin Dunn",
            "John Ehresman",
            "Steffen Goeldner",
            "Deepak S.",
            "DevelopMentor http://www.develop.com",
            "Yann Gaillard",
            "Aubin Paul",
            "Jason Diamond",
            "Ahmad Baitalmal",
            "Paul Winwood",
            "Maxim Baranov",
#if PLAT_GTK
            "Icons Copyright(C) 1998 by Dean S. Jones",
            "    http://jfa.javalobby.org/projects/icons/",
#endif
            "Ragnar H\xc3\xb8jland",
            "Christian Obrecht",
            "Andreas Neukoetter",
            "Adam Gates",
            "Steve Lhomme",
            "Ferdinand Prantl",
            "Jan Dries",
            "Markus Gritsch",
            "Tahir Karaca",
            "Ahmad Zawawi",
            "Laurent le Tynevez",
            "Walter Braeu",
            "Ashley Cambrell",
            "Garrett Serack",
            "Holger Schmidt",
            "ActiveState http://www.activestate.com",
            "James Larcombe",
            "Alexey Yutkin",
            "Jan Hercek",
            "Richard Pecl",
            "Edward K. Ream",
            "Valery Kondakoff",
            "Sm\xc3\xa1ri McCarthy",
            "Clemens Wyss",
            "Simon Steele",
            "Serge A. Baranov",
            "Xavier Nodet",
            "Willy Devaux",
            "David Clain",
            "Brendon Yenson",
            "Vamsi Potluru http://www.baanboard.com",
            "Praveen Ambekar",
            "Alan Knowles",
            "Kengo Jinno",
            "Valentin Valchev",
            "Marcos E. Wurzius",
            "Martin Alderson",
            "Robert Gustavsson",
            "Jos\xc3\xa9 Fonseca",
            "Holger Kiemes",
            "Francis Irving",
            "Scott Kirkwood",
            "Brian Quinlan",
            "Ubi",
            "Michael R. Duerig",
            "Deepak T",
            "Don Paul Beletsky",
            "Gerhard Kalab",
            "Olivier Dagenais",
            "Josh Wingstrom",
            "Bruce Dodson",
            "Sergey Koshcheyev",
            "Chuan-jian Shen",
            "Shane Caraveo",
            "Alexander Scripnik",
            "Ryan Christianson",
            "Martin Steffensen",
            "Jakub Vr\xc3\xa1na",
            "The Black Horus",
            "Bernd Kreuss",
            "Thomas Lauer",
            "Mike Lansdaal",
            "Yukihiro Nakai",
            "Jochen Tucht",
            "Greg Smith",
            "Steve Schoettler",
            "Mauritius Thinnes",
            "Darren Schroeder",
            "Pedro Guerreiro",
            "Steven te Brinke",
            "Dan Petitt",
            "Biswapesh Chattopadhyay",
            "Kein-Hong Man",
            "Patrizio Bekerle",
            "Nigel Hathaway",
            "Hrishikesh Desai",
            "Sergey Puljajev",
            "Mathias Rauen",
            "Angelo Mandato http://www.spaceblue.com",
            "Denis Sureau",
            "Kaspar Schiess",
            "Christoph H\xc3\xb6sler",
            "Jo\xc3\xa3o Paulo F Farias",
            "Ron Schofield",
            "Stefan Wosnik",
            "Marius Gheorghe",
            "Naba Kumar",
            "Sean O'Dell",
            "Stefanos Togoulidis",
            "Hans Hagen",
            "Jim Cape",
            "Roland Walter",
            "Brian Mosher",
            "Nicholas Nemtsev",
            "Roy Wood",
            "Peter-Henry Mander",
            "Robert Boucher",
            "Christoph Dalitz",
            "April White",
            "S. Umar",
            "Trent Mick",
            "Filip Yaghob",
            "Avi Yegudin",
            "Vivi Orunitia",
            "Manfred Becker",
            "Dimitris Keletsekis",
            "Yuiga",
            "Davide Scola",
            "Jason Boggs",
            "Reinhold Niesner",
            "Jos van der Zande",
            "Pescuma",
            "Pavol Bosik",
            "Johannes Schmid",
            "Blair McGlashan",
            "Mikael Hultgren",
            "Florian Balmer",
            "Hadar Raz",
            "Herr Pfarrer",
            "Ben Key",
            "Gene Barry",
            "Niki Spahiev",
            "Carsten Sperber",
            "Phil Reid",
            "Iago Rubio",
            "R\xc3\xa9gis Vaquette",
            "Massimo Cor\xc3\xa0",
            "Elias Pschernig",
            "Chris Jones",
            "Josiah Reynolds",
            "Robert Roessler http://www.rftp.com",
            "Steve Donovan",
            "Jan Martin Pettersen",
            "Sergey Philippov",
            "Borujoa",
            "Michael Owens",
            "Franck Marcia",
            "Massimo Maria Ghisalberti",
            "Frank Wunderlich",
            "Josepmaria Roca",
            "Tobias Engvall",
            "Suzumizaki Kimitaka",
            "Michael Cartmell",
            "Pascal Hurni",
            "Andre",
            "Randy Butler",
            "Georg Ritter",
            "Michael Goffioul",
            "Ben Harper",
            "Adam Strzelecki",
            "Kamen Stanev",
            "Steve Menard",
            "Oliver Yeoh",
            "Eric Promislow",
            "Joseph Galbraith",
            "Jeffrey Ren",
            "Armel Asselin",
            "Jim Pattee",
            "Friedrich Vedder",
            "Sebastian Pipping",
            "Andre Arpin",
            "Stanislav Maslovski",
            "Martin Stone",
            "Fabien Proriol",
            "mimir",
            "Nicola Civran",
            "Snow",
            "Mitchell Foral",
            "Pieter Holtzhausen",
            "Waldemar Augustyn",
            "Jason Haslam",
            "Sebastian Steinlechner",
            "Chris Rickard",
            "Rob McMullen",
            "Stefan Schwendeler",
            "Cristian Adam",
            "Nicolas Chachereau",
            "Istvan Szollosi",
            "Xie Renhui",
            "Enrico Tr\xc3\xb6ger",
            "Todd Whiteman",
            "Yuval Papish",
            "instanton",
            "Sergio Lucato",
            "VladVRO",
            "Dmitry Maslov",
            "chupakabra",
            "Juan Carlos Arevalo Baeza",
            "Nick Treleaven",
            "Stephen Stagg",
            "Jean-Paul Iribarren",
            "Tim Gerundt",
            "Sam Harwell",
            "Boris",
            "Jason Oster",
            "Gertjan Kloosterman",
            "alexbodn",
            "Sergiu Dotenco",
            "Anders Karlsson",
            "ozlooper",
            "Marko Njezic",
            "Eugen Bitter",
            "Christoph Baumann",
            "Christopher Bean",
            "Sergey Kishchenko",
            "Kai Liu",
            "Andreas Rumpf",
            "James Moffatt",
            "Yuzhou Xin",
            "Nic Jansma",
            "Evan Jones",
            "Mike Lischke",
            "Eric Kidd",
            "maXmo",
            "David Severwright",
            "Jon Strait",
            "Oliver Kiddle",
            "Etienne Girondel",
        };





// AddStyledText only called from About so static size buffer is OK
void AddStyledText(WindowID hwnd, const char *s, int attr) {
	char buf[1000];
	size_t len = strlen(s);
	for (size_t i = 0; i < len; i++) {
		buf[i*2] = s[i];
		buf[i*2 + 1] = static_cast<char>(attr);
	}
	Platform::SendScintillaPointer(hwnd, SCI_ADDSTYLEDTEXT,
	        static_cast<int>(len*2), const_cast<char *>(buf));
}

// AddStyledText only called from About so static size buffer is OK
void AddStyledText(WindowID hwnd, const wchar_t *s, int attr) {		//added
	wchar_t buf[1000];
	size_t len = wcslen(s);
	for (size_t i = 0; i < len; i++) {
		buf[i*2] = s[i];
		buf[i*2 + 1] = static_cast<wchar_t>(attr);
	}
	Platform::SendScintillaPointer(hwnd, SCI_ADDSTYLEDTEXT,
	        static_cast<int>(len*2), const_cast<wchar_t *>(buf));
}


#if PLAT_WIN
static unsigned int UTF8Length(const wchar_t *uptr, unsigned int tlen) {
	unsigned int len = 0;
	for (unsigned int i = 0; i < tlen && uptr[i]; i++) {
		unsigned int uch = uptr[i];
		if (uch < 0x80)
			len++;
		else if (uch < 0x800)
			len += 2;
		else
			len += 3;
	}
	return len;
}

static void UTF8FromUCS2(const wchar_t *uptr, unsigned int tlen, char *putf, unsigned int len) {
	int k = 0;
	for (unsigned int i = 0; i < tlen && uptr[i]; i++) {
		unsigned int uch = uptr[i];
		if (uch < 0x80) {
			putf[k++] = static_cast<char>(uch);
		} else if (uch < 0x800) {
			putf[k++] = static_cast<char>(0xC0 | (uch >> 6));
			putf[k++] = static_cast<char>(0x80 | (uch & 0x3f));
		} else {
			putf[k++] = static_cast<char>(0xE0 | (uch >> 12));
			putf[k++] = static_cast<char>(0x80 | ((uch >> 6) & 0x3f));
			putf[k++] = static_cast<char>(0x80 | (uch & 0x3f));
		}
	}
	putf[len] = '\0';
}
#endif

void SetAboutStyle(WindowID wsci, int style, ColourDesired fore) {
	Platform::SendScintilla(wsci, SCI_STYLESETFORE, style, fore.AsLong());
}

SString SciTEBase::GetTranslationToAbout(const char * const propname, bool retainIfNotFound) {
#if PLAT_WIN
	// By code below, all translators can write their name in their own
	// language in locale.properties on Windows.
	SString result = localiser.Text(propname, retainIfNotFound);
	if (!result.length())
		return result;
	int translationCodePage = props.GetInt("code.page", CP_ACP);
	int bufwSize = ::MultiByteToWideChar(translationCodePage, MB_PRECOMPOSED, result.c_str(), -1, NULL, 0);
	if (!bufwSize)
		return result;
	wchar_t *bufw = new wchar_t[bufwSize+1];
	bufwSize = ::MultiByteToWideChar(translationCodePage, MB_PRECOMPOSED, result.c_str(), -1, bufw, bufwSize);
	if (!bufwSize) {
		delete []bufw;
		return result;
	}
	int bufcSize = UTF8Length(bufw, bufwSize);
	if (!bufcSize)
		return result;
	char *bufc = new char[bufcSize+1];
	UTF8FromUCS2(bufw, bufwSize, bufc, bufcSize);
	delete []bufw;
	result = bufcSize ? bufc : "";
	delete []bufc;
	return result;
#else
	// 在 GTK+ 上, localiser.Text 总是转换为 UTF-8.
	return localiser.Text(propname, retainIfNotFound);
#endif
}

static void HackColour(int &n) {
	n += (rand() % 100) - 50;
	if (n > 0xE7)
		n = 0x60;
	if (n < 0)
		n = 0x80;
}

void SciTEBase::SetAboutMessage(WindowID wsci, const char *appTitle) {
	if (wsci) {
		Platform::SendScintilla(wsci, SCI_SETSTYLEBITS, 7, 0);
		Platform::SendScintilla(wsci, SCI_STYLERESETDEFAULT, 0, 0);
		int fontSize = 15;
#if PLAT_GTK
#if GTK_MAJOR_VERSION == 1
		// On GTK+ 1.x, try a font set that may allow unicode display
		Platform::SendScintilla(wsci, SCI_STYLESETFONT, STYLE_DEFAULT,
		        reinterpret_cast<uptr_t>("misc-fixed-iso10646-1,*"));
#else
		Platform::SendScintilla(wsci, SCI_STYLESETFONT, STYLE_DEFAULT,
		        reinterpret_cast<uptr_t>("!Serif"));
#endif
		fontSize = 14;
#endif

		Platform::SendScintilla(wsci, SCI_SETCODEPAGE, SC_CP_UTF8, 0);

		Platform::SendScintilla(wsci, SCI_STYLESETSIZE, STYLE_DEFAULT, fontSize);
		Platform::SendScintilla(wsci, SCI_STYLESETBACK, STYLE_DEFAULT, ColourDesired(0xff, 0xff, 0xff).AsLong());
		Platform::SendScintilla(wsci, SCI_STYLECLEARALL, 0, 0);

		SetAboutStyle(wsci, 0, ColourDesired(0xff, 0xff, 0xff));
		Platform::SendScintilla(wsci, SCI_STYLESETSIZE, 0, fontSize);
		Platform::SendScintilla(wsci, SCI_STYLESETBACK, 0, ColourDesired(0, 0, 0x80).AsLong());
		AddStyledText(wsci, appTitle, 0);
		AddStyledText(wsci, "\n", 0);
		SetAboutStyle(wsci, 1, ColourDesired(0, 0, 0));
		int trsSty = 5; // define the stylenumber to assign font for translators.
		SString translator = GetTranslationToAbout("TranslationCredit", false);
		SetAboutStyle(wsci, trsSty, ColourDesired(0, 0, 0));
#if PLAT_WIN
		// On Windows Me (maybe 9x also), we must assign another font to display translation.
		if (translator.length()) {
			SString fontBase = props.GetExpanded("font.translators");
			StyleDefinition sd(fontBase.c_str());
			if (sd.specified & StyleDefinition::sdFont) {
				Platform::SendScintilla(wsci, SCI_STYLESETFONT, trsSty,
				        reinterpret_cast<uptr_t>(sd.font.c_str()));
			}
			if (sd.specified & StyleDefinition::sdSize) {
				Platform::SendScintilla(wsci, SCI_STYLESETSIZE, trsSty, sd.size);
			}
		}
#endif
		AddStyledText(wsci, GetTranslationToAbout("汉化增强版本").c_str(), trsSty);
		AddStyledText(wsci, " 2.02\n", 1);
		AddStyledText(wsci, " Build: " __DATE__ " " __TIME__ "\n", 1);
		SetAboutStyle(wsci, 2, ColourDesired(0, 0, 0));
		Platform::SendScintilla(wsci, SCI_STYLESETITALIC, 2, 1);
		AddStyledText(wsci, GetTranslationToAbout("by").c_str(), trsSty);
		AddStyledText(wsci, " Neil Hodgson.\n", 2);
		SetAboutStyle(wsci, 3, ColourDesired(0, 0, 0));
		AddStyledText(wsci, "December 1998-January 2010.\n", 3);
		SetAboutStyle(wsci, 4, ColourDesired(0, 0x7f, 0x7f));
		AddStyledText(wsci, "http://www.scintilla.org\n", 4);
		AddStyledText(wsci, "Lua scripting language by TeCGraf, PUC-Rio\n", 3);
		AddStyledText(wsci, "    http://www.lua.org\n", 4);
		SetAboutStyle(wsci, 5, ColourDesired(0, 0, 0));
		AddStyledText(wsci, "This Chinese version by thesnoW\n", 3);
#ifdef AUTOIT
		AddStyledText(wsci, "    http://www.autoitX.com\n", 4);
#else
		AddStyledText(wsci, "    thesnoW@QQ.com\n", 4);
#endif
		if (translator.length()) {
			AddStyledText(wsci, translator.c_str(), trsSty);
			AddStyledText(wsci, "\n", 5);
		}
		AddStyledText(wsci, GetTranslationToAbout("贡献者:").c_str(), trsSty);
		srand(static_cast<unsigned>(time(0)));
		for (unsigned int co = 0;co < ELEMENTS(contributors);co++) {
			int colourIndex = 50 + (co % 78);
			AddStyledText(wsci, "\n    ", colourIndex);
			AddStyledText(wsci, contributors[co], colourIndex);
		}
		int r = rand() % 256;
		int g = rand() % 256;
		int b = rand() % 256;
		for (unsigned int sty = 0;sty < 78; sty++) {
			HackColour(r);
			HackColour(g);
			HackColour(b);
			SetAboutStyle(wsci, sty + 50, ColourDesired(r, g, b));
		}
		Platform::SendScintilla(wsci, SCI_SETREADONLY, 1, 0);
	}
}

void DONATE_MSG(){
		wchar_t *msg = 
			 L"如果您愿意捐助thesnoW的汉化/论坛(硬盘或者RMB),请联系:\n"
			 L"thegfw#Gmail.com,thesnow#QQ.com\n\n"
			 L"┌───────────────────────────────────┐	\n"
			 L"  捐助thesnoW名单(汉化):				\n"
			 L"  KiwiCsj			30.00RMB			\n"
			 L"  大绯狼			30.00RMB			\n"
			 L"  卜一样的青年		30.00RMB			\n"
			 L"  什么也不懂		50.00RMB			\n"
			 L"  路人甲(匿名)		50.00RMB			\n"		
			 L"  nxbigdaddy		55.5RMB			\n"
			 L"  silentdream		100.00RMB			\n"
			 L"  &老刀			200.00RMB			\n"			
			 L"  o︻$▅▆▇◤		40GB+80GB HD	\n"
			 L"  fengwei646		100.00RMB	\n"
			 L"  最後の夢		200.00RMB	\n"
			 L"└───────────────────────────────────┘	\n\n"
			 L"┌───────────────────────────────────┐	\n"
			 L"  捐助ACN网站名单(服务器):			\n"
			 L"  gooker			100.00RMB			\n"
			 L"  Crafter			100.00RMB			\n"
			 L"  小可			100.00RMB			\n"
			 L"  o瀟灑..酷唲		100.00RMB			\n"
			 L"  Sanhen			500.00RMB			\n"
			 L"  特别鸣谢:		KYO/jack金枪鱼		\n"	
			 L"└───────────────────────────────────┘	\n";
		::MessageBoxW(0, msg,L"感谢你们!", MB_OK | MB_ICONWARNING);
};