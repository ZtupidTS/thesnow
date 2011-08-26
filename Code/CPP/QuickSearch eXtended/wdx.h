// wdx_proxy.h: wdx support for files

#pragma once

// options
const wstring TimeSeparator=L":";
const wstring DateSeparator=L".";

// global variables
const int BSize=32768;
static int DebugOutput=0;
static int CacheMax=0;
static WCHAR wcBuffer[BSize+2];
static char cBuffer[BSize+1];
static char cBuffer2[BSize+1];
static wstring wsIniPath;
static wstring wsIniFile;
static wstring wsOutputFile;
static map<WCHAR,wstring> ReplaceChars;

// useful functions
void DebugMode(int Status,int Cache_Value);
int DebugLevel(int PriorityLevel);
void DebugMessage(const wstring &Text);
void DebugMessage(const string  &Text);
void DebugMessage(int           Value);
void DebugMessage(double        Value);
void MeasureTime(int State=2);
wstring& C_to_wString(wstring Pattern,...);
void C_wsString_to_lower(wstring &Value);
void replace_chars_clear();
int replace_chars_size();
void replace_chars_add(WCHAR character,wstring Text);
void C_wsString_replace_chars(wstring &Value);
wstring getIniPath(HMODULE hModule);
wstring& StringSplit(wstring &TextToSplit,const wstring &splitText,int splitLast=0);

// defines of wdx plugins
typedef struct{ int size; DWORD PluginVersionLow; DWORD PluginVersionHi; char DefaultIniName[MAX_PATH]; }VersionInfo;
typedef struct{ WORD Year; WORD Month;  WORD Day;   } Dateformat;
typedef struct{ WORD Hour; WORD Minute; WORD Second;} Timeformat;

#define ft_nomorefields     0
#define ft_numeric_32       1
#define ft_numeric_64       2
#define ft_numeric_floating 3
#define ft_date             4
#define ft_time             5
#define ft_boolean          6
#define ft_multiplechoice   7
#define ft_string           8
#define ft_fulltext         9
#define ft_datetime        10
#define ft_stringw         11

typedef void (__stdcall *T_ContentSetDefaultParams)     (VersionInfo* dps);
typedef int  (__stdcall *T_ContentGetSupportedField)    (int FieldIndex,char* FieldName,char* Units,int maxlen);
typedef int  (__stdcall *T_ContentGetValue)             (char* FileName,int FieldIndex,int UnitIndex,void* FieldValue,int maxlen,int flags);
typedef int  (__stdcall *T_ContentGetValueW)            (WCHAR* FileName,int FieldIndex,int UnitIndex,void* FieldValue,int maxlen,int flags);
typedef void (__stdcall *T_ContentSendStateInformation) (int state,char* path);
typedef void (__stdcall *T_ContentSendStateInformationW)(int state,WCHAR* path);
typedef void (__stdcall *T_ContentPluginUnloading)      ();

// wdx class
class wdx_class{
public:
 int size;
 std::vector< vector<int> > Groups; 
 
 wdx_class();                                                                  // Constructor
 ~wdx_class();                                                                 // Destructor

 void LoadFromIni();                                                           // Load the wdx plugins from ini file
 void Create_wdxStrings(const wstring &wsFilename,int INI_case_sensitive);     // Gets the wdx strings for a file
 wstring Get_wdxStrings(int n);                                                // Gets a specific wdx string
 void Free();                                                                  // Free the wdx plugins

private:
 int ContentGetValue(int n,int Unitindex);
 int LoadEntryFromIni(int Entry,int &LibraryCount,int &FieldCount);

 struct wdx_field_type{
  wstring Name,Fieldname;
  string Units;
  int PluginIndex,FieldIndex,FieldType,Init;
 };

 struct wdx_plugin_type{
  wstring Name;
  HMODULE Lib;
  T_ContentSetDefaultParams      F_ContentSetDefaultParams;
  T_ContentGetSupportedField     F_ContentGetSupportedField;
  T_ContentGetValue              F_ContentGetValue;
  T_ContentGetValueW             F_ContentGetValueW;
  T_ContentSendStateInformation  F_ContentSendStateInformation;
  T_ContentSendStateInformationW F_ContentSendStateInformationW;
  T_ContentPluginUnloading       F_ContentPluginUnloading;
 };

 struct cache_type{
  wstring Name;
  int Init;
 };

 wstring wsLastPath;
 wstring wsLastFile;
 int LastCaseSensitive;
 vector<wdx_field_type> wdx_field;
 vector<wdx_plugin_type> wdx_plugin;

 map<wstring,vector<cache_type> > Cache;
 list<wstring> CacheOrder;
};