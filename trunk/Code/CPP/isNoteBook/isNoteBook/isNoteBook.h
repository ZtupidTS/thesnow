// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� ISNOTEBOOK_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// ISNOTEBOOK_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef ISNOTEBOOK_EXPORTS
#define ISNOTEBOOK_API extern "C" __declspec(dllexport)
#else
#define ISNOTEBOOK_API __declspec(dllimport)
#endif

// ISNOTEBOOK_API 
int isNoteBook(int argc, char *argv[], char *envp[]);
int HaveBattery();
int BatteryLifeTime();
int BatteryLifeTimePercent();
int ACLineStatus();
int CPUNum();