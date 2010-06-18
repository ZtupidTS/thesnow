/*
 *  VirtuaWin - Virtual Desktop Manager (virtuawin.sourceforge.net)
 *  VWDesktopIcons.c - Module to control which icons are displayed on each desktop.
 * 
 *  Copyright (c) 2006-2010 VirtuaWin (VirtuaWin@home.se)
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, 
 *  USA.
 * 
 *************************************************************************
 * 
 * Simple module skeleton for VirtuaWin. These are the minimal requirements.
 * It is a simple application with a hidden window that receives messages from virtuawin
 * Look in Messages.h to see what can be sent to and from VirtuaWin
 * 
 * Note that the classname must be the same as the filename including the '.exe'
 */
#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <commctrl.h>
#include "../../Defines.h"
#include "../../Messages.h"
#include "Resource.h"

#define vwICON_HIDDEN 0x01

typedef struct vwIcon {
    struct vwIcon *next ;
    unsigned char  show[vwDESKTOP_SIZE] ;
    POINT          point[vwDESKTOP_SIZE] ;
    TCHAR          name[1] ;
} vwIcon ;

#if defined(_WIN64)

typedef LVITEM vwLVITEM64 ;

#else

typedef struct { 
    UINT mask; 
    int  iItem; 
    int  iSubItem; 
    UINT state; 
    UINT stateMask; 
    int  pad1 ; 
    LPTSTR pszText; 
    int  pszTextUpper ; 
    int  cchTextMax; 
    int  iImage; 
    LPARAM lParam;
    int  pad3 ; 
#if (_WIN32_IE >= 0x0300)
    int iIndent;
#endif
    int iGroupId;
} vwLVITEM64 ;

#endif

#define diItemSize (sizeof(vwLVITEM64) + sizeof(POINT) + (sizeof(TCHAR) * MAX_PATH))
#define vwGRID_OFFSET_Y  2
#define gridXPos(x) (((((x) + (gridSize >> 1) - gridXOffset) / gridSize) * gridSize) + gridXOffset)
#define gridYPos(y) (((((y) + (gridSize >> 1) - vwGRID_OFFSET_Y) / gridSize) * gridSize) + vwGRID_OFFSET_Y)

HINSTANCE hInst;                 /* Instance handle */
HWND    wHnd;                    /* DesktopIcons Handle */
HWND    setupWHnd;               /* DesktopIcons Setup Handle */
HWND    vwHandle;                /* Handle to VirtuaWin */
TCHAR   userAppPath[MAX_PATH] ;  /* User's config path */
HWND    diHandle;                /* Handle to SystemList containing icons */
HANDLE  diProcess;
LVITEM *diLocalItem ;
char   *diShareItem ;
TCHAR  *diItemName ;
POINT  *diItemPos ;

int     deskCrrnt ;
int     deskCopy ;
vwIcon *iconHead ;
int     gridSize = 75 ;
int     gridXOffset = 21 ;
int     setupChanged ;
unsigned char initialised ;
unsigned char shuttingDown ;
unsigned char autoGrid[vwDESKTOP_SIZE] ;
unsigned char autoUpdate[vwDESKTOP_SIZE] ;
unsigned char saveDesktop[vwDESKTOP_SIZE] ;
unsigned char showNewIcons[vwDESKTOP_SIZE] ;

#define vwDI_DEBUG 0
#if vwDI_DEBUG
FILE *logfp ;
#endif

static void
CheckDesktopSettings(void)
{
    HKEY hkey=NULL ;
    if((RegOpenKeyEx(HKEY_CURRENT_USER,_T("Software\\Microsoft\\Windows\\Shell\\Bags\\1\\Desktop"), 0,KEY_READ, &hkey) == ERROR_SUCCESS) && (hkey != NULL))
    {
        DWORD fflags, fflagsType, fflagsLen ;
        fflagsLen = 4 ;
        if((RegQueryValueEx(hkey,_T("FFlags"),NULL,&fflagsType,(LPBYTE) &fflags,&fflagsLen) == ERROR_SUCCESS) && 
           (fflagsType == REG_DWORD) && (fflagsLen == 4) && (fflags & 0x05))
        {
            MessageBox(wHnd,_T("Your current Windows desktop settings appear to be incompatible\n")
                       _T("with VWDesktopIcons, you need to disable 'Auto Arrange' and\n")
                       _T("'Align to Grid' settings in the 'Arrange Icons By' desktop context\n")
                       _T("menu to use this module.\n\nPlease ignore this warning if you have already made this change."),_T("VWDesktopIcons Warning"), MB_ICONWARNING) ;
        }            
    }
}

static void
LoadConfigFile(void)
{
    char buff[1024], *ss ;
#ifdef _UNICODE
    TCHAR buffW[1024], *ssT ;
#else
    char *ssT ;
#endif
    vwIcon *ci=NULL ;
    int len, rr, ia[4] ;
    FILE *fp ;
    memset(showNewIcons,1,vwDESKTOP_SIZE) ;
    memset(autoUpdate,1,vwDESKTOP_SIZE) ;
    memset(autoGrid,0,vwDESKTOP_SIZE) ;
    
    ssT = userAppPath + _tcslen(userAppPath) ;
    _tcscpy(ssT,_T("desktopicons.cfg")) ;
    fp = _tfopen(userAppPath,_T("r")) ;
    *ssT = '\0' ;
    if(fp != NULL)
    {
        while(fgets(buff,1024,fp) != NULL)
        {
            if(buff[0] == 'A')
            {
                if((sscanf(buff+1,"%d %d",ia,ia+1) == 2) && (ia[0] > 0) && (ia[0] < vwDESKTOP_SIZE))
                    showNewIcons[ia[0]] = ia[1] ;
            }
            else if(buff[0] == 'B')
            {
                if((sscanf(buff+1,"%d %d",ia,ia+1) == 2) && (ia[0] > 0) && (ia[0] < vwDESKTOP_SIZE))
                    autoUpdate[ia[0]] = ia[1] ;
            }
            else if(buff[0] == 'C')
            {
                if((sscanf(buff+1,"%d %d",ia,ia+1) == 2) && (ia[0] > 0) && (ia[0] < vwDESKTOP_SIZE))
                    autoGrid[ia[0]] = ia[1] ;
            }
            else if(buff[0] == 'D')
            {
                if((ci != NULL) && (sscanf(buff+1,"%d %d %d %d",ia,ia+1,ia+2,ia+3) == 4) &&
                   (ia[0] > 0) && (ia[0] < vwDESKTOP_SIZE))
                {
                    saveDesktop[ia[0]] = 1 ;
                    ci->show[ia[0]] = ia[1] ;
                    ci->point[ia[0]].x = ia[2] ;
                    ci->point[ia[0]].y = ia[3] ;
                }
            }
            else if(buff[0] == 'G')
            {
                if((sscanf(buff+1,"%d",ia) == 1) && (ia[0] > 0))
                {
                    gridSize = ia[0] ;
                    gridXOffset = (gridSize * 7) / 24 ;
                }
            }
            else if(buff[0] == 'I')
            {
                ss = buff+1 ;
                if(*ss == ' ')
                    ss++ ;
                if(((len = ((int) strlen(ss)) - 1) >= 0) && (ss[len] == '\n'))
                    ss[len] = '\0' ;
                if(*ss == '\0')
                    ci = NULL ;
                else
                {
#ifdef _UNICODE
                    MultiByteToWideChar(CP_UTF8,0,ss,-1,buffW,1024) ;
                    ssT = buffW ;
#else
                    ssT = ss ;
#endif
                    rr = 1 ;
                    ci = iconHead ;
                    while((ci != NULL) && ((rr=_tcscmp(ci->name,ssT)) < 0))
                        ci = ci->next ;
                    if(rr != 0)
                        ci = NULL ;
                }
            }
        }
        fclose(fp) ;
    }
}

static void
SaveConfigFile(void)
{
    TCHAR *ss ;
    int ii ;
    vwIcon *ci ;
#ifdef _UNICODE
    char name[1024] ;
#else
    char *name ;
#endif
    FILE *fp ;
    
    ss = userAppPath + _tcslen(userAppPath) ;
    _tcscpy(ss,_T("desktopicons.cfg")) ;
    fp = _tfopen(userAppPath,_T("w")) ;
    *ss = '\0' ;
    if(fp != NULL)
    {
        fprintf(fp,"G %d\n",gridSize) ;
        for(ii=1 ; ii< vwDESKTOP_SIZE ; ii++)
        {
            if(showNewIcons[ii] != 1)
                fprintf(fp,"A %d %d\n",ii,(int) showNewIcons[ii]) ;
            if(autoUpdate[ii] != 1)
                fprintf(fp,"B %d %d\n",ii,(int) autoUpdate[ii]) ;
            if(autoGrid[ii] != 0)
                fprintf(fp,"C %d %d\n",ii,(int) autoGrid[ii]) ;
        }
        ci = iconHead ;
        while(ci != NULL)
        {
#ifdef _UNICODE
            WideCharToMultiByte(CP_UTF8,0,ci->name,-1,name,1024, 0, 0) ;
#else
            name = ci->name ;
#endif
            fprintf(fp,"I %s\n",name) ;
            for(ii=1 ; ii< vwDESKTOP_SIZE ; ii++)
                if(saveDesktop[ii])
                    fprintf(fp,"D %d %d %d %d\n",ii,(int) ci->show[ii],(int) ci->point[ii].x,(int) ci->point[ii].y) ;
            ci = ci->next ;
        }
        fclose(fp) ;
    }
}


static int
UpdateDesktopIcons(int fdesk, int tdesk, int flags)
{
    SIZE_T dwNumberOfBytes;
    int ii, rr, xx, yy, itemCount ;
    vwIcon *ci, *pi, *ti ;
    
    if(tdesk == fdesk)
        tdesk = -1 ;
    else
        saveDesktop[tdesk] = 1 ;
    
    if((fdesk <= 0) || ((autoUpdate[fdesk] == 0) && ((flags & 0x01) == 0)))
        fdesk = -1 ;
    
    itemCount = ListView_GetItemCount(diHandle) ;

#if vwDI_DEBUG
    fprintf(logfp,"UpdateDesktopIcons %d: %d %d %x\n",itemCount,fdesk,tdesk,flags) ;
#endif
    
    for(ii=0 ; ii<itemCount ; ii++)
    {
        diLocalItem->iItem = ii;
#if vwDI_DEBUG
        fprintf(logfp,"Item %d:",ii) ;
        for(rr=0 ; rr < 56 ; rr++)
            fprintf(logfp," %02x",(unsigned int) ((unsigned char *) diLocalItem)[rr]) ;
        fprintf(logfp,"\n") ;
#endif
        WriteProcessMemory(diProcess, diShareItem, diLocalItem, diItemSize, &dwNumberOfBytes);
        SendMessage(diHandle, LVM_GETITEM, 0, (LPARAM)diShareItem);
        ListView_GetItemPosition(diHandle,ii,(POINT *)(diShareItem+sizeof(vwLVITEM64)));
        ReadProcessMemory(diProcess, diShareItem, diLocalItem, diItemSize, &dwNumberOfBytes);
        rr = 1 ;
        pi = NULL ;
        ci = iconHead ;
        while((ci != NULL) && ((rr=_tcscmp(ci->name,diItemName)) < 0))
        {
            pi = ci ;
            ci = ci->next ;
        }
#if vwDI_DEBUG
        fprintf(logfp,"%d: %d %d %d [%s]\n",ii,rr,diItemPos->x,diItemPos->y,diItemName) ;
#endif
        if(rr != 0)
        {
            if((ti = malloc(sizeof(vwIcon)+(_tcslen(diItemName)*sizeof(TCHAR)))) == NULL)
                return 1 ;
            _tcscpy(ti->name,diItemName) ;
            for(rr=0 ; rr<vwDESKTOP_SIZE ; rr++)
            {
                ti->show[rr] = showNewIcons[rr] ;
                ti->point[rr].x = diItemPos->x ;
                ti->point[rr].y = diItemPos->y ;
            }
            ti->show[deskCrrnt] = 1 ;
            if(pi == NULL)
                iconHead = ti ;
            else
                pi->next = ti ;
            ti->next = ci ;
            ci = ti ;
        }
        else if(fdesk > 0)
        {
            if((ci->show[fdesk] != 0) ||
               ((flags & 0x01) && (diItemPos->y > -5000)))
            {
                ci->show[fdesk] = 1 ;
                if(autoGrid[fdesk])
                {
                    ci->point[fdesk].x = gridXPos(diItemPos->x) ;
                    ci->point[fdesk].y = gridYPos(diItemPos->y) ;
                }
                else
                {
                    ci->point[fdesk].x = diItemPos->x ;
                    ci->point[fdesk].y = diItemPos->y ;
                }
            }
        }
        if(tdesk >= 0)
        {
            if(ci->show[tdesk] != 0)
            {
                xx = ci->point[tdesk].x ;
                yy = ci->point[tdesk].y ;
            }
            else
            {
                xx = ci->point[0].x ;
                yy = ci->point[0].y - 20000 ;
            }
            if((xx != diItemPos->x) || (yy != diItemPos->y))
               ListView_SetItemPosition(diHandle,ii,xx,yy) ;
        }
    }
    return 0 ;
}

static void
SetDesktopIcons(int tdesk)
{
    SIZE_T dwNumberOfBytes;
    int ii, itemCount ;
    vwIcon *ci ;
    
    itemCount = ListView_GetItemCount(diHandle) ;
    for(ii=0 ; ii<itemCount ; ii++)
    {
        diLocalItem->iItem = ii;
        WriteProcessMemory(diProcess, diShareItem, diLocalItem, diItemSize, &dwNumberOfBytes);
        SendMessage(diHandle, LVM_GETITEM, 0, (LPARAM)diShareItem);
        ListView_GetItemPosition(diHandle,ii,(POINT *)(diShareItem+sizeof(vwLVITEM64)));
        ReadProcessMemory(diProcess, diShareItem, diLocalItem, diItemSize, &dwNumberOfBytes);
        ci = iconHead ;
        while(ci != NULL)
        {
            if(!_tcscmp(ci->name,diItemName))
            {
                if(ci->show[tdesk] == 0)
                    ListView_SetItemPosition(diHandle,ii,ci->point[0].x,ci->point[0].y-20000) ;
                else
                    ListView_SetItemPosition(diHandle,ii,ci->point[tdesk].x,ci->point[tdesk].y) ;
                break ;
            }
            ci = ci->next ;
        }
    }
}

static void
AlignDesktopIconsToGrid(int tdesk)
{
    vwIcon *ci ;
    
    UpdateDesktopIcons(tdesk,tdesk,1) ;
    ci = iconHead ;
    while(ci != NULL)
    {
        if(ci->show[tdesk] != 0)
        {
            ci->point[tdesk].x = gridXPos(ci->point[tdesk].x) ;
            ci->point[tdesk].y = gridYPos(ci->point[tdesk].y) ;
        }
        ci = ci->next ;
    }
}


static void
GenerateDesktopIconList(HWND hDlg)
{
    vwIcon *ci ;
    
    SendDlgItemMessage(hDlg,ID_HIDE_LIST,LB_RESETCONTENT,0,0);
    SendDlgItemMessage(hDlg,ID_SHOW_LIST,LB_RESETCONTENT,0,0);
    
    ci = iconHead ;
    while(ci != NULL)
    {
        if(ci->show[deskCrrnt] == 0)
        {
            /* icon is flagged as hidden */
            SendDlgItemMessage(hDlg,ID_HIDE_LIST,LB_ADDSTRING,0,(LPARAM) ci->name);
        }
        else
        {
            SendDlgItemMessage(hDlg,ID_SHOW_LIST,LB_ADDSTRING,0,(LPARAM) ci->name);
        }
        ci = ci->next ;
    }
}

static void
initDialog(HWND hwndDlg, BOOL enableApply)
{
    HWND hBtn ;
    TCHAR buff[64] ;
    
    _stprintf(buff,_T("Configure icons for desktop %d:\n"),deskCrrnt) ;
    SetDlgItemText(hwndDlg, ID_LABEL,buff) ;
    GenerateDesktopIconList(hwndDlg);
    SendDlgItemMessage(hwndDlg, ID_SHOW_NEW_ICN, BM_SETCHECK, showNewIcons[deskCrrnt], 0);
    SendDlgItemMessage(hwndDlg, ID_AUTO_UPDATE, BM_SETCHECK, autoUpdate[deskCrrnt], 0);
    SendDlgItemMessage(hwndDlg, ID_AUTO_GRID, BM_SETCHECK, autoGrid[deskCrrnt], 0);
    _stprintf(buff,_T("%d"),gridSize) ;
    SetDlgItemText(hwndDlg, ID_GRID_SIZE,buff) ;
    if((hBtn=GetDlgItem(hwndDlg,ID_AUTO_GRID)) != NULL)
        EnableWindow(hBtn,autoUpdate[deskCrrnt]) ;
    if((hBtn=GetDlgItem(hwndDlg,ID_APPLY)) != NULL)
        EnableWindow(hBtn,enableApply) ;
    if((hBtn=GetDlgItem(hwndDlg,ID_SHOW_ICON)) != NULL)
        EnableWindow(hBtn,FALSE) ;
    if((hBtn=GetDlgItem(hwndDlg,ID_HIDE_ICON)) != NULL)
        EnableWindow(hBtn,FALSE) ;
    if((hBtn=GetDlgItem(hwndDlg,ID_PASTE)) != NULL)
        EnableWindow(hBtn,(deskCopy && (deskCopy != deskCrrnt)) ? TRUE:FALSE) ;
    if(!enableApply)
        setupChanged = 0 ;
    else if(!setupChanged)
        setupChanged = -1 ;
}

static void
SetupShowCurrentIcon(HWND hDlg)
{
    vwIcon *ci ;
    TCHAR lbl_buff[1024] ;
    int i, count, *id_buff ;
    
    if(((count = (int) SendDlgItemMessage(hDlg, ID_HIDE_LIST, LB_GETSELCOUNT, 0, 0)) != LB_ERR) && (count > 0) &&
       ((id_buff = GlobalAlloc(GPTR, sizeof(int) * count)) != NULL))
    {
        SendDlgItemMessage(hDlg, ID_HIDE_LIST, LB_GETSELITEMS, (WPARAM)count, (LPARAM)id_buff);
        for(i = count - 1; i >= 0; i--)
        {
            if (SendDlgItemMessage(hDlg,ID_HIDE_LIST,LB_GETTEXT, (WPARAM)id_buff[i],(LPARAM) lbl_buff) > 0)
            {
                ci = iconHead;
                while (ci != NULL)
                {
                    if (!_tcscmp(ci->name, lbl_buff))
                    {
                        ci->show[deskCrrnt] = 1;
                        ci->point[deskCrrnt] = ci->point[0] ; 
                        break;
                    }
                    ci = ci->next;
                }				
            }
            
        }
        GlobalFree(id_buff) ;
    }
}

static void
SetupHideCurrentIcon(HWND hDlg)
{
    vwIcon *ci ;
    TCHAR lbl_buff[1024] ;
    int i, count, *id_buff ;
    
    if(((count = (int) SendDlgItemMessage(hDlg, ID_SHOW_LIST, LB_GETSELCOUNT, 0, 0)) != LB_ERR) && (count > 0) &&
       ((id_buff = GlobalAlloc(GPTR, sizeof(int) * count)) != NULL))
    {
        SendDlgItemMessage(hDlg, ID_SHOW_LIST, LB_GETSELITEMS, (WPARAM)count, (LPARAM)id_buff);
        for(i = count - 1; i >= 0; i--)
        {
            if (SendDlgItemMessage(hDlg,ID_SHOW_LIST,LB_GETTEXT, (WPARAM)id_buff[i],(LPARAM) lbl_buff) > 0)
            {
                ci = iconHead;
                while (ci != NULL)
                {
                    if (!_tcscmp(ci->name, lbl_buff))
                    {
                        ci->show[deskCrrnt] = 0;
                        break;
                    }
                    ci = ci->next;
                }				
            }
            
        }
        GlobalFree(id_buff) ;
    }
}

static void
StoreDesktopConfig(HWND hDlg)
{
    if(hDlg != NULL)
    {
        TCHAR buff[16] ;
        showNewIcons[deskCrrnt] = (SendDlgItemMessage(hDlg, ID_SHOW_NEW_ICN, BM_GETCHECK, 0, 0) == BST_CHECKED) ;
        autoUpdate[deskCrrnt] = (SendDlgItemMessage(hDlg, ID_AUTO_UPDATE, BM_GETCHECK, 0, 0) == BST_CHECKED) ;
        autoGrid[deskCrrnt] = (SendDlgItemMessage(hDlg, ID_AUTO_GRID, BM_GETCHECK, 0, 0) == BST_CHECKED) ;
        GetDlgItemText(hDlg,ID_GRID_SIZE,buff,16) ;
        gridSize = _ttoi(buff) ;
        if(gridSize <= 0)
            gridSize = 75 ;
        gridXOffset = (gridSize * 7) / 24 ;
    }
    SetDesktopIcons(deskCrrnt) ;
    SaveConfigFile() ;
}

/*
   This is the main function for the dialog. 
 */
static BOOL CALLBACK
DialogFunc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HWND hBtn ;
    switch(msg)
    {
    case WM_INITDIALOG:
        setupWHnd = hwndDlg ;
        setupChanged = 0 ;
        /* Force VW not to manage the setup window */
        SendMessage(vwHandle,VW_WINMANAGE,(WPARAM) hwndDlg,0) ;
        initDialog(hwndDlg,FALSE);
        return TRUE;
        
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            if(setupChanged)
                UpdateDesktopIcons(deskCrrnt,deskCrrnt,1) ;
            EndDialog(hwndDlg,0);
            setupWHnd = NULL ;
            return 1;
        
        case ID_OK:
        case ID_APPLY:
            if(setupChanged)
                StoreDesktopConfig(hwndDlg) ;
            if(LOWORD(wParam) == ID_OK)
            {
                EndDialog(hwndDlg,0) ;
                setupWHnd = NULL ;
            }
            else
                initDialog(hwndDlg,FALSE) ;
            return 1;
            
        case ID_GRID:
            AlignDesktopIconsToGrid(deskCrrnt) ;
            if(!setupChanged)
            {
                if((hBtn=GetDlgItem(hwndDlg,ID_APPLY)) != NULL)
                    EnableWindow(hBtn,TRUE) ;
                setupChanged = -1 ;
            }
            return 1;
        
        case ID_STORE:
            UpdateDesktopIcons(deskCrrnt,deskCrrnt,1) ;
            initDialog(hwndDlg,FALSE) ;
            return 1;
        
        case ID_RESET:
            if(deskCrrnt)
            {
                vwIcon *ci = iconHead ;
                while(ci != NULL)
                {
                    if(ci->point[0].y < -5000)
                        ci->point[0].y += -20000 ;
                    ci->show[deskCrrnt] = 1 ;
                    ci->point[deskCrrnt] = ci->point[0] ;
                    ci = ci->next ;
                }
                initDialog(hwndDlg,TRUE);
            }
            return 1;
        
        case ID_AUTO_UPDATE:
            {
                BOOL enable = (SendDlgItemMessage(hwndDlg,ID_AUTO_UPDATE,BM_GETCHECK,0,0) == BST_CHECKED) ;
                if((hBtn=GetDlgItem(hwndDlg,ID_AUTO_GRID)) != NULL)
                    EnableWindow(hBtn,enable) ;
            }
            /* no break */
        case ID_SHOW_NEW_ICN:
        case ID_AUTO_GRID:
        case ID_GRID_SIZE:
            if(!setupChanged)
            {
                if((hBtn=GetDlgItem(hwndDlg,ID_APPLY)) != NULL)
                    EnableWindow(hBtn,TRUE) ;
                setupChanged = -1 ;
            }
            return 1;
            
        case ID_COPY:
            deskCopy = deskCrrnt ;
            if((hBtn=GetDlgItem(hwndDlg,ID_PASTE)) != NULL)
                EnableWindow(hBtn,FALSE) ;
            return 1;
            
        case ID_PASTE:
            if(deskCopy && deskCrrnt)
            {
                vwIcon *ci = iconHead ;
                while(ci != NULL)
                {
                    ci->show[deskCrrnt] = ci->show[deskCopy] ;
                    ci->point[deskCrrnt] = ci->point[deskCopy] ;
                    ci = ci->next ;
                }
                initDialog(hwndDlg,TRUE);
                if((hBtn=GetDlgItem(hwndDlg,ID_PASTE)) != NULL)
                    EnableWindow(hBtn,FALSE) ;
            }
            return 1;
        
        case ID_SHOW_ICON:
            SetupShowCurrentIcon(hwndDlg) ;
            initDialog(hwndDlg,TRUE);
            return 1;
        
        case ID_HIDE_ICON:
            SetupHideCurrentIcon(hwndDlg) ;
            initDialog(hwndDlg,TRUE);
            return 1;
        
        case ID_SHOW_LIST:
            if((HIWORD(wParam) == LBN_SELCHANGE) &&
               ((hBtn=GetDlgItem(hwndDlg,ID_HIDE_ICON)) != NULL))
                EnableWindow(hBtn,TRUE) ;
            break ;
            
        case ID_HIDE_LIST:
            if((HIWORD(wParam) == LBN_SELCHANGE) &&
               ((hBtn=GetDlgItem(hwndDlg,ID_SHOW_ICON)) != NULL))
                EnableWindow(hBtn,TRUE) ;
            break ;
        }
        break;
        
    case WM_CLOSE:
        EndDialog(hwndDlg,0);
        setupWHnd = NULL ;
        return TRUE;
	
    }
    return FALSE;
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int newDesk ;
    
    switch (msg)
    {
    case MOD_INIT: 
        /* This must be taken care of in order to get the handle to VirtuaWin. */
        /* The handle to VirtuaWin comes in the wParam */
        vwHandle = (HWND) wParam;
        if((deskCrrnt = (int) SendMessage(vwHandle,VW_CURDESK,0,0)) >= vwDESKTOP_SIZE)
            deskCrrnt = 0 ;
        else if(deskCrrnt > 0)
            saveDesktop[deskCrrnt] = 1 ;
        // If not yet initialized get the user path and initialize.
        if(!initialised)
        {
            if(!SendMessage(vwHandle, VW_USERAPPPATH, (WPARAM) hwnd, 0) || !initialised)
            {
                MessageBox(hwnd,_T("VirtuaWin failed to send the UserApp path."),_T("VWDesktopIcons Error"), MB_ICONWARNING);
                exit(1) ;
            }
            SetDesktopIcons(deskCrrnt) ;
            CheckDesktopSettings() ;
        }
        break;
    
    case WM_COPYDATA:
        if(!initialised)
        {
            COPYDATASTRUCT *cds;         
            cds = (COPYDATASTRUCT *) lParam ;         
            if(cds->dwData == (0-VW_USERAPPPATH))
            {
                if((cds->cbData < 2) || (cds->lpData == NULL))
                    return FALSE ;
                initialised = 1 ;
#ifdef _UNICODE
                MultiByteToWideChar(CP_ACP,0,(char *) cds->lpData,-1,userAppPath,MAX_PATH) ;
#else
                strcpy(userAppPath,(char *) cds->lpData) ;
#endif
                LoadConfigFile() ;
            }
        }
        return TRUE ;
        
    case MOD_CHANGEDESK:
        if((newDesk = (int) lParam) <= 0)
            return TRUE ;
        if((setupWHnd != NULL) && setupChanged)
        {
            if(setupChanged > 0)
            {
                setupChanged = newDesk ;
                return TRUE ;
            }
            setupChanged = newDesk ;
            if(MessageBox(setupWHnd,_T("Apply changes made to desktop?"),_T("VWDesktopIcons"),
                          MB_ICONQUESTION | MB_YESNO) == IDYES)
                StoreDesktopConfig(setupWHnd) ;
            else
                UpdateDesktopIcons(deskCrrnt,deskCrrnt,1) ;
            newDesk = setupChanged ;
            setupChanged = 0 ;
        }
        if(newDesk >= vwDESKTOP_SIZE)
            newDesk = 0 ;
        UpdateDesktopIcons(deskCrrnt,newDesk,0) ;
        deskCrrnt = newDesk ;
        if(setupWHnd != NULL)
            initDialog(setupWHnd,FALSE);
        return TRUE ;
    case MOD_SETUP:
        if(wParam != 0)
            hwnd = (HWND) wParam ;
        else
            hwnd = (HWND) wHnd ;
        if(deskCrrnt == 0)
        {
            MessageBox(hwnd,_T("Cannot configure icons as current desktop is out of support range."),_T("VWDesktopIcons Error"), MB_ICONWARNING);
        }
        else
        {
            SetForegroundWindow(hwnd) ;
            DialogBox(hInst, MAKEINTRESOURCE(IDD_MAINDIALOG), hwnd, (DLGPROC) DialogFunc);
        }
        break;
    
    case MOD_QUIT:
        /* This must be handled, otherwise VirtuaWin can't shut down the module */
    case WM_ENDSESSION:
    case WM_DESTROY:
        if(shuttingDown == 0)
        {
            /* Don't store the last positions as Explorer often muddles them during shutdown */
            shuttingDown = 1 ;
            SetDesktopIcons(0) ;
            SaveConfigFile() ;
        }
        PostQuitMessage(0);
        break;
    
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    
    return 0;
}

#define WIN64_KEY "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"
int
vwWindowsIs64Bit(void)
{
    TCHAR buff[128] ;
    DWORD size, type;
    HKEY base_key;
    int ret=0 ;
    
    if(RegOpenKey(HKEY_LOCAL_MACHINE,_T(WIN64_KEY),&base_key) == ERROR_SUCCESS)
    {
        size = 128 ;
        if(RegQueryValueEx(base_key,_T("Identifier"),(LPDWORD) 0,&type,(LPBYTE)&buff,&size) == ERROR_SUCCESS)
        {
            if(!_tcsncmp(buff,_T("AMD64"),5) || !_tcsncmp(buff,_T("EM64T"),5) || !_tcsncmp(buff,_T("Intel64"),7))
                ret = 1 ;
        }
        RegCloseKey(base_key);
    }
    return ret ;
}

/*
 * Main startup function
 */
int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
    DWORD dwPID;
    WNDCLASS wc;
    MSG msg;
    
#if vwDI_DEBUG
    logfp = fopen("c:\\VWDesktopIcons.log","w+") ;
#endif
    hInst = hInstance;
    memset(&wc, 0, sizeof(WNDCLASS));
    wc.style = 0;
    wc.lpfnWndProc = (WNDPROC)MainWndProc;
    wc.hInstance = hInstance ;
    /* IMPORTANT! The classname must be the same as the filename since VirtuaWin uses 
       this for locating the window */
    wc.lpszClassName = _T("VWDesktopIcons.exe");
  
    if(!RegisterClass(&wc))
        return 1 ;
  
    /* In this example, the window is never shown */
    if((wHnd=CreateWindow(_T("VWDesktopIcons.exe"), 
                          _T("VWDesktopIcons"), 
                          WS_POPUP,
                          CW_USEDEFAULT, 
                          0, 
                          CW_USEDEFAULT, 
                          0,
                          NULL,
                          NULL,
                          hInstance,
                          NULL)) == (HWND) 0)
        return 1 ;
    
    /* get the SystemList handle holding the deskop icon and allocate required resources */ 
    if((diHandle = FindWindow(_T("Progman"),NULL)) == NULL)
    {
        MessageBox(wHnd,_T("Failed to obtain handle to Progman\n"),_T("VWDesktopIcons Error"), MB_ICONERROR) ;
        exit(1) ;
    }
    if((diHandle = FindWindowEx(diHandle,NULL,_T("SHELLDLL_DefView"),NULL)) == NULL)
    {
        MessageBox(wHnd,_T("Failed to obtain handle to SHELLDLL_DefView\n"),_T("VWDesktopIcons Error"), MB_ICONERROR) ;
        exit(1) ;
    }
    if((diHandle = FindWindowEx(diHandle,NULL,_T("SysListView32"),NULL)) == NULL)
    {
        MessageBox(wHnd,_T("Failed to obtain handle to SysListView32\n"),_T("VWDesktopIcons Error"), MB_ICONERROR) ;
        exit(1) ;
    }
    
    GetWindowThreadProcessId(diHandle, &dwPID);
    diProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE,FALSE,dwPID);
    diLocalItem = (LVITEM *) VirtualAlloc(NULL,diItemSize,MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    diShareItem = (char *) VirtualAllocEx(diProcess, NULL, diItemSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if((diLocalItem == NULL) || (diShareItem == NULL))
    {
        MessageBox(wHnd,_T("Failed to malloc required memory\n"),_T("VWDesktopIcons Error"), MB_ICONERROR) ;
        exit(1) ;
    }
    diLocalItem->mask = LVIF_TEXT|LVIF_PARAM ; 
    diLocalItem->iSubItem = 0; 
    
    if(vwWindowsIs64Bit())
    {
        ((vwLVITEM64 *) diLocalItem)->pszText = (TCHAR *) (diShareItem + sizeof(vwLVITEM64) + sizeof(POINT)) ; 
        ((vwLVITEM64 *) diLocalItem)->pszTextUpper = 0 ; 
        ((vwLVITEM64 *) diLocalItem)->cchTextMax = MAX_PATH ; 
    }
    else
    {
        diLocalItem->pszText = (TCHAR *) (diShareItem + sizeof(vwLVITEM64) + sizeof(POINT)) ; 
        diLocalItem->cchTextMax = MAX_PATH ;
    }
    diItemPos = (POINT *) (((char *) diLocalItem) + sizeof(vwLVITEM64)) ;
    diItemName = (TCHAR *) (((char *) diLocalItem) + sizeof(vwLVITEM64) + sizeof(POINT)) ; 
    
    /* get icon list */ 
    memset(autoUpdate,1,vwDESKTOP_SIZE) ;
    memset(showNewIcons,1,vwDESKTOP_SIZE) ;
    if(UpdateDesktopIcons(0,0,0))
    {
        MessageBox(wHnd,_T("Failed to get the initial list of desktop icons\n"),_T("VWDesktopIcons Error"), MB_ICONERROR) ;
        exit(1) ;
    }

    /* main messge loop */
    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    if(shuttingDown == 0)
        SetDesktopIcons(0) ;
    VirtualFree(diLocalItem, 0, MEM_RELEASE);
    VirtualFreeEx(diProcess, diShareItem, 0, MEM_RELEASE);
    CloseHandle(diProcess);
    return 0 ;
}
