// GetMonitorType.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"



void FreePhysicalMonitor(DWORD npm, LPPHYSICAL_MONITOR ppm)
{
	DestroyPhysicalMonitors(npm, ppm);
	// Free the array.
	free(ppm);
}

LPPHYSICAL_MONITOR GetPhysicalMonitor(DWORD *pnpm)
{
	HMONITOR hMon = NULL;
	hMon = MonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);
	LPPHYSICAL_MONITOR ppm = NULL;
	DWORD npm = 0;
	BOOL bRet = GetNumberOfPhysicalMonitorsFromHMONITOR(hMon, &npm);
	if (bRet) {
		ppm = (LPPHYSICAL_MONITOR)malloc(npm * sizeof(PHYSICAL_MONITOR));
		if (ppm) {
			bRet = GetPhysicalMonitorsFromHMONITOR(hMon, npm, ppm);
			if (!bRet) {
				FreePhysicalMonitor(npm, ppm);
				ppm = NULL;
				npm = 0;
			}
		}
	}
	*pnpm = npm;
	return ppm;
}

int GetMonitorType(){
	LPPHYSICAL_MONITOR ppm = 0;
	DWORD npm = 0;
//	TCHAR *str;
	int ret=-1;
	ppm = GetPhysicalMonitor(&npm);
	if (ppm) {
/*
		TCHAR *descs[] = {
			TEXT("Shadow-mask cathode ray tube (CRT)"),
			TEXT("Aperture-grill CRT"),
			TEXT("Thin-film transistor (TFT) display"),
			TEXT("Liquid crystal on silicon (LCOS) display"),
			TEXT("Plasma display"),
			TEXT("Organic light emitting diode (LED) display"),
			TEXT("Electroluminescent display"),
			TEXT("Microelectromechanical display"),
			TEXT("Field emission device (FED) display")
		};
*/
		MC_DISPLAY_TECHNOLOGY_TYPE dtt;
		GetMonitorTechnologyType(ppm->hPhysicalMonitor, &dtt);
//		CString str;
//		str.Format(TEXT("Technology type: %s"), descs[(int)dtt]);
//		str=descs[(int)dtt];
		ret=(int)dtt;
//		MessageBox(NULL,str,L"",MB_OK);
		FreePhysicalMonitor(npm, ppm);
	}
return ret;
}