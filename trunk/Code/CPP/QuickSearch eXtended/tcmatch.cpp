// tcmatch.cpp: QuickSearch eXtended for Total Commander 7.5+
// Version 2.0.1
// by Samuel Plentz

#include "stdafx.h"
#include "tcmatch.h"
#include "wdx.h" // wdx support

// ------------------------------------------------------------------------------------------------
// global variables

HMODULE hInst;
WORD PinYinTable[0x9FA5-0x4E00+1];
DWORD curFileAttributes;
int TableCreated=0;
int LevenshteinThreshold=1;
wstring wsPreviousPresetChar=L"";
wstring wsPreviousPresetString=L"";
wstring wsIni;
wdx_class *wdx_Plugins;
struct _stat oldFiletime,newFiletime;

wstring INI_simple_search_activate_char;
wstring INI_regex_search_activate_char;
wstring INI_leven_search_activate_char;
wstring INI_srch_activate_char;
wstring INI_simple_search_match_beginning_activate_char;
wstring INI_preset_activate_char;
wstring INI_and_separator_char;
wstring INI_or_separator_char;
wstring INI_wdx_separator_char;
wstring INI_negate_char;
int     INI_case_sensitive;
int     INI_allow_empty_result;
int     INI_filter_files_and_folders;
int     INI_match_beginning;
int     INI_use_pinyin;
int     INI_debug_output;
int     INI_wdx_cache;
int     INI_override_search;
int     INI_invert_result;

// ------------------------------------------------------------------------------------------------
// load the settings from the inifile
void LoadInifile(int FirstTime=0)
{
 int Value;
 if(FirstTime){
// init the settings
  INI_simple_search_activate_char                =L"";
  INI_regex_search_activate_char                 =L"?";
  INI_leven_search_activate_char                 =L"<";
  INI_srch_activate_char                         =L"*";
  INI_preset_activate_char                       =L">";
  INI_simple_search_match_beginning_activate_char=L"^";
  INI_and_separator_char                         =L" ";
  INI_or_separator_char                          =L"|";
  INI_wdx_separator_char                         =L"/";
  INI_negate_char                                =L"!";
  INI_case_sensitive                             =0;
  INI_allow_empty_result                         =1;
  INI_filter_files_and_folders                   =3;
  INI_match_beginning                            =0;
  INI_use_pinyin                                 =(PRIMARYLANGID(GetSystemDefaultLangID())==0x04)?1:0; // use PinYin if Systemlanguage is Chinese
  INI_debug_output                               =1;
  INI_wdx_cache                                  =1000;
  wdx_Plugins                                    =0;
  INI_override_search                            =1;
  INI_invert_result                              =0;
 }

// try to load the file "tcmatch.ini" / create entries that doesnt exist
 if(    GetPrivateProfileString(L"general",L"simple_search_activate_char"                ,L"ERROR"                                ,wcBuffer,BSize,wsIni.c_str())<=1) INI_simple_search_activate_char.assign(wcBuffer);
 else WritePrivateProfileString(L"general",L"simple_search_activate_char"                ,INI_simple_search_activate_char.c_str()                ,wsIni.c_str());
 if(    GetPrivateProfileString(L"general",L"regex_search_activate_char"                 ,L"ERROR"                                ,wcBuffer,BSize,wsIni.c_str())<=1) INI_regex_search_activate_char.assign(wcBuffer);
 else WritePrivateProfileString(L"general",L"regex_search_activate_char"                 ,INI_regex_search_activate_char.c_str()                 ,wsIni.c_str());
 if(    GetPrivateProfileString(L"general",L"leven_search_activate_char"                 ,L"ERROR"                                ,wcBuffer,BSize,wsIni.c_str())<=1) INI_leven_search_activate_char.assign(wcBuffer);
 else WritePrivateProfileString(L"general",L"leven_search_activate_char"                 ,INI_leven_search_activate_char.c_str()                 ,wsIni.c_str());
 if(    GetPrivateProfileString(L"general",L"srch_activate_char"                         ,L"ERROR"                                ,wcBuffer,BSize,wsIni.c_str())<=1) INI_srch_activate_char.assign(wcBuffer);
 else WritePrivateProfileString(L"general",L"srch_activate_char"                         ,INI_srch_activate_char.c_str()                         ,wsIni.c_str());
 if(    GetPrivateProfileString(L"general",L"preset_activate_char"                       ,L"ERROR"                                ,wcBuffer,BSize,wsIni.c_str())==1) INI_preset_activate_char.assign(wcBuffer);
 else WritePrivateProfileString(L"general",L"preset_activate_char"                       ,INI_preset_activate_char.c_str()                       ,wsIni.c_str());
 if(    GetPrivateProfileString(L"general",L"simple_search_match_beginning_activate_char",L"ERROR"                                ,wcBuffer,BSize,wsIni.c_str())<=1) INI_simple_search_match_beginning_activate_char.assign(wcBuffer);
 else WritePrivateProfileString(L"general",L"simple_search_match_beginning_activate_char",INI_simple_search_match_beginning_activate_char.c_str(),wsIni.c_str());
 if(    GetPrivateProfileString(L"general",L"and_separator_char"                         ,L"ERROR"                                ,wcBuffer,BSize,wsIni.c_str())==1) INI_and_separator_char.assign(wcBuffer);
 else WritePrivateProfileString(L"general",L"and_separator_char"                         ,L"\" \""                                               ,wsIni.c_str());
 if(    GetPrivateProfileString(L"general",L"or_separator_char"                          ,L"ERROR"                                ,wcBuffer,BSize,wsIni.c_str())==1) INI_or_separator_char.assign(wcBuffer);
 else WritePrivateProfileString(L"general",L"or_separator_char"                          ,INI_or_separator_char.c_str()                          ,wsIni.c_str());
 if(    GetPrivateProfileString(L"general",L"wdx_separator_char"                         ,L"ERROR"                                ,wcBuffer,BSize,wsIni.c_str())==1) INI_wdx_separator_char.assign(wcBuffer);
 else WritePrivateProfileString(L"general",L"wdx_separator_char"                         ,INI_wdx_separator_char.c_str()                         ,wsIni.c_str());
 if(    GetPrivateProfileString(L"general",L"negate_char"                                ,L"ERROR"                                ,wcBuffer,BSize,wsIni.c_str())==1) INI_negate_char.assign(wcBuffer);
 else WritePrivateProfileString(L"general",L"negate_char"                                ,INI_negate_char.c_str()                                ,wsIni.c_str());
 Value= GetPrivateProfileInt   (L"general",L"case_sensitive"                             ,-1                                                     ,wsIni.c_str()); if(Value!=-1) INI_case_sensitive=Value;
 else WritePrivateProfileString(L"general",L"case_sensitive"                             ,L"0"                                                   ,wsIni.c_str());
 if(FirstTime){
  Value= GetPrivateProfileInt   (L"general",L"allow_empty_result"                        ,-1                                                     ,wsIni.c_str()); if(Value!=-1) INI_allow_empty_result=Value;
  else WritePrivateProfileString(L"general",L"allow_empty_result"                        ,L"1"                                                   ,wsIni.c_str());
 }
 Value= GetPrivateProfileInt   (L"general",L"filter_files_and_folders"                   ,-1                                                     ,wsIni.c_str()); if(Value!=-1) INI_filter_files_and_folders=Value;
 else WritePrivateProfileString(L"general",L"filter_files_and_folders"                   ,L"3"                                                   ,wsIni.c_str());
 Value= GetPrivateProfileInt   (L"general",L"match_beginning"                            ,-1                                                     ,wsIni.c_str()); if(Value!=-1) INI_match_beginning=Value;
 else WritePrivateProfileString(L"general",L"match_beginning"                            ,L"0"                                                   ,wsIni.c_str());
 Value= GetPrivateProfileInt   (L"general",L"use_pinyin"                                 ,-1                                                     ,wsIni.c_str()); if(Value!=-1) INI_use_pinyin=Value;
 else WritePrivateProfileString(L"general",L"use_pinyin"                                 ,(INI_use_pinyin)?L"1":L"0"                             ,wsIni.c_str());
 Value= GetPrivateProfileInt   (L"wdx"    ,L"debug_output"                               ,-1                                                     ,wsIni.c_str()); if(Value!=-1) INI_debug_output=Value;
 else WritePrivateProfileString(L"wdx"    ,L"debug_output"                               ,L"1"                                                   ,wsIni.c_str());
 Value= GetPrivateProfileInt   (L"wdx"    ,L"wdx_cache"                                  ,-1                                                     ,wsIni.c_str()); if(Value!=-1) INI_wdx_cache=Value;
 else WritePrivateProfileString(L"wdx"    ,L"wdx_cache"                                  ,L"1000"                                                ,wsIni.c_str());
 Value= GetPrivateProfileInt   (L"gui"    ,L"override_search"                            ,-1                                                     ,wsIni.c_str()); if(Value!=-1) INI_override_search=Value;
 else WritePrivateProfileString(L"gui"    ,L"override_search"                            ,L"1"                                                   ,wsIni.c_str());
 Value= GetPrivateProfileInt   (L"gui"    ,L"invert_result"                              ,-1                                                     ,wsIni.c_str()); if(Value!=-1) INI_invert_result=Value;
 else WritePrivateProfileString(L"gui"    ,L"invert_result"                              ,L"0"                                                   ,wsIni.c_str());
 DebugMode(INI_debug_output,INI_wdx_cache);

// load the replace chars
 replace_chars_clear();
 for(int i=1;(1==1);i++){
  wstring KeyIndex=C_to_wString(L"chars%d",i);
  if(GetPrivateProfileString(L"replace",KeyIndex.c_str(),L"",wcBuffer,BSize,wsIni.c_str())==0) break;
  wstring temp(wcBuffer);
  int pos=temp.find(L'|');
  if(pos==-1) continue;
  for(int j=0;j<pos;j++){
   replace_chars_add(temp[j],temp.substr(pos+1));
  }
 }

// load the wdx plugins
 if(wdx_Plugins) delete wdx_Plugins;
 wdx_Plugins=new wdx_class();
 wdx_Plugins->LoadFromIni();

// open gui if it is enabled
 if(FirstTime && INI_override_search){
  wstring GuiProgram=wsIni;
  GuiProgram.erase(GuiProgram.length()-3);
  GuiProgram+=L"exe";
  ShellExecute(0,L"open",GuiProgram.c_str(),L"gui",0,SW_SHOWNORMAL);
 }

// get timestamp of last modification of the ini file
 if(_wstat(wsIni.c_str(),&oldFiletime)==-1) oldFiletime.st_mtime=0;
}

// ------------------------------------------------------------------------------------------------
// initialisation of the Process
int APIENTRY DllMain(HANDLE hModule,DWORD ul_reason_for_call,LPVOID lpReserved)
{
 switch (ul_reason_for_call)
 {
  case DLL_PROCESS_ATTACH:
   {
    hInst=(HMODULE)hModule;

// get the location of "tcmatch.ini" and load the settings
    wsIni=getIniPath(hInst);
    LoadInifile(1);
   }
   break;
  case DLL_THREAD_ATTACH:
   break;
  case DLL_THREAD_DETACH:
   break;
  case DLL_PROCESS_DETACH:
   delete wdx_Plugins;
   break;
 }
 return 1;
}

// ------------------------------------------------------------------------------------------------
// load PinYin table from file (thanks to Christian Ghisler - only load part is used, not the creation part)
void LoadPinYinTable()
{
 if(TableCreated) return;
 TableCreated=1;
 memset(&PinYinTable,0,sizeof(PinYinTable));
 char cName[MAX_PATH];
 GetModuleFileNameA(hInst,cName,sizeof(cName)-12);
 char* cPos=strrchr(cName,'\\');
 if(!cPos) return;
 cPos=strrchr(cPos,'.');
 if(cPos) cPos[0]=0;
 strncat(cName,".tbl",MAX_PATH-1);
 HANDLE File=CreateFileA(cName,GENERIC_READ,FILE_SHARE_READ | FILE_SHARE_WRITE,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
 if(File!=INVALID_HANDLE_VALUE){
  int Filesize=GetFileSize(File,0);
  DWORD bytesread;
  if(Filesize==sizeof(PinYinTable)) ReadFile(File,&PinYinTable,sizeof(PinYinTable),&bytesread,0);
  CloseHandle(File);
 }

}

// ------------------------------------------------------------------------------------------------
// compare the english PinYin "wcEnglishChar" with the chinese char "wcChineseChar" (thanks to Christian Ghisler)
int PinYinMatch(WCHAR wcEnglishChar,WCHAR wcChineseChar)
{
 if(!INI_use_pinyin) return 0;
 if(wcChineseChar==0x3007) return (wcEnglishChar=='l');
 else if(wcChineseChar>=0x4E00 && wcChineseChar<=0x9FA5){
  LoadPinYinTable();
  WORD Dbl=PinYinTable[wcChineseChar-0x4E00];
  WCHAR wcEnglishChar1=(Dbl & 0x1F)-1+'a';
  WCHAR wcEnglishChar2=((Dbl>>5)&0x1F)-1+'a';
  WCHAR wcEnglishChar3=((Dbl>>10)&0x1F)-1+'a';
  int result=(wcEnglishChar==wcEnglishChar1 || wcEnglishChar==wcEnglishChar2 || wcEnglishChar==wcEnglishChar3);
  if(!result){  // there are only 3 ideographs with more than 3 spellings:
   switch (wcChineseChar){
   case 0x7AD3: // qian1 fen1 zhi1 yi1 gong1 sheng1
    result=(wcEnglishChar=='y' || wcEnglishChar=='g' || wcEnglishChar=='s');
    break;
   case 0x7AD5:  // shi2  fen1 zhi1 yi1 gong1 sheng1
    result=(wcEnglishChar=='y' || wcEnglishChar=='g');
    break;
   case 0x7AE1:  // yi1  gong1 sheng1 bai3 bei4
    result=(wcEnglishChar=='b');
    break;
   }
  }
  return result;
 }else return false;
}

// ------------------------------------------------------------------------------------------------
// compare the english PinYin wstring "wsFilter" with the chinese wstring "wsFilename"
int PinYinStringMatch(const wstring &wsFilter,const wstring &wsFilename,int MatchBeginning)
{
 if(!INI_use_pinyin) return 0;
 if(wsFilter.length()>wsFilename.length()) return 0;
 for(int i=0;i<(int)(wsFilename.length()-wsFilter.length()+1);i++){
  if(MatchBeginning && i!=0) break;
  int PinyinOccurrence=0;
  for(int j=0;j<(int)wsFilter.length();j++){
   if(wsFilter[j]!=wsFilename[i+j]){
    if(!PinYinMatch(wsFilter[j],wsFilename[i+j])) break;    // neither normal character nor PinYin found for this position
    PinyinOccurrence=1;
   }
   if(j==wsFilter.length()-1 && PinyinOccurrence) return 1; // PinYin found
  }
 }
 return 0;
}

// ------------------------------------------------------------------------------------------------
// used to calculate the LevenshteinDistance between the "wsFilename" and the "wsFilter"
// original by Anders Sewerin Johansen (http://www.merriampark.com/ldcpp.htm)
// modified the original algorithm to make substrings and PinYin work
int LevenshteinDistance(const wstring &wsFilter,const wstring &wsFilename,int MatchBeginning)
{
 const int n=wsFilename.length();
 const int m=wsFilter.length();
 if(n==0) return m;
 if(m==0) return 0;                                                        // changed from: if(m==0) return n;

 typedef std::vector< vector<int> > Tmatrix; 
 Tmatrix matrix(n+1);
 for(int i=0;i<=n;i++) matrix[i].resize(m+1);
 for(int i=0;i<=n;i++) matrix[i][0]=0;                                     // changed from: for(int i=0;i<=n;i++) matrix[i][0]=i;
 if(MatchBeginning) for(int i=1;i<=n;i++) matrix[i][0]=i;                  // match_beginning: added
 for(int j=0;j<=m;j++) matrix[0][j]=j;

 int MinimumLastLine=MAXINT;                                               // added
 for(int j=1;j<=m;j++){
  const WCHAR t_j=wsFilter[j-1];
  for(int i=1;i<=n;i++){
   const WCHAR s_i=wsFilename[i-1];
   int cost=1;
   if(s_i==t_j || PinYinMatch(t_j,s_i)) cost=0;                            // PinYin: changed from: if(s_i==t_j) cost=0;
   int cell=min(matrix[i-1][j]+1,min(matrix[i][j-1]+1,matrix[i-1][j-1]+cost));
   if(i>2 && j>2){
    int trans=matrix[i-2][j-2]+1;
    if(wsFilename[i-2]!=t_j && !PinYinMatch(t_j,wsFilename[i-2])) trans++; // PinYin: changed from: if(wsFilename[i-2]!=t_j) trans++;
    if(s_i!=wsFilter[j-2]   && !PinYinMatch(wsFilter[j-2],s_i))   trans++; // PinYin: changed from: if(s_i!=wsFilter[j-2])   trans++;
    if(cell>trans) cell=trans;
   }
   matrix[i][j]=cell;
   if(j==m && matrix[i][j]<MinimumLastLine) MinimumLastLine=matrix[i][j];  // added
  }
 }
 return MinimumLastLine;                                                   // changed from: return matrix[n][m]
}

// ------------------------------------------------------------------------------------------------
// Filter a single Filename for the different Searchmodes
int FindInFilename(wstring wsFilter,wstring wsFilename,int SearchMode,int MatchBeginning)
{
// ------------------------------------------------------------------ Simple search
 if(SearchMode==1 || SearchMode==4){
// convert Filter to lowercase (Filename already is lowercase)
  if(!INI_case_sensitive) C_wsString_to_lower(wsFilter);

// search substring
  int Pos=int(wsFilename.find(wsFilter));
  if(MatchBeginning && Pos!=0) Pos=-1;
  if(Pos==-1 && !PinYinStringMatch(wsFilter,wsFilename,MatchBeginning)) return 0;
  else                                                                  return 1;
 }

// ------------------------------------------------------------------ RegEx search
 if(SearchMode==2){
// Workaround: dont filter anything if RegEx seems to be broken
  if(!INI_allow_empty_result){
   wstring::const_iterator it=wsFilter.begin();
   int Backslash=0,Count1=0,Count2=0,Count3=0;
   while(it!=wsFilter.end()){
    if(*it=='(' && !Backslash) Count1++;
    if(*it==')' && !Backslash) Count1--;
    if(*it=='[' && !Backslash) Count2++;
    if(*it==']' && !Backslash) Count2--;
    if(*it=='{' && !Backslash) Count3++;
    if(*it=='}' && !Backslash) Count3--;
    if(*it=='\\') Backslash=1;
    else          Backslash=0;
    it++;
   }
   if(Count1 || Count2 || Count3) return 1;
  }

// add ".*" after search text to make it possible to write a "\"
  wstring wsFilter_RegEx=wsFilter+L".*";
  if(MatchBeginning) wsFilter_RegEx=L"^"+wsFilter+L".*";

// search substring with RegEx
  CRegexpT <WCHAR> regexp(wsFilter_RegEx.c_str(),wsFilter_RegEx.length(),(INI_case_sensitive)?NO_FLAG:IGNORECASE);
  MatchResult result=regexp.Match(wsFilename.c_str());
  if(!result.IsMatched()){
   if(!INI_case_sensitive){
    wstring wsFilename_lower(wsFilename);
    C_wsString_to_lower(wsFilter); // for pinyin the Filename and the Filter has to be made lowercase
    C_wsString_to_lower(wsFilename_lower);
    if(     !PinYinStringMatch(wsFilter,wsFilename_lower,MatchBeginning)) return 0;
   }else if(!PinYinStringMatch(wsFilter,wsFilename      ,MatchBeginning)) return 0;
  }
  return 1;
 }

// ------------------------------------------------------------------ Levenshtein search
 if(SearchMode==3){
// convert Filterpart to lowercase (Filename already is lowercase)
  if(!INI_case_sensitive) C_wsString_to_lower(wsFilter);

// search substring
  int Differences=LevenshteinDistance(wsFilter,wsFilename,MatchBeginning);
  if(Differences>LevenshteinThreshold) return 0;
 }

// ------------------------------------------------------------------ Srch
 if(SearchMode==5){
// convert Filter to lowercase (Filename already is lowercase)
  if(!INI_case_sensitive) C_wsString_to_lower(wsFilter);

// search every char from the Filter in the Filename
  const int n=wsFilename.length();
  const int m=wsFilter.length();
  if(m==0) return 1;
  if(n==0) return 0;
  int j=0;
  int i=0;
  for(int j=0;j<n;j++){
   if(wsFilter[i]==wsFilename[j] || PinYinMatch(wsFilter[i],wsFilename[j])) i++;
   if(MatchBeginning && j==0 && i==0) return 0;
   if(i==m) return 1;
  }
  return 0;
 }
 return 1;
}


// ------------------------------------------------------------------------------------------------
// main compare options
int __stdcall MatchGetSetOptions(int Status)
{
 //returns sum of:
 // 1: override internal search                                          -> always do this
 // 2: no leading/trailing asterisk                                      -> always do this
 // 4: file name with path, not just name                                -> always do this
 // 8: allow empty result: longer search strings can return more results -> depends on ini-entry
 int                                                         sum =7;
 if(INI_allow_empty_result)                                  sum+=8;
 return                                                      sum;
}

// ------------------------------------------------------------------------------------------------
// main compare function
int __stdcall MatchFileW(WCHAR* wcFilter,WCHAR* wcFilename)
{
//* ----------------------------------------------------------------------------------------------- */ MeasureTime(1);

// convert WCHAR* to wString
 wstring wsFilter(wcFilter);
 if(!wsFilter.length()) return 1; // no text at all -> match all files
 wstring wsFilenameWithPath(wcFilename);
 int Pos=wsFilenameWithPath.find_last_of(L"\\/");
 wstring wsFilename=wsFilenameWithPath.substr(Pos+1,wsFilenameWithPath.length()-Pos-1);
 if(wsFilename==L"..") return 0; // entry ".." is also send for filtering
 if(DebugLevel(2)) DebugMessage(L"\nFile: \""+wsFilename+L"\"\nFilter: \""+wsFilter+L"\"");

// replace chars by text if neccesary
 if(replace_chars_size()){
  C_wsString_replace_chars(wsFilter);
  C_wsString_replace_chars(wsFilename);
 }

//* ----------------------------------------------------------------------------------------------- */ MeasureTime();

// get timestamp of last modification and check if the ini file has been manipulated
 static int Filter_with_wdx=0;
 static wstring OldFilter=L"";
 if(OldFilter!=wsFilter){
  OldFilter=wsFilter;
  if(_wstat(wsIni.c_str(),&newFiletime)==-1) newFiletime.st_mtime=0;
  if(newFiletime.st_mtime!=oldFiletime.st_mtime) LoadInifile();
  Filter_with_wdx=(wsFilter.find(INI_wdx_separator_char)!=-1)?1:0;
 }
//* ----------------------------------------------------------------------------------------------- */ MeasureTime();

// Filter only files or folders
 curFileAttributes=-1;
 if(INI_filter_files_and_folders!=3 || (wdx_Plugins->size!=0 && (Filter_with_wdx || wdx_Plugins->Groups[0].size()))){
  if(wsFilenameWithPath.find(L"\\\\")!=0){
   curFileAttributes=GetFileAttributes(wsFilenameWithPath.c_str());
   if(INI_filter_files_and_folders==1 && curFileAttributes!=-1 &&  (curFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) return 1;
   if(INI_filter_files_and_folders==2 && curFileAttributes!=-1 && !(curFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) return 1;
  }
 }
 
// load preset if first char is preset_activate_char
 if(wsFilter.substr(0,1)==INI_preset_activate_char){
  if(wsFilter.length()==1)                                                               return 1; // no preset chosen until now -> match all files
  if(wsFilter.length()>=3 && !INI_allow_empty_result && INI_filter_files_and_folders==3) return 0; // only one char after preset char allowed

// load previousPresetString from ini only if neccessary
  if(wsFilter.substr(1,1)!=wsPreviousPresetChar){
   wsPreviousPresetChar=wsFilter.substr(1,1);
   GetPrivateProfileString(L"presets",wsPreviousPresetChar.c_str(),L"",wcBuffer,BSize,wsIni.c_str());
   wsPreviousPresetString.assign(wcBuffer);
  }
  wsFilter=wsPreviousPresetString;
  if(!wsFilter.length()) return 0;
 }

// save as preset if preset_activate_char is found
 if(wsFilter.find(INI_preset_activate_char)!=-1){
  wstring wsSavePreset=wsFilter;
  wsFilter=StringSplit(wsSavePreset,INI_preset_activate_char);
  if((wsSavePreset.length()>=2 || wsSavePreset==L"=") && !INI_allow_empty_result && INI_filter_files_and_folders==3) return 0; // only one char after preset char allowed
  if(wsSavePreset.length()==1 && wsFilter.length()){
   WritePrivateProfileString(L"presets",wsSavePreset.c_str(),wsFilter.c_str(),wsIni.c_str());
   if(_wstat(wsIni.c_str(),&oldFiletime)==-1) oldFiletime.st_mtime=0; // get timestamp of last modification of the ini file
   if(wsSavePreset==wsPreviousPresetChar) wsPreviousPresetString=wsFilter;
  }
 }

//* ----------------------------------------------------------------------------------------------- */ MeasureTime();

// check if current expression should be negated
 int ReturnTrueValue=1;
 if(wsFilter.substr(0,1)==INI_negate_char){
  wsFilter.erase(0,1);
  ReturnTrueValue=0;
  if(!wsFilter.length()) return 0;
 }
 if(INI_override_search && INI_invert_result) ReturnTrueValue=1-ReturnTrueValue;

// determine what searchmode to use
 int SearchMode=0;
 if     (wsFilter.substr(0,1)==INI_simple_search_activate_char                ) SearchMode=1;
 else if(wsFilter.substr(0,1)==INI_regex_search_activate_char                 ) SearchMode=2;
 else if(wsFilter.substr(0,1)==INI_leven_search_activate_char                 ) SearchMode=3;
 else if(wsFilter.substr(0,1)==INI_srch_activate_char                         ) SearchMode=5;
 else if(wsFilter.substr(0,1)==INI_simple_search_match_beginning_activate_char) SearchMode=4;
 if(SearchMode==0){
  if(INI_override_search){
   if(INI_override_search>0 && INI_override_search<4) SearchMode=INI_override_search;
   else if(INI_override_search==4)                    SearchMode=5;
   else                                               SearchMode=1;
  }else{
   if     (INI_simple_search_activate_char.length()                ==0) SearchMode=1;
   else if(INI_regex_search_activate_char.length()                 ==0) SearchMode=2;
   else if(INI_leven_search_activate_char.length()                 ==0) SearchMode=3;
   else if(INI_srch_activate_char.length()                         ==0) SearchMode=5;
   else if(INI_simple_search_match_beginning_activate_char.length()==0) SearchMode=4;
  }
 }else wsFilter.erase(0,1);
 if(SearchMode==0) return 0;

// announce new file for matching
 if(wdx_Plugins->size!=0 && curFileAttributes!=-1) wdx_Plugins->Create_wdxStrings(wsFilenameWithPath,INI_case_sensitive);

// convert Filename to lowercase
 if((SearchMode!=2) && !INI_case_sensitive) C_wsString_to_lower(wsFilename);

// get count of character changes in the levenshtein search
 if(SearchMode==3 && wsFilter.length() && wsFilter.at(0)>='1' && wsFilter.at(0)<='9'){
  LevenshteinThreshold=wsFilter.at(0)-'0';
  wsFilter.erase(0,1);
 }

// ------------------------------------------------------------------ Main search routine in Filename
//* ----------------------------------------------------------------------------------------------- */ MeasureTime();

 if(!wsFilter.length()) return 1; // Structure: "w1 w2 w3|w4 w5/w6 w7|w8//w9" -> ((w1 & w2 & w3)|(w4 & w5)) & ((w6 & w7)|(w8)) & 1 & ((w9))
 int wdxNumber=0;

// split at each wdx-separator in filter
 while(wsFilter.length()){
  wstring wsFilterWdxPart=StringSplit(wsFilter,INI_wdx_separator_char);
  int Result_OrPart=0;
  wdxNumber++;
  if(!wsFilterWdxPart.length()) Result_OrPart=1; // empty Filter for a wdx part

// split at each or-separator in filter
  while(wsFilterWdxPart.length()){
   wstring wsFilterOrPart=StringSplit(wsFilterWdxPart,INI_or_separator_char);
   int Result_AndPart=1;
   int MatchBeginningOnlyFirst=INI_match_beginning;
   if(SearchMode==4) MatchBeginningOnlyFirst=1;

// split at each and-separator in filter
   while(wsFilterOrPart.length()){
    wstring wsFilterAndPart=StringSplit(wsFilterOrPart,INI_and_separator_char);
    if(!wsFilterAndPart.length()){ MatchBeginningOnlyFirst=0; continue; }

// Filter the current Filename or the wdx strings
    if(wdxNumber!=1 || !FindInFilename(wsFilterAndPart,wsFilename,SearchMode,MatchBeginningOnlyFirst)){
     if(wdx_Plugins->size!=0 && curFileAttributes!=-1 && (int)wdx_Plugins->Groups.size()>=wdxNumber){
      int Result_wdx=0;
      for(int i=0;i<(int)wdx_Plugins->Groups[wdxNumber-1].size();i++){
       if(FindInFilename(wsFilterAndPart,wdx_Plugins->Get_wdxStrings(wdx_Plugins->Groups[wdxNumber-1][i]),SearchMode,0)){
        Result_wdx=1;
        break;
       }
      }
      if(Result_wdx){ MatchBeginningOnlyFirst=0; continue; }
     }
     Result_AndPart=0; // neither found it in filename nor in wdx strings
     break;
    }

    MatchBeginningOnlyFirst=0;
   } // end of split and
   if(Result_AndPart){ Result_OrPart=1; break; }
   else                continue;
  } // end of split or
  if(Result_OrPart) continue;
  else{
//* ----------------------------------------------------------------------------------------------- */ MeasureTime(3);
   return 1-ReturnTrueValue;
  }
 } // end of split wdx
//* ----------------------------------------------------------------------------------------------- */ MeasureTime(3);
 return ReturnTrueValue;
}
