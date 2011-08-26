// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 CODESIGN_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// CODESIGN_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef CODESIGN_EXPORTS
#define CODESIGN_API extern "C"__declspec(dllexport)
#else
#define CODESIGN_API __declspec(dllimport)
#endif
#define ENCODING (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING)
CODESIGN_API int fnCodeSign(LPCWSTR pwszSourceFile);
CODESIGN_API LPCWSTR fnSignWhois(LPCWSTR pwszSourceFile);