/*
* Copyright (c) 2016 AutoChips Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#pragma once

#define FLG_DEF		0x0000		// Default flag
#define XML_SOT		'<'			// Start of tag symbol
#define FLG_SOT		0x0001		// Start of tag flag
#define XML_EOT		'>'			// End of tag symbol
#define FLG_EOT		0x0002		// End of tag flag
#define XML_PRT		'?'			// Prolog tag symbol
#define FLG_PRT		0x0004		// Prolog tag flag
#define XML_SPT		'!'			// Special tag symbol
#define FLG_SPT		0x0008		// Special tag flag
#define XML_ESC		' '			// Escape symbol
#define FLG_ESC		0x0010		// Escape flag
#define XML_CLT		'/'			// Closing tag symbol
#define FLG_CLT		0x0020		// Closing tag flag
#define FLG_OPT		0x0040		// Opening tag flag
#define XML_ATR		'='			// Attribute symbol
#define FLG_ATR		0x0080		// Attribute flag
#define XML_SQT		'\''		// Single quote symbol
#define XML_DQT		'\"'		// Double quote symbol
#define MOD_DEF		0x0000		// Mode "Default"
#define MOD_APN		0x0001		// Mode "Append"
#define MOD_CLS		0x0002		// Mode "Close"

typedef char * LPTSTR;
typedef char _TCHAR;
typedef bool BOOL;
typedef unsigned int DWORD;
typedef unsigned char BYTE;
typedef unsigned char * LPBYTE;

#define _tcslen strlen
#define _tcsncpy strncpy
#define _tcscmp strcmp
#define _tcscpy strcpy
#define _tcsncmp strncmp

#define FALSE false
#define TRUE true

#define _T(x) x
typedef enum _XML_ELEMENT_TYPE
{
	XET_INVALID = 0,
	XET_TAG,
	XET_ATTRIBUTE,
	XET_TEXT

} XML_ELEMENT_TYPE;


class CXMLElement
{
	typedef struct _XMLElementArray
	{
		CXMLElement* pCurrent;
		_XMLElementArray* pNext;

	} XMLElementArray;

public:
	// Public methods
	CXMLElement(void);
	virtual ~CXMLElement(void);
	void Create(LPTSTR lpszElementName, XML_ELEMENT_TYPE type);
	void AppendChild(CXMLElement* lpXMLChild);
	LPTSTR GetElementName();
	XML_ELEMENT_TYPE GetElementType();
	int GetChildNumber();
	CXMLElement* GetFirstChild();
	CXMLElement* GetCurrentChild();
	CXMLElement* GetNextChild();
	CXMLElement* GetLastChild();
	void SetValue(LPTSTR lpszValue);
	LPTSTR GetValue();

private:
	// Private methods
	void Destroy();

private:
	// Private members
	XML_ELEMENT_TYPE m_Type;
	LPTSTR m_lpszName;
	int m_iChildNumber;
	XMLElementArray* m_lpXMLChildArray;
	XMLElementArray* m_lpXMLChildPointer;
	XMLElementArray* m_lpXMLLastChild;
	LPTSTR m_lpszValue;
};


class CXMLFile
{
public:
	// Public methods
	CXMLFile(void);
	virtual ~CXMLFile(void);
	BOOL LoadFromFile(LPTSTR lpszXMLFilePath);
	BOOL LoadFromStream(LPBYTE lpData, DWORD dwDataSize);
	CXMLElement* GetRoot();

private:
	// Private methods
	void Unload();
	BOOL Parse();
	DWORD ExtractAttribute(DWORD offset, LPTSTR lpszAttributeName, LPTSTR lpszAttributeValue);
	BOOL ParseXMLElement(CXMLElement* lpXMLElement, DWORD dwStartOffset, DWORD dwEndOffset);

private:
	// Private members
	DWORD m_dwDataSize;
	BYTE* m_lpData;
	CXMLElement* m_lpXMLElement;
};


