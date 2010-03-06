///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/stockitem.cpp
// Purpose:     Stock buttons, menu and toolbar items labels
// Author:      Vaclav Slavik
// Modified by:
// Created:     2004-08-15
// RCS-ID:      $Id: stockitem.cpp 58617 2009-02-02 05:12:43Z SC $
// Copyright:   (c) Vaclav Slavik, 2004
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/stockitem.h"

#ifndef WX_PRECOMP
    #include "wx/intl.h"
    #include "wx/utils.h" // for wxStripMenuCodes()
#endif

bool wxIsStockID(wxWindowID id)
{
    switch (id)
    {
        case wxID_ABOUT:
        case wxID_ADD:
        case wxID_APPLY:
        case wxID_BOLD:
        case wxID_CANCEL:
        case wxID_CLEAR:
        case wxID_CLOSE:
        case wxID_COPY:
        case wxID_CUT:
        case wxID_DELETE:
        case wxID_EDIT:
        case wxID_FIND:
        case wxID_FILE:
        case wxID_REPLACE:
        case wxID_BACKWARD:
        case wxID_DOWN:
        case wxID_FORWARD:
        case wxID_UP:
        case wxID_HELP:
        case wxID_HOME:
        case wxID_INDENT:
        case wxID_INDEX:
        case wxID_ITALIC:
        case wxID_JUSTIFY_CENTER:
        case wxID_JUSTIFY_FILL:
        case wxID_JUSTIFY_LEFT:
        case wxID_JUSTIFY_RIGHT:
        case wxID_NEW:
        case wxID_NO:
        case wxID_OK:
        case wxID_OPEN:
        case wxID_PASTE:
        case wxID_PREFERENCES:
        case wxID_PRINT:
        case wxID_PREVIEW:
        case wxID_PROPERTIES:
        case wxID_EXIT:
        case wxID_REDO:
        case wxID_REFRESH:
        case wxID_REMOVE:
        case wxID_REVERT_TO_SAVED:
        case wxID_SAVE:
        case wxID_SAVEAS:
        case wxID_SELECTALL:
        case wxID_STOP:
        case wxID_UNDELETE:
        case wxID_UNDERLINE:
        case wxID_UNDO:
        case wxID_UNINDENT:
        case wxID_YES:
        case wxID_ZOOM_100:
        case wxID_ZOOM_FIT:
        case wxID_ZOOM_IN:
        case wxID_ZOOM_OUT:
            return true;

        default:
            return false;
    }
}

wxString wxGetStockLabel(wxWindowID id, long flags)
{
    wxString stockLabel;

    #define STOCKITEM(stockid, label) \
        case stockid:                 \
            stockLabel = label;       \
            break;

    switch (id)
    {
        STOCKITEM(wxID_ABOUT,               _("����(&A)"))
        STOCKITEM(wxID_ADD,                 _("���"))
        STOCKITEM(wxID_APPLY,               _("Ӧ��(&A)"))
        STOCKITEM(wxID_BOLD,                _("&Bold"))
        STOCKITEM(wxID_CANCEL,              _("ȡ��(&C)"))
        STOCKITEM(wxID_CLEAR,               _("���(&C)"))
        STOCKITEM(wxID_CLOSE,               _("�ر�(&C)"))
        STOCKITEM(wxID_COPY,                _("����(&C)"))
        STOCKITEM(wxID_CUT,                 _("����(&T)"))
        STOCKITEM(wxID_DELETE,              _("ɾ��(&D)"))
        STOCKITEM(wxID_EDIT,                _("�༭(&E)"))
        STOCKITEM(wxID_FIND,                _("����(&F)"))
        STOCKITEM(wxID_FILE,                _("�ļ�(&F)"))
        STOCKITEM(wxID_REPLACE,             _("�滻(&L)"))
        STOCKITEM(wxID_BACKWARD,            _("��һ��(&B)"))
        STOCKITEM(wxID_DOWN,                _("��(&D)"))
        STOCKITEM(wxID_FORWARD,             _("��һ��(&F)"))
        STOCKITEM(wxID_UP,                  _("&Up"))
        STOCKITEM(wxID_HELP,                _("����(&H)"))
        STOCKITEM(wxID_HOME,                _("&Home"))
        STOCKITEM(wxID_INDENT,              _("Indent"))
        STOCKITEM(wxID_INDEX,               _("&Index"))
        STOCKITEM(wxID_ITALIC,              _("&Italic"))
        STOCKITEM(wxID_JUSTIFY_CENTER,      _("����"))
        STOCKITEM(wxID_JUSTIFY_FILL,        _("Justified"))
        STOCKITEM(wxID_JUSTIFY_LEFT,        _("Align Left"))
        STOCKITEM(wxID_JUSTIFY_RIGHT,       _("Align Right"))
        STOCKITEM(wxID_NEW,                 _("�½�(&N)"))
        STOCKITEM(wxID_NO,                  _("��(&N)"))
        STOCKITEM(wxID_OK,                  _("ȷ��(&O)"))
        STOCKITEM(wxID_OPEN,                _("��(&O)"))
        STOCKITEM(wxID_PASTE,               _("ճ��(&P)"))
        STOCKITEM(wxID_PREFERENCES,         _("����(&P)"))
        STOCKITEM(wxID_PRINT,               _("��ӡ(&P)"))
        STOCKITEM(wxID_PREVIEW,             _("��ӡԤ��(&W)"))
        STOCKITEM(wxID_PROPERTIES,          _("����(&P)"))
        STOCKITEM(wxID_EXIT,                _("�˳�(&Q)"))
        STOCKITEM(wxID_REDO,                _("����(&R)"))
        STOCKITEM(wxID_REFRESH,             _("ˢ��(&R)"))
        STOCKITEM(wxID_REMOVE,              _("�Ƴ�"))
        STOCKITEM(wxID_REVERT_TO_SAVED,     _("Revert to Saved"))
        STOCKITEM(wxID_SAVE,                _("����(&S)"))
        STOCKITEM(wxID_SAVEAS,              _("���Ϊ(&A)..."))
        STOCKITEM(wxID_SELECTALL,           _("ȫѡ"))
        STOCKITEM(wxID_STOP,                _("ֹͣ(&S)"))
        STOCKITEM(wxID_UNDELETE,            _("Undelete"))
        STOCKITEM(wxID_UNDERLINE,           _("�»���(&U)"))
        STOCKITEM(wxID_UNDO,                _("����(&U)"))
        STOCKITEM(wxID_UNINDENT,            _("&Unindent"))
        STOCKITEM(wxID_YES,                 _("��(&Y)"))
        STOCKITEM(wxID_ZOOM_100,            _("ʵ�ʴ�С(&A)"))
        STOCKITEM(wxID_ZOOM_FIT,            _("Zoom to &Fit"))
        STOCKITEM(wxID_ZOOM_IN,             _("�Ŵ�(&I)"))
        STOCKITEM(wxID_ZOOM_OUT,            _("��С(&O)"))

        default:
            wxFAIL_MSG( _T("invalid stock item ID") );
            break;
    };

    #undef STOCKITEM

    if ( !(flags & wxSTOCK_WITH_MNEMONIC) )
    {
        stockLabel = wxStripMenuCodes(stockLabel);
    }

#if wxUSE_ACCEL
    if ( !stockLabel.empty() && (flags & wxSTOCK_WITH_ACCELERATOR) )
    {
        wxAcceleratorEntry accel = wxGetStockAccelerator(id);
        if (accel.IsOk())
            stockLabel << _T('\t') << accel.ToString();
    }
#endif // wxUSE_ACCEL

    return stockLabel;
}

wxString wxGetStockHelpString(wxWindowID id, wxStockHelpStringClient client)
{
    wxString stockHelp;

    #define STOCKITEM(stockid, ctx, helpstr)             \
        case stockid:                                    \
            if (client==ctx) stockHelp = helpstr;        \
            break;

    switch (id)
    {
        // NB: these help string should be not too specific as they could be used
        //     in completely different programs!
        STOCKITEM(wxID_ABOUT,    wxSTOCK_MENU, _("Show about dialog"))
        STOCKITEM(wxID_COPY,     wxSTOCK_MENU, _("������ѡ"))
        STOCKITEM(wxID_CUT,      wxSTOCK_MENU, _("������ѡ"))
        STOCKITEM(wxID_DELETE,   wxSTOCK_MENU, _("ɾ����ѡ"))
        STOCKITEM(wxID_REPLACE,  wxSTOCK_MENU, _("�滻��ѡ"))
        STOCKITEM(wxID_PASTE,    wxSTOCK_MENU, _("ճ����ѡ"))
        STOCKITEM(wxID_EXIT,     wxSTOCK_MENU, _("�˳��˳���"))
        STOCKITEM(wxID_REDO,     wxSTOCK_MENU, _("Redo last action"))
        STOCKITEM(wxID_UNDO,     wxSTOCK_MENU, _("Undo last action"))
        STOCKITEM(wxID_CLOSE,    wxSTOCK_MENU, _("�رյ�ǰ�ĵ�"))
        STOCKITEM(wxID_SAVE,     wxSTOCK_MENU, _("Save current document"))
        STOCKITEM(wxID_SAVEAS,   wxSTOCK_MENU, _("Save current document with a different filename"))

        default:
            // there's no stock help string for this ID / client
            return wxEmptyString;
    }

    #undef STOCKITEM

    return stockHelp;
}

#if wxUSE_ACCEL

wxAcceleratorEntry wxGetStockAccelerator(wxWindowID id)
{
    wxAcceleratorEntry ret;

    #define STOCKITEM(stockid, flags, keycode)      \
        case stockid:                               \
            ret.Set(flags, keycode, stockid);       \
            break;

    switch (id)
    {
        STOCKITEM(wxID_COPY,                wxACCEL_CMD,'C')
        STOCKITEM(wxID_CUT,                 wxACCEL_CMD,'X')
        STOCKITEM(wxID_FIND,                wxACCEL_CMD,'F')
        STOCKITEM(wxID_REPLACE,             wxACCEL_CMD,'R')
        STOCKITEM(wxID_HELP,                wxACCEL_CMD,'H')
        STOCKITEM(wxID_NEW,                 wxACCEL_CMD,'N')
        STOCKITEM(wxID_OPEN,                wxACCEL_CMD,'O')
        STOCKITEM(wxID_PASTE,               wxACCEL_CMD,'V')
        STOCKITEM(wxID_SAVE,                wxACCEL_CMD,'S')

        default:
            // set the wxAcceleratorEntry to return into an invalid state:
            // there's no stock accelerator for that.
            ret.Set(0, 0, id);
            break;
    };

    #undef STOCKITEM

    // always use wxAcceleratorEntry::IsOk on returned value !
    return ret;
}

#endif // wxUSE_ACCEL

bool wxIsStockLabel(wxWindowID id, const wxString& label)
{
    if (label.empty())
        return true;

    wxString stock = wxGetStockLabel(id);

    if (label == stock)
        return true;

    stock.Replace(_T("&"), wxEmptyString);
    if (label == stock)
        return true;

    return false;
}
