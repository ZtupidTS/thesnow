/*
 * xmlfunctions.h declares some useful xml helper functions, especially to
 * improve usability together with wxWidgets.
 */

#ifndef __XMLFUNCTIONS_H__
#define __XMLFUNCTIONS_H__

#ifdef HAVE_LIBTINYXML
#include <tinyxml.h>
#else
#include "../tinyxml/tinyxml.h"
#endif

class CXmlFile
{
public:
	CXmlFile(const wxString& fileName);
	CXmlFile(const wxFileName& fileName = wxFileName());

	virtual ~CXmlFile();

	TiXmlElement* CreateEmpty();

	void SetFileName(const wxString& name);
	void SetFileName(const wxFileName& fileName);
	const wxFileName& GetFileName() const { return m_fileName; }

	bool HasFileName() const { return m_fileName.IsOk(); }

	// Sets error description on failure
	TiXmlElement* Load(const wxString& name);
	TiXmlElement* Load(const wxFileName& fileName = wxFileName());

	wxString GetError() const { return m_error; }
	int GetRawDataLength();
	void GetRawDataHere(char* p); // p has to big enough to hold at least GetRawDataLength() bytes

	bool ParseData(char* data); // data has to be 0-terminated

	void Close();

	TiXmlElement* GetElement();
	const TiXmlElement* GetElement() const;

	bool Modified();

	bool Save(wxString* error = 0);

protected:
	wxDateTime m_modificationTime;
	wxFileName m_fileName;
	TiXmlDocument* m_pDocument;
	TiXmlPrinter *m_pPrinter;

	wxString m_error;
};

// Convert the given string into an UTF-8 string. Returned string has to be deleted manually.
char* ConvUTF8(const wxString& value);

// Convert the given utf-8 string into wxString
wxString ConvLocal(const char *value);

void SetTextAttribute(TiXmlElement* node, const char* name, const wxString& value);
wxString GetTextAttribute(TiXmlElement* node, const char* name);

int GetAttributeInt(TiXmlElement* node, const char* name);
void SetAttributeInt(TiXmlElement* node, const char* name, int value);

TiXmlElement* FindElementWithAttribute(TiXmlElement* node, const char* element, const char* attribute, const char* value);
TiXmlElement* FindElementWithAttribute(TiXmlElement* node, const char* element, const char* attribute, int value);

// Add a new child element with the specified name and value to the xml document
void AddTextElement(TiXmlElement* node, const char* name, const wxString& value);
void AddTextElement(TiXmlElement* node, const char* name, int value);
void AddTextElementRaw(TiXmlElement* node, const char* name, const char* value);

// Set the current element's text value
void AddTextElement(TiXmlElement* node, const wxString& value);
void AddTextElement(TiXmlElement* node, int value);
void AddTextElementRaw(TiXmlElement* node, const char* value);

// Get string from named child element
wxString GetTextElement(TiXmlElement* node, const char* name);
wxString GetTextElement_Trimmed(TiXmlElement* node, const char* name);

// Get string from current element
wxString GetTextElement(TiXmlElement* node);
wxString GetTextElement_Trimmed(TiXmlElement* node);

// Get (64-bit) integer from named element
int GetTextElementInt(TiXmlElement* node, const char* name, int defValue = 0);
wxLongLong GetTextElementLongLong(TiXmlElement* node, const char* name, int defValue = 0);

bool GetTextElementBool(TiXmlElement* node, const char* name, bool defValue = false);

// Opens the specified XML file if it exists or creates a new one otherwise.
// Returns 0 on error.
TiXmlElement* GetXmlFile(wxFileName file, wxString *error = 0);

// Save the XML document to the given file
bool SaveXmlFile(const wxFileName& file, TiXmlNode* node, wxString* error = 0, bool move = false);

// Functions to save and retrieve CServer objects to the XML file
void SetServer(TiXmlElement *node, const CServer& server);
bool GetServer(TiXmlElement *node, CServer& server);

bool LoadXmlDocument(TiXmlDocument* pXmlDocument, const wxString& file, wxString* error = 0);

#endif //__XMLFUNCTIONS_H__
