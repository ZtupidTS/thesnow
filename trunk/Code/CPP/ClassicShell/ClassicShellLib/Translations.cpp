// Classic Shell (c) 2009-2011, Ivo Beltchev
// The sources for Classic Shell are distributed under the MIT open source license

#include "SettingsParser.h"

static CSettingsParser g_Translations;
static CSettingsParser g_TranslationOverrides;
static bool g_bRTL;

// Parses the settings from an ini file. Supports UTF16, UTF8 or ANSI files
// Use forceLang for force a specific language
void ParseTranslations( const wchar_t *fname, const wchar_t *forceLang )
{
	g_Translations.Reset();

	if (fname)
	{
		if (!g_Translations.LoadText(fname)) return;
		g_Translations.ParseText();
	}

	wchar_t languages[100]={0};
	if (forceLang && *forceLang)
	{
		int len=(int)wcslen(forceLang);
		if (len>5) len=5;
		memcpy(languages,forceLang,len*2);
		wcscpy_s(languages+len+1,10,L"default");
	}
	else
	{
		ULONG size=4; // up to 4 languages
		ULONG len=_countof(languages);
		GetThreadPreferredUILanguages(MUI_LANGUAGE_NAME,&size,languages,&len);
		wcscpy_s(languages+len-1,10,L"default");
		languages[len+7]=0;
	}

	g_Translations.FilterLanguages(languages);

	// Checks for right-to-left languages
	g_bRTL=false;
	LOCALESIGNATURE localesig;
	LANGID language=GetUserDefaultUILanguage();
	if (forceLang && *forceLang)
	{
		if (GetLocaleInfoEx(forceLang,LOCALE_FONTSIGNATURE,(LPWSTR)&localesig,(sizeof(localesig)/sizeof(wchar_t))) && (localesig.lsUsb[3]&0x08000000))
			g_bRTL=true;
	}
	else
	{
		if (GetLocaleInfoW(language,LOCALE_FONTSIGNATURE,(LPWSTR)&localesig,(sizeof(localesig)/sizeof(wchar_t))) && (localesig.lsUsb[3]&0x08000000))
			g_bRTL=true;
	}
}

// Loads text overrides from the given module. They must be in a "L10N" resource with ID=1
void LoadTranslationOverrides( HMODULE hModule )
{
	HRSRC hResInfo=FindResource(hModule,MAKEINTRESOURCE(1),L"L10N");
	if (hResInfo)
	{
		g_TranslationOverrides.LoadText(hModule,hResInfo);
		g_TranslationOverrides.ParseText();
	}
}

// Returns a setting with the given name. If no setting is found, returns def
const wchar_t *FindTranslation( const wchar_t *name, const wchar_t *def )
{
	const wchar_t *str=g_TranslationOverrides.FindSetting(name);
	if (str) return str;
	return g_Translations.FindSetting(name,def);
}

// Checks for right-to-left languages
bool IsLanguageRTL( void )
{
	return g_bRTL;
}
