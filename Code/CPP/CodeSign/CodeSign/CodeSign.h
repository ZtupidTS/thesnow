// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� CODESIGN_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// CODESIGN_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef CODESIGN_EXPORTS
#define CODESIGN_API extern "C"__declspec(dllexport)
#else
#define CODESIGN_API __declspec(dllimport)
#endif
#define ENCODING (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING)
CODESIGN_API int fnCodeSign(LPCWSTR pwszSourceFile);
CODESIGN_API LPCWSTR fnSignWhois(LPCWSTR pwszSourceFile);