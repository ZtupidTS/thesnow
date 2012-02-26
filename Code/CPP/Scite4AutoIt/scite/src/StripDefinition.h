// SciTE - Scintilla based Text Editor
/** @file StripDefinition.h
 ** Definition of classes build user strips from a description string.
 **/
// Copyright 2012 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

class UserControl {
public:
	enum UCControlType { ucStatic, ucEdit, ucCombo, ucButton } controlType;
	GUI::gui_string text;
	int item;
	bool fixedWidth;
	int widthDesired;
	int widthAllocated;
	GUI::Window w;
	UserControl(UCControlType controlType_, GUI::gui_string text_) : 
		controlType(controlType_),
		text(text_),
		item(0),
		fixedWidth(true),
		widthDesired(20),
		widthAllocated(20)	{
	}
};

struct ColumnWidth {
	int widthDesired;
	int widthAllocated;
	bool isResizeable;
	ColumnWidth() : widthDesired(0), widthAllocated(0), isResizeable(false) {
	}
};

class StripDefinition {
public:
	bool hasClose;
	unsigned int columns;
	std::vector<std::vector<UserControl> > controls;
	std::vector<ColumnWidth> widths;

	StripDefinition(GUI::gui_string definition) {
		hasClose = false;
		controls.clear();
		columns = 0;
		controls.push_back(std::vector<UserControl>());
		const GUI::gui_char *pdef=definition.c_str();
		int line = 0;
		unsigned int column = 0;
		while (*pdef) {
			if (*pdef == '\n') {
				controls.push_back(std::vector<UserControl>());
				column = 0;
				line++;
				pdef++;
				continue;
			}
			if (*pdef == '!') {
				hasClose = true;
				pdef++;
				continue;
			}
			UserControl::UCControlType controlType = UserControl::ucStatic;
			GUI::gui_char endChar = 0;
			switch (*pdef) {
				case '\'':
					controlType = UserControl::ucStatic;
					endChar = '\'';
					break;
				case '[':
					controlType = UserControl::ucEdit;
					endChar = ']';
					break;
				case '{':
					controlType = UserControl::ucCombo;
					endChar = '}';
					break;
				case '(':
					controlType = UserControl::ucButton;
					endChar = ')';
					break;
			}
			pdef++;
			GUI::gui_string text;
			while (*pdef && (*pdef != endChar)) {
				text += *pdef;
				pdef++;
			}
			controls.back().push_back(UserControl(controlType, text));
			column++;
			if (columns < column)
				columns = column;
			if (*pdef)
				pdef++;
		}
	}

	void CalculateColumnWidths(int widthToAllocate) {
		widths.clear();

		int widthOfNonResizeables = 4 * (columns - 1);
		int resizeables = 0;
		for (size_t column=0; column<columns; column++) {
			ColumnWidth cw;
			for (size_t line=0; line<controls.size(); line++) {
				if (column <  controls[line].size()) {
					UserControl *ctl = &controls[line][column];
					if (ctl->fixedWidth) {
						if  (cw.widthDesired < ctl->widthDesired)
							cw.widthDesired = ctl->widthDesired;
					} else {
						cw.isResizeable = true;
					}
				}
			}
			widths.push_back(cw);
			widthOfNonResizeables += cw.widthDesired;
			if (cw.isResizeable)
				resizeables++;
		}
		int widthSpareEach = (widthToAllocate - widthOfNonResizeables) / resizeables;
		for (std::vector<ColumnWidth>::iterator cw=widths.begin(); cw != widths.end(); ++cw) {
			if (cw->isResizeable) {
				cw->widthAllocated = widthSpareEach + cw->widthDesired;
			} else {
				cw->widthAllocated = cw->widthDesired;
			}
		}
	}

	UserControl *FindControl(int control) {
		int controlID=0;
		for (std::vector<std::vector<UserControl> >::iterator line=controls.begin(); line != controls.end(); ++line) {
			for (std::vector<UserControl>::iterator ctl=line->begin(); ctl != line->end(); ++ctl) {
				if (controlID == control) {
					return &(*ctl);
				}
				controlID++;
			}
		}
		return 0;
	}
};

enum StripCommand { scUnknown, scClicked, scChange, scFocusIn, scFocusOut };
