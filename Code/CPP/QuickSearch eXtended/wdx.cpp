// wdx_proxy.cpp: wdx support for files

#include "stdafx.h"
#include "wdx.h"

// ------------------------------------------------------------------------------------------------
// used to write the debug informations to a file
void DebugMode(int Status,int Cache_Value)
{
 DebugOutput=Status;
 CacheMax=Cache_Value;
}

int DebugLevel(int PriorityLevel)
{
 if(DebugOutput<PriorityLevel) return 0;
 return 1;
}

void DebugMessage(const wstring &wsText)
{
 string sText(wsText.begin(),wsText.end());
 fstream file;
 file.open(wsOutputFile.c_str(),ios::out|ios::app);
 file<<sText<<"\n";
 file.close();
}

void DebugMessage(const string &sText)
{
 fstream file;
 file.open(wsOutputFile.c_str(),ios::out|ios::app);
 file<<sText<<"\n";
 file.close();
}

void DebugMessage(int Value)
{
 fstream file;
 file.open(wsOutputFile.c_str(),ios::out|ios::app);
 file<<Value<<"\n";
 file.close();
}

void DebugMessage(double Value)
{
 fstream file;
 file.open(wsOutputFile.c_str(),ios::out|ios::app);
 file<<Value<<"\n";
 file.close();
}

void MeasureTime(int State)
{
 if(!DebugLevel(1)) return;
 static int Tcount;
 static LARGE_INTEGER T[50];
 if(State==1){
  Tcount=0;
  for(int i=0;i<50;i++) T[i].QuadPart=0;
  QueryPerformanceCounter(&T[Tcount++]);
 }else{
  QueryPerformanceCounter(&T[Tcount++]);
  if(State==3){
   LARGE_INTEGER Frequency;
   QueryPerformanceFrequency(&Frequency);
   DebugMessage("t");
   for(int i=0;i<(Tcount-1);i++){
    double Zeit=((double)T[i+1].QuadPart - (double)T[i].QuadPart)*1000000/Frequency.QuadPart;
    DebugMessage(Zeit);
   }
  }
 }
}

// ------------------------------------------------------------------------------------------------
// String convert functions
wstring& C_sString_to_wsString(const string &Value)
{
 if(MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,Value.c_str(),Value.length(),wcBuffer,BSize)){
  wstring *Result=new wstring(wcBuffer,(BSize-1>Value.length())?Value.length():BSize-1);
  return *Result;
 }
 wstring *Result=new wstring(Value.begin(),Value.end());
 return *Result;
}

string& C_wsString_to_sString(const wstring &Value)
{
 int CouldntTransform;
 if(WideCharToMultiByte(CP_ACP,0,Value.c_str(),Value.length(),cBuffer,BSize,"?",&CouldntTransform)){
  string *Result=new string(cBuffer,(BSize-1>Value.length())?Value.length():BSize-1);
  return *Result;
 }
 string *Result=new string(Value.begin(),Value.end());
 return *Result;
}

void C_sString_to_cString(char *Destination,const string &Value)
{
 strcpy(Destination,Value.c_str());
}

void C_wsString_to_wcString(WCHAR *Destination,const wstring &Value)
{
 wcscpy(Destination,Value.c_str());
}

void C_wsString_to_lower(wstring &Value)
{
 C_wsString_to_wcString(wcBuffer,Value);
 CharLowerW(wcBuffer);
 Value.assign(wcBuffer);
}

void replace_chars_clear()
{
 ReplaceChars.clear();
}

void replace_chars_add(WCHAR character,wstring Text)
{
 ReplaceChars.insert(make_pair(character,Text));
}

int replace_chars_size()
{
 return ReplaceChars.size();
}

void C_wsString_replace_chars(wstring &Value)
{
 for(int pos=0;pos<(int)Value.length();pos++){
  map<WCHAR,wstring>::const_iterator item(ReplaceChars.find(Value[pos]));
  if(item!=ReplaceChars.end()){
   Value.replace(pos,1,item->second);
   pos+=item->second.length()-1;
  }
 }
}

// ------------------------------------------------------------------------------------------------
// String convert functions
wstring& C_to_wString(wstring Pattern,...)
{
 va_list Arguments;
 va_start(Arguments,Pattern);
 _vsnwprintf(wcBuffer,BSize,Pattern.c_str(),Arguments);
 va_end(Arguments);
 wstring *Result=new wstring(wcBuffer);
 return *Result;
}

// ------------------------------------------------------------------------------------------------
// get the path of wincmd.ini and use for tcmatch.ini
wstring getIniPath(HMODULE hModule)
{
// get "wincmd.ini" path
 GetEnvironmentVariable(L"COMMANDER_INI",wcBuffer,BSize);
 wsIniPath.assign(wcBuffer);
 int Pos=wsIniPath.find_last_of('\\');
 if(Pos!=-1) wsIniPath=wsIniPath.substr(0,Pos+1);

// get "tcmatch.dll" path
 DWORD size=GetModuleFileName(hModule,wcBuffer,BSize);   
 wstring wsDllPath(wcBuffer);
 Pos=wsDllPath.find_last_of('\\');
 if(Pos!=-1) wsDllPath=wsDllPath.substr(0,Pos+1);

// first possible "tcmatch.ini" location -> "tcmatch.dll" path
 wsIniFile=wsDllPath+L"tcmatch.ini";
 DWORD Attr=GetFileAttributes(wsIniFile.c_str());
 if(Attr!=-1 && !(Attr & FILE_ATTRIBUTE_DIRECTORY)){
  wsOutputFile=wsDllPath+L"tcmatch.log";
  return wsIniFile;
 }

// second possible "tcmatch.ini" location -> "wincmd.ini" path
 wsIniFile=wsIniPath+L"tcmatch.ini";
 Attr=GetFileAttributes(wsIniFile.c_str());
 if(Attr!=-1 && !(Attr & FILE_ATTRIBUTE_DIRECTORY)){
  wsOutputFile=wsIniPath+L"tcmatch.log";
  return wsIniFile;
 }

// if there is no "tcmatch.ini" at all try to create it at "tcmatch.dll" path
 wsIniFile=wsDllPath+L"tcmatch.ini";
 fstream file;
 file.open(wsIniFile.c_str(),ios::out|ios::trunc);
 file<<"[general]";
 file.close();
 Attr=GetFileAttributes(wsIniFile.c_str());
 if(Attr!=-1 && !(Attr & FILE_ATTRIBUTE_DIRECTORY)){
  wsOutputFile=wsDllPath+L"tcmatch.log";
  return wsIniFile;
 }

// if everything fails create it at "wincmd.ini" path
 wsIniFile=wsIniPath+L"tcmatch.ini";
 wsOutputFile=wsIniPath+L"tcmatch.log";
 return wsIniFile;
}

// ------------------------------------------------------------------------------------------------
// used to split "TextToSplit" at "splitText" - return value holds left split, src holds right split
wstring& StringSplit(wstring &SplitText,const wstring &SplitChar,int SplitLastMatch)
{
 int Pos;
 if(SplitLastMatch) Pos=int(SplitText.find_last_of(SplitChar));
 else               Pos=int(SplitText.find(SplitChar));
 if(Pos==-1){
  wstring *temp=new wstring(SplitText.substr(0,SplitText.length()));
  SplitText=L"";
  return *temp;
 }
 wstring *temp=new wstring(SplitText.substr(0,Pos));
 SplitText=SplitText.substr(Pos+SplitChar.length(),SplitText.length()-Pos-SplitChar.length());
 return *temp;
}

// ------------------------------------------------------------------------------------------------
// Constructor and Destructor of wdx class
wdx_class::wdx_class()
{
 size=0;
 wsLastPath=L"";
}

wdx_class::~wdx_class()
{
 Free();
}

// ------------------------------------------------------------------------------------------------
// wdx class: load the wdx plugins to use from ini
int wdx_class::LoadEntryFromIni(int Entry,int &LibraryCount,int &FieldCount)
{
 wstring KeyIndex=C_to_wString(L"wdx_text_plugin%d",Entry);
 int OldFieldCount=FieldCount;

// try to load library
 if(GetPrivateProfileString(L"wdx",KeyIndex.c_str(),L"",wcBuffer,BSize,wsIniFile.c_str())==0) return -1;
 wdx_plugin.resize(++LibraryCount);
 wdx_plugin[LibraryCount-1].Name.assign(wcBuffer);
 wstring wsFieldnames=StringSplit(wdx_plugin[LibraryCount-1].Name,L"@");
 wdx_plugin[LibraryCount-1].Lib=LoadLibrary(wdx_plugin[LibraryCount-1].Name.c_str());
 if(!wdx_plugin[LibraryCount-1].Lib) return 0;
 StringSplit(wdx_plugin[LibraryCount-1].Name,L"\\",1);
 if(DebugLevel(2)) DebugMessage(L"\nLoading library: \""+wdx_plugin[LibraryCount-1].Name+L"\"");

// load functions from library
 wdx_plugin[LibraryCount-1].F_ContentSetDefaultParams     =(T_ContentSetDefaultParams)     GetProcAddress(wdx_plugin[LibraryCount-1].Lib,"ContentSetDefaultParams");
 wdx_plugin[LibraryCount-1].F_ContentGetSupportedField    =(T_ContentGetSupportedField)    GetProcAddress(wdx_plugin[LibraryCount-1].Lib,"ContentGetSupportedField");
 wdx_plugin[LibraryCount-1].F_ContentGetValue             =(T_ContentGetValue)             GetProcAddress(wdx_plugin[LibraryCount-1].Lib,"ContentGetValue");
 wdx_plugin[LibraryCount-1].F_ContentGetValueW            =(T_ContentGetValueW)            GetProcAddress(wdx_plugin[LibraryCount-1].Lib,"ContentGetValueW");
 wdx_plugin[LibraryCount-1].F_ContentSendStateInformation =(T_ContentSendStateInformation) GetProcAddress(wdx_plugin[LibraryCount-1].Lib,"ContentSendStateInformation");
 wdx_plugin[LibraryCount-1].F_ContentSendStateInformationW=(T_ContentSendStateInformationW)GetProcAddress(wdx_plugin[LibraryCount-1].Lib,"ContentSendStateInformationW");
 wdx_plugin[LibraryCount-1].F_ContentPluginUnloading      =(T_ContentPluginUnloading)      GetProcAddress(wdx_plugin[LibraryCount-1].Lib,"ContentPluginUnloading");

// give version info to library
 if(wdx_plugin[LibraryCount-1].F_ContentSetDefaultParams){
  VersionInfo Version;
  Version.PluginVersionHi=2;
  Version.PluginVersionLow=0;
  C_sString_to_cString(Version.DefaultIniName,C_wsString_to_sString(wsIniPath+L"contplug.ini"));
  Version.size=sizeof(Version);
  if(DebugLevel(3)) DebugMessage(L"Plugin: \""+wdx_plugin[LibraryCount-1].Name+L"\" calls Function: \"ContentSetDefaultParams\"");
  try       { wdx_plugin[LibraryCount-1].F_ContentSetDefaultParams(&Version);                                                   }
  catch(...){ if(DebugLevel(1)) DebugMessage(L"Error in Plugin: \""+wdx_plugin[LibraryCount-1].Name+L"\" calling \"ContentSetDefaultParams\""); }
 }

// get fields from library
 string sFieldnames;
 sFieldnames="|"+C_wsString_to_sString(wsFieldnames)+"|";
 if(wdx_plugin[LibraryCount-1].F_ContentGetSupportedField) for(int i=0;;i++){
  int Result;
  if(DebugLevel(3)) DebugMessage(L"Plugin: \""+wdx_plugin[LibraryCount-1].Name+L"\" calls Function: \"ContentGetSupportedField\"");
  try       { Result=wdx_plugin[LibraryCount-1].F_ContentGetSupportedField(i,cBuffer,cBuffer2,BSize);                                      }
  catch(...){ if(DebugLevel(1)) DebugMessage(L"Error in Plugin: \""+wdx_plugin[LibraryCount-1].Name+L"\" calling \"ContentGetSupportedField\""); continue; }
  if(Result==ft_nomorefields) break;
  string sCurrentName(cBuffer);
  if(sFieldnames.find("|"+sCurrentName+"|")==-1) continue; 
  if(DebugLevel(2)) DebugMessage(" +Field added: \""+sCurrentName+"\"");

  wdx_field.resize(++FieldCount);
  wdx_field[FieldCount-1].Init=0;
  wdx_field[FieldCount-1].FieldType=Result;
  wdx_field[FieldCount-1].FieldIndex=i;
  wdx_field[FieldCount-1].PluginIndex=LibraryCount-1;
  wdx_field[FieldCount-1].Units.assign(cBuffer2);
  wdx_field[FieldCount-1].Fieldname.assign(C_sString_to_wsString(sCurrentName));
 }
 return FieldCount-OldFieldCount;
}

// ------------------------------------------------------------------------------------------------
// wdx class: load the wdx plugins to use from ini and init the dll
void wdx_class::LoadFromIni()
{
 int LibraryCount=0,FieldCount=0,Result=0;
 Groups.resize(1);

// check if there exist wdx groups like: "1,2,3|7,8|6"
 if(GetPrivateProfileString(L"wdx",L"wdx_groups",L"",wcBuffer,BSize,wsIniFile.c_str())!=0){
  wstring IniText(wcBuffer);
  int i=0;
  while(IniText.length()){
   Groups.resize(++i);
   wstring IniTextGroup=StringSplit(IniText,L"|");
   while(IniTextGroup.length()){
    wstring IniTextNumber=StringSplit(IniTextGroup,L",");
    int Number=_wtoi(IniTextNumber.c_str());
    if(Number<=0) continue;
    Result=LoadEntryFromIni(Number,LibraryCount,FieldCount); // load ini entrys
    for(int j=0;j<Result;j++) Groups[i-1].push_back(FieldCount-Result+j);
   }
  }
 }else{ // load all Plugins to first group
  int i=0;
  while(Result!=-1){
   Result=LoadEntryFromIni(++i,LibraryCount,FieldCount); // load ini entrys
   for(int j=0;j<Result;j++) Groups[0].push_back(FieldCount-Result+j);
  }
 }
 size=FieldCount;
}

// ------------------------------------------------------------------------------------------------
// wdx class: announce new file for matching
void wdx_class::Create_wdxStrings(const wstring &wsFilename,int INI_case_sensitive)
{
// send directory change message
 wstring wsCurrentPath=StringSplit(wstring(wsFilename),L"\\",1);
 if(wsCurrentPath!=wsLastPath){
  wsLastPath=wsCurrentPath;
  Cache.clear();
  CacheOrder.clear();
  for(int i=0;i<(int)wdx_plugin.size();i++) if(wdx_plugin[i].Lib){
   if(wdx_plugin[i].F_ContentSendStateInformationW){
    C_wsString_to_wcString(wcBuffer,wsLastPath.c_str());
    if(DebugLevel(3)) DebugMessage(L"Plugin: \""+wdx_plugin[i].Name+L"\" calls Function: \"ContentSendStateInformationW\"");
    try       { wdx_plugin[i].F_ContentSendStateInformationW(1,wcBuffer);                                                 }
    catch(...){ if(DebugLevel(1)) DebugMessage(L"Error in Plugin: \""+wdx_plugin[i].Name+L"\" calling \"ContentSendStateInformationW\""); }
   }else if(wdx_plugin[i].F_ContentSendStateInformation){
    C_sString_to_cString(cBuffer,C_wsString_to_sString(wsLastPath).c_str());
    if(DebugLevel(3)) DebugMessage(L"Plugin: \""+wdx_plugin[i].Name+L"\" calls Function: \"ContentSendStateInformation\"");
    try       { wdx_plugin[i].F_ContentSendStateInformation(1,cBuffer);                                                  }
    catch(...){ if(DebugLevel(1)) DebugMessage(L"Error in Plugin: \""+wdx_plugin[i].Name+L"\" calling \"ContentSendStateInformation\""); }
   }
  }
 }

// write old values to cache
 if(wsLastFile!=L""){
  if((int)CacheOrder.size()>CacheMax){
   if(Cache.find(CacheOrder.front())!=Cache.end()) Cache.erase(Cache.find(CacheOrder.front()));
   CacheOrder.pop_front();
  }
  if(Cache.find(wsLastFile)!=Cache.end()){ // entry for this file already exists -> update
   for(int i=0;i<size;i++) if(wdx_field[i].Init){
    Cache[wsLastFile][i].Init=1;
    Cache[wsLastFile][i].Name=wdx_field[i].Name;
   }
  }else{                                   // no entry for this file available -> create
   CacheOrder.push_back(wsLastFile);
   vector<cache_type> Value;
   for(int i=0;i<size;i++){
    Value.resize(i+1);
    Value[i].Init=wdx_field[i].Init;
    Value[i].Name=wdx_field[i].Name;
   }
   Cache[wsLastFile]=Value;
  }
 }

// make field values invalid
 for(int i=0;i<size;i++) wdx_field[i].Init=0;
 wsLastFile=wsFilename;
 LastCaseSensitive=INI_case_sensitive;
}

// ------------------------------------------------------------------------------------------------
// wdx class: get field for file
int wdx_class::ContentGetValue(int n,int Unitindex)
{
 int Result=0;
 if(wdx_plugin[wdx_field[n].PluginIndex].F_ContentGetValueW){                 // plugin with Unicode filename support
  C_wsString_to_wcString(wcBuffer,wsLastFile.c_str());
  if(DebugLevel(3)) DebugMessage(L"Plugin: \""+wdx_plugin[wdx_field[n].PluginIndex].Name+L"\" calls Function: \"ContentGetValueW\"");
  try       { Result=wdx_plugin[wdx_field[n].PluginIndex].F_ContentGetValueW(wcBuffer,wdx_field[n].FieldIndex,Unitindex,cBuffer,BSize,0);     }
  catch(...){ if(DebugLevel(1)) DebugMessage(L"Error in Plugin: \""+wdx_plugin[wdx_field[n].PluginIndex].Name+L"\" calling \"ContentGetValueW\""); return -7; }
 }else if(wdx_plugin[wdx_field[n].PluginIndex].F_ContentGetValue){            // plugin without Unicode filename support
  if(GetShortPathNameW(wsLastFile.c_str(),wcBuffer,BSize)!=0) C_sString_to_cString(cBuffer2,C_wsString_to_sString(wstring(wcBuffer))); // convert to 8.3
  else                                                        C_sString_to_cString(cBuffer2,C_wsString_to_sString(wsLastFile));
  if(DebugLevel(3)) DebugMessage(L"Plugin: \""+wdx_plugin[wdx_field[n].PluginIndex].Name+L"\" calls Function: \"ContentGetValue\"");
  try       { Result=wdx_plugin[wdx_field[n].PluginIndex].F_ContentGetValue(cBuffer2,wdx_field[n].FieldIndex,Unitindex,cBuffer,BSize,0);     }
  catch(...){ if(DebugLevel(1)) DebugMessage(L"Error in Plugin: \""+wdx_plugin[wdx_field[n].PluginIndex].Name+L"\" calling \"ContentGetValue\""); return -7; }
 }else return -7;                                                             // unable to access Field
 return Result;
}


// ------------------------------------------------------------------------------------------------
// wdx class: get or create the cached string
wstring wdx_class::Get_wdxStrings(int n)
{
 if(wdx_field[n].Init) return wdx_field[n].Name;
 if(Cache.find(wsLastFile)!=Cache.end() && Cache[wsLastFile][n].Init) return Cache[wsLastFile][n].Name; // lookup in the cache
 wdx_field[n].Name=L"";
 wdx_field[n].Init=1;

 int Result=0,Unitindex=0;
 for(int i=0;i<BSize;i++) cBuffer[i]=0;

// get field for file
 Result=ContentGetValue(n,Unitindex);
 if(Result<0) return L"";                                                               // error or empty value in F_ContentGetValue

// convert data to wstring
 if(Result==ft_boolean){                                                                // boolean
  if(*((int*)&cBuffer)==0) wdx_field[n].Name=L"false";
  else                     wdx_field[n].Name=L"true";
 }else if(Result==ft_numeric_32){                                                       // int 32
  _itow(*((int*)&cBuffer),wcBuffer,10);
  wdx_field[n].Name.assign(wcBuffer);
 }else if(Result==ft_numeric_64){                                                       // int 64
  _i64tow(*((__int64*)&cBuffer),wcBuffer,10);
  wdx_field[n].Name.assign(wcBuffer);
 }else if(Result==ft_numeric_floating){                                                 // double
  wdx_field[n].Name.assign(C_to_wString(L"%.3lf",*((double*)&cBuffer)));
 }else if(Result==ft_date){                                                             // date
  Dateformat *Date=(Dateformat*)&cBuffer;
  wdx_field[n].Name.assign(C_to_wString(L"%02d"+DateSeparator+L"%02d"+DateSeparator+L"%d",Date->Day,Date->Month,Date->Year));
 }else if(Result==ft_time){                                                             // time
  Timeformat *Time=(Timeformat*)&cBuffer;
  wdx_field[n].Name.assign(C_to_wString(L"%d"+TimeSeparator+L"%02d"+TimeSeparator+L"%02d",Time->Hour,Time->Minute,Time->Second));
 }else if(Result==ft_datetime){                                                         // date and time
  FILETIME *Date1=(FILETIME*)&cBuffer,Date2;
  SYSTEMTIME Date3;
  FileTimeToLocalFileTime(Date1,&Date2);
  FileTimeToSystemTime(&Date2,&Date3);
  wdx_field[n].Name.assign(C_to_wString(L"%d"+TimeSeparator+L"%02d"+TimeSeparator+L"%02d %02d"+DateSeparator+L"%02d"+DateSeparator+L"%d",Date3.wHour,Date3.wMinute,Date3.wSecond,Date3.wDay,Date3.wMonth,Date3.wYear));
 }else if(Result==ft_multiplechoice || Result==ft_string){                              // multiple choice, string
  wdx_field[n].Name=C_sString_to_wsString(string(cBuffer));
 }else if(Result==ft_fulltext){                                                         // fulltext
  wdx_field[n].Name=C_sString_to_wsString(string(cBuffer));
  while(Result==ft_fulltext){                                                           // -> get additional text
   Unitindex+=BSize-1;
   Result=ContentGetValue(n,Unitindex);
   if(Result==ft_fulltext) wdx_field[n].Name+=C_sString_to_wsString(string(cBuffer));
  }
 }else if(Result==ft_stringw){                                                          // stringw
  wdx_field[n].Name.assign((WCHAR*)&cBuffer);
 }
 if(!LastCaseSensitive) C_wsString_to_lower(wdx_field[n].Name);
 if(DebugLevel(2)) DebugMessage(L" >\""+wdx_field[n].Fieldname+L"@"+wdx_plugin[wdx_field[n].PluginIndex].Name+L"\": \""+wdx_field[n].Name+L"\"");

// replace chars by text if neccesary
 if(replace_chars_size()){
  C_wsString_replace_chars(wdx_field[n].Name);
  //MessageBox(0,wdx_field[n].Name.c_str(),L"",0);
 }

 return wdx_field[n].Name;
}

// ------------------------------------------------------------------------------------------------
// wdx class: free everything
void wdx_class::Free()
{
 for(int i=0;i<(int)wdx_plugin.size();i++){
  if(wdx_plugin[i].Lib){
   if(wdx_plugin[i].F_ContentPluginUnloading){
    if(DebugLevel(3)) DebugMessage(L"Plugin: \""+wdx_plugin[i].Name+L"\" calls Function: \"ContentPluginUnloading\"");
    try       { wdx_plugin[i].F_ContentPluginUnloading();                                                           }
    catch(...){ if(DebugLevel(1)) DebugMessage(L"Error in Plugin: \""+wdx_plugin[i].Name+L"\" calling \"ContentPluginUnloading\""); }
   }
   FreeLibrary(wdx_plugin[i].Lib);
   wdx_plugin[i].Lib=0;
  }
 }
 if(DebugLevel(2)) DebugMessage(L"Unloaded librarys.\n");
 size=0;
}
