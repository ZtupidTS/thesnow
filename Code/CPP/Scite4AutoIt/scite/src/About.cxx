﻿// SciTE - Scintilla based Text Editor
/** @file About.cxx
 ** About SciTe editor.
 **/
// Copyright 1998-2010 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

/*
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/stat.h>
*/
#include <time.h>
#include "About.h"
#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif

//#include <string>
#include <vector>
#include <map>
//#include <algorithm>

#if !defined(GTK)
#undef _WIN32_WINNT
#define _WIN32_WINNT  0x0500
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

#include "Scintilla.h"
#include "SciLexer.h"

#include "GUI.h"
#include "SString.h"
#include "StringList.h"
#include "StringHelpers.h"
#include "FilePath.h"
#include "PropSetFile.h"
#include "StyleWriter.h"
#include "Extender.h"
#include "SciTE.h"
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
#if defined(GTK)
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
            "Haimag Ren",
            "Andrey Moskalyov",
            "Xavi",
            "Toby Inkster",
            "Eric Forgeot",
            "Colomban Wendling",
            "Neo",
            "Jordan Russell",
            "Farshid Lashkari",
            "Sam Rawlins",
            "Michael Mullin",
            "Carlos SS",
            "vim",
            "Martial Demolins",
            "Tino Weinkauf",
            "J\xc3\xa9r\xc3\xb4me Laforge",
            "Udo Lechner",
            "Marco Falda",
            "Dariusz Knoci\xc5\x84ski",
            "Ben Fisher",
        };

// AddStyledText only called from About so static size buffer is OK
void AddStyledText(GUI::ScintillaWindow &wsci, const char *s, int attr) {
	char buf[1000];
	size_t len = strlen(s);
	for (size_t i = 0; i < len; i++) {
		buf[i*2] = s[i];
		buf[i*2 + 1] = static_cast<char>(attr);
	}
	wsci.SendPointer(SCI_ADDSTYLEDTEXT,
	        static_cast<int>(len*2), const_cast<char *>(buf));
}

void SetAboutStyle(GUI::ScintillaWindow &wsci, int style, Colour fore) {
	wsci.Send(SCI_STYLESETFORE, style, fore);
}

static void HackColour(int &n) {
	n += (rand() % 100) - 50;
	if (n > 0xE7)
		n = 0x60;
	if (n < 0)
		n = 0x80;
}


SString SciTEBase::GetTranslationToAbout(const char * const propname, bool retainIfNotFound) {
#if !defined(GTK)
	return SString(GUI::UTF8FromString(localiser.Text(propname, retainIfNotFound)).c_str());
#else
	// On GTK+, localiser.Text always converts to UTF-8.
	return SString(localiser.Text(propname, retainIfNotFound).c_str());
#endif
}

void SciTEBase::SetAboutMessage(GUI::ScintillaWindow &wsci, const char *appTitle) {
	if (wsci.Created()) {
		wsci.Send(SCI_SETSTYLEBITS, 7, 0);
		wsci.Send(SCI_STYLERESETDEFAULT, 0, 0);
		int fontSize = 15;
#if defined(GTK)
		wsci.Send(SCI_STYLESETFONT, STYLE_DEFAULT,
		        reinterpret_cast<uptr_t>("!Serif"));
		fontSize = 14;
#endif

		wsci.Send(SCI_SETCODEPAGE, SC_CP_UTF8, 0);

		wsci.Send(SCI_STYLESETSIZE, STYLE_DEFAULT, fontSize);
		wsci.Send(SCI_STYLESETBACK, STYLE_DEFAULT, ColourRGB(0xff, 0xff, 0xff));
		wsci.Send(SCI_STYLECLEARALL, 0, 0);

		SetAboutStyle(wsci, 0, ColourRGB(0xff, 0xff, 0xff));
		wsci.Send(SCI_STYLESETSIZE, 0, fontSize);
		wsci.Send(SCI_STYLESETBACK, 0, ColourRGB(0, 0, 0x80));
		AddStyledText(wsci, appTitle, 0);
		AddStyledText(wsci, "\n", 0);
		SetAboutStyle(wsci, 1, ColourRGB(0, 0, 0));
		int trsSty = 5; // define the stylenumber to assign font for translators.
		SString translator = GetTranslationToAbout("TranslationCredit", false);
		SetAboutStyle(wsci, trsSty, ColourRGB(0, 0, 0));
#if !defined(GTK)
		// On Windows Me (maybe 9x also), we must assign another font to display translation.
		if (translator.length()) {
			SString fontBase = props.GetExpanded("font.translators");
			StyleDefinition sd(fontBase.c_str());
			if (sd.specified & StyleDefinition::sdFont) {
				wsci.Send(SCI_STYLESETFONT, trsSty,
				        reinterpret_cast<uptr_t>(sd.font.c_str()));
			}
			if (sd.specified & StyleDefinition::sdSize) {
				wsci.Send(SCI_STYLESETSIZE, trsSty, sd.size);
			}
		}
#endif
		AddStyledText(wsci, GetTranslationToAbout("Version").c_str(), trsSty);
		AddStyledText(wsci, " 2.25\n", 1);
		AddStyledText(wsci, " Build On: " __DATE__ " " __TIME__ "\n", 1);
		SetAboutStyle(wsci, 2, ColourRGB(0, 0, 0));
		wsci.Send(SCI_STYLESETITALIC, 2, 1);
		AddStyledText(wsci, GetTranslationToAbout("by").c_str(), trsSty);
		AddStyledText(wsci, " Neil Hodgson.\n", 2);
		SetAboutStyle(wsci, 3, ColourRGB(0, 0, 0));
		AddStyledText(wsci, "December 1998-March 2011.\n", 3);
		SetAboutStyle(wsci, 4, ColourRGB(0, 0x7f, 0x7f));
		AddStyledText(wsci, "http://www.scintilla.org\n", 4);
		AddStyledText(wsci, "Lua scripting language by TeCGraf, PUC-Rio\n", 3);
		AddStyledText(wsci, "    http://www.lua.org\n", 4);
		SetAboutStyle(wsci, 5, ColourRGB(0, 0, 0));
		AddStyledText(wsci, "This Chinese version by thesnoW\n", 3);
#ifdef AUTOIT
		AddStyledText(wsci, "    http://www.AutoitX.com\n", 4);
#else
		AddStyledText(wsci, "    thesnoW@QQ.com\n", 4);
#endif
		if (translator.length()) {
			AddStyledText(wsci, translator.c_str(), trsSty);
			AddStyledText(wsci, "\n", 5);
		}
		AddStyledText(wsci, GetTranslationToAbout("Contributors:").c_str(), trsSty);
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
			SetAboutStyle(wsci, sty + 50, ColourRGB(r, g, b));
		}
		wsci.Send(SCI_SETREADONLY, 1, 0);
	}
}
/*
#if !defined(GTK)
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
*/

void DONATE_MSG(){
		TCHAR *msg = 
			TEXT("如果您愿意捐助thesnoW的汉化/论坛(硬盘或者RMB),请联系:\n")
			TEXT("thegfw#Gmail.com,thesnoW#QQ.com\n\n")
			TEXT("当然,互联网是免费的.我知道您是不小心点到这个对话框的.囧.\n");
		MessageBox(0, msg,TEXT("我点到什么东西了?"), MB_OK | MB_ICONWARNING);
};