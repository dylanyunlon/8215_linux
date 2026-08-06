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


#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <stdint.h>



#include "XMLFile.h"

CXMLFile::CXMLFile(void)
{
	// Init members
	m_dwDataSize = 0;
	m_lpData = NULL;
	m_lpXMLElement = NULL;
}

CXMLFile::~CXMLFile(void)
{
	// Unload previously loaded XML file
	Unload();
}

void CXMLFile::Unload()
{
	// Destroy XML element collection
	if (m_lpXMLElement)
	{
		delete m_lpXMLElement;
		m_lpXMLElement = NULL;
	}

	// Free data buffer
	if (m_lpData != NULL)
	{
		free(m_lpData);
		m_lpData = NULL;
		m_dwDataSize = 0;
	}
}

BOOL CXMLFile::LoadFromFile(LPTSTR lpszXMLFilePath)
{
	BOOL bResult = FALSE;
  int fd;

	// Unload previously loaded XML file
	Unload();

	// Try to open XML file
    fd = open(lpszXMLFilePath, O_RDONLY, 0);
    if (fd < 0) 
    {
        return (bResult);
    }
	
	{
		// Get file length
    m_dwDataSize = lseek(fd, 0, SEEK_END);

		// Read file data
	m_lpData = (BYTE*)malloc(m_dwDataSize*sizeof(BYTE));
	DWORD dwActualSize = 0;
    // Read file to buffer
    lseek(fd, 0, SEEK_SET);
    dwActualSize = read(fd, m_lpData, m_dwDataSize);
		
		if (dwActualSize)
		{

			// Create "XML:ROOT" element
			m_lpXMLElement = new CXMLElement();
			m_lpXMLElement->Create(_T("XML:ROOT"), XET_TAG);

			// Parse XML file
			DWORD dwStartOffset = 0;
			DWORD dwEndOffset = m_dwDataSize;
			if (ParseXMLElement(m_lpXMLElement, dwStartOffset, dwEndOffset))
			{
				bResult = TRUE;
			}
		}

		// Close XML file
    close(fd);
	}

	return bResult;
}

BOOL CXMLFile::LoadFromStream(LPBYTE lpData, DWORD dwDataSize)
{
	BOOL bResult = FALSE;

	// Unload previously loaded XML file
	Unload();

	// Check for valid XML data
	if (lpData != NULL)
	{
		// Get data size
		m_dwDataSize = dwDataSize;

		// Get data
		m_lpData = (BYTE*)malloc(m_dwDataSize*sizeof(BYTE));
		memcpy(m_lpData, lpData, m_dwDataSize);

		// Create "XML:ROOT" element
		m_lpXMLElement = new CXMLElement();
		m_lpXMLElement->Create(_T("XML:ROOT"), XET_TAG);

		// Parse XML data
		DWORD dwStartOffset = 0;
		DWORD dwEndOffset = m_dwDataSize;
		if (ParseXMLElement(m_lpXMLElement, dwStartOffset, dwEndOffset))
		{
			bResult = TRUE;
		}
	}

	return bResult;
}


BOOL CXMLFile::ParseXMLElement(CXMLElement* lpXMLElement, DWORD dwStartOffset, DWORD dwEndOffset)
{
	BOOL bResult = FALSE;

	// Check for valid data
	if (m_lpData != NULL)
	{
		// Parse XML data
		BOOL bPlainText = TRUE;
		DWORD i = dwStartOffset;
		DWORD dwAttributeOffset;
		DWORD dwFlags = FLG_DEF;
		DWORD dwMode = MOD_DEF;
		_TCHAR lpName[1024], lpOpenTagName[1024], lpCloseTagName[1024], lpAttributeName[1024], lpAttributeValue[1024];
		int iNameLength = 0;
		DWORD startOffset, endOffset;
		_tcscpy((LPTSTR)lpOpenTagName, _T(""));
		_tcscpy((LPTSTR)lpCloseTagName, _T(""));
		while ((i <= dwEndOffset) && (i < m_dwDataSize))
		{
			// Check for input character
			switch (m_lpData[i])
			{
				case XML_SOT:
				{
					// Update flags
					dwFlags |= FLG_SOT;
					dwFlags |= FLG_OPT;
					bPlainText = FALSE;

					// Clear name
					_tcscpy((LPTSTR)lpName, _T(""));
					iNameLength = 0;

					// Set new mode
					dwMode = MOD_DEF;
				}
				break;

				case XML_EOT:
				{
					// Update flags
					dwFlags |= FLG_EOT;

					// Set new mode
					dwMode = MOD_CLS;

					dwStartOffset = i + 1;
				}
				break;

				case XML_PRT:
				{
					// Update flags
					dwFlags |= FLG_PRT;

					// Set new mode
					dwMode = MOD_DEF;
				}
				break;

				case XML_SPT:
				{
					// Update flags
					dwFlags |= FLG_SPT;

					// Set new mode
					dwMode = MOD_DEF;
				}
				break;

				case XML_ESC:
				{
					// Update flags
					dwFlags |= FLG_ESC;

					// Set new mode
					if (dwMode != MOD_DEF)
					{
						dwMode = MOD_CLS;
					}
				}
				break;

				case XML_CLT:
				{
					// Update flags
					dwFlags |= FLG_CLT;
				}
				break;

				case XML_ATR:
				{
					// Update flags
					dwFlags |= FLG_ATR;

					// Clear attribute
					_tcscpy((LPTSTR)lpAttributeName, _T(""));
					_tcscpy((LPTSTR)lpAttributeValue, _T(""));
				}
				break;

				default:
				{
					// Default (do nothing)
				}
			}

			// Check mode
			switch (dwMode)
			{
				case MOD_APN:
				{
					// Append name
					lpName[iNameLength++] = m_lpData[i];
				}
				break;

				case MOD_CLS:
				{
					// Close name
					lpName[iNameLength++] = '\0';

					// Check for opening tag
					if (dwFlags & FLG_OPT)
					{
						// Update flags
						dwFlags &= ~FLG_OPT;

						// Check for opened tag
						if (_tcscmp(lpOpenTagName, _T("")) == 0)
						{
							// Update start offset
							startOffset = i + 1;

							// Save tag name
							_tcsncpy(lpOpenTagName, lpName, iNameLength);
							lpCloseTagName[0] = '/';
							_tcsncpy(lpCloseTagName+1, lpOpenTagName, iNameLength);
						}
						// Compare tag names
						else if (_tcsncmp(lpName, lpCloseTagName, iNameLength) == 0)
						{
							// Update end offset
							endOffset = i - iNameLength - 1;

							// Create new XML child element
							CXMLElement* pNewXMLElement = new CXMLElement();
							pNewXMLElement->Create(lpOpenTagName, XET_TAG);
							lpXMLElement->AppendChild(pNewXMLElement);

							// Parse child XML element
							bResult = ParseXMLElement(pNewXMLElement, startOffset, endOffset);
							if (!bResult)
							{
								return FALSE;
							}

							// Clear opening and closing tag name
							_tcscpy(lpOpenTagName, _T(""));
							_tcscpy(lpCloseTagName, _T(""));
						}
						else
						{
							bResult = FALSE;
						}
					}
					// Check for closing tag
					if (dwFlags & FLG_CLT)
					{
						// Update flags
						dwFlags &= ~FLG_CLT;
					}
				}
				break;

				case MOD_DEF:
				{
					// Default (do nothing)
				}
				break;
			}

			// Check flags
			if (dwFlags & FLG_SOT)
			{
				// Set new mode
				dwMode = MOD_APN;

				// Update flags
				dwFlags &= ~FLG_SOT;
			}
			else if (dwFlags & FLG_EOT)
			{
				// Set new mode
				dwMode = MOD_DEF;

				// Update flags
				dwFlags &= ~FLG_EOT;
			}
			else if (dwFlags & FLG_PRT)
			{
				// Set new mode
				dwMode = MOD_DEF;

				// Update flags
				dwFlags &= ~FLG_PRT;
			}
			else if (dwFlags & FLG_ESC)
			{
				// Set new mode
				dwMode = MOD_DEF;

				// Update flags
				dwFlags &= ~FLG_ESC;
			}
			else if (dwFlags & FLG_ATR)
			{
				// Extract attribute name and value
				dwAttributeOffset = ExtractAttribute(i, lpAttributeName, lpAttributeValue);

				// Update flags
				dwFlags &= ~FLG_ATR;
				i=dwAttributeOffset;
			}

			// Check for valid attribute
			if (i == dwAttributeOffset)
			{
				// Check for valid tag
				if (_tcscmp(lpOpenTagName, _T("")) == 0)
				{
					// Create new XML child element
					CXMLElement* pNewXMLElement = new CXMLElement();
					pNewXMLElement->Create(lpAttributeName, XET_ATTRIBUTE);
					pNewXMLElement->SetValue(lpAttributeValue);
					lpXMLElement->AppendChild(pNewXMLElement);
				}
			}

			// Increment offset
			i++;
		}

		// Check for XML text element
		if (bPlainText)
		{
			// Extract element name
			int iNameLen = dwEndOffset - dwStartOffset + 1;
			LPTSTR lpszElementName = (LPTSTR)malloc((iNameLen+1)*sizeof(_TCHAR));
			int k = 0;
			for (DWORD j=dwStartOffset; j<=dwEndOffset; j++)
			{
				lpszElementName[k++] = m_lpData[j];
			}
			lpszElementName[k] = '\0';

			// Create new XML child element
			CXMLElement* pNewXMLElement = new CXMLElement();
			pNewXMLElement->Create(lpszElementName, XET_TEXT);
			lpXMLElement->AppendChild(pNewXMLElement);
			free(lpszElementName);

			bResult = TRUE;
		}
	}

	return bResult;
}

DWORD CXMLFile::ExtractAttribute(DWORD offset, LPTSTR lpszAttributeName, LPTSTR lpszAttributeValue)
{
	DWORD dwResult = 0;

	// Extract attribute name
	int k = 0;
	DWORD dwNameOffset = offset - 1;
	while ((dwNameOffset >= 0) && (m_lpData[dwNameOffset] != XML_ESC) && (m_lpData[dwNameOffset] != XML_SOT) &&
		(m_lpData[dwNameOffset] != XML_EOT) && (m_lpData[dwNameOffset] != XML_PRT) && (m_lpData[dwNameOffset] != XML_SPT))
	{
		if ((m_lpData[dwNameOffset] != XML_SQT) && (m_lpData[dwNameOffset] != XML_DQT))
		{
			lpszAttributeName[k++] = m_lpData[dwNameOffset];
		}
		dwNameOffset--;
	}
	lpszAttributeName[k] = '\0';
	// Reverse attribute name
	_TCHAR tchTmp;
	for (int i=0; i<k/2; i++)
	{
		tchTmp = lpszAttributeName[i];
		lpszAttributeName[i] = lpszAttributeName[k-i-1];
		lpszAttributeName[k-i-1] = tchTmp;
	}

	// Extract attribute value
	DWORD dwStartAttributeOffset=0, dwEndAttributeOffset=0;
	DWORD dwValueOffset = offset + 1;
	while (dwValueOffset < m_dwDataSize)
	{
		if ((m_lpData[dwValueOffset] == XML_SQT) || (m_lpData[dwValueOffset] == XML_DQT))
		{
			dwStartAttributeOffset = dwValueOffset + 1;
			break;
		}
		dwValueOffset++;
	}
	dwValueOffset++;
	while (dwValueOffset < m_dwDataSize)
	{
		if ((m_lpData[dwValueOffset] == XML_SQT) || (m_lpData[dwValueOffset] == XML_DQT))
		{
			dwEndAttributeOffset = dwValueOffset - 1;
			break;
		}
		dwValueOffset++;
	}
	k = 0;
	for (DWORD j=dwStartAttributeOffset; j<=dwEndAttributeOffset; j++)
	{
		lpszAttributeValue[k++] = m_lpData[j];
	}
	lpszAttributeValue[k] = '\0';

/*	DWORD dwValueOffset = offset + 1;
	k = 0;
	while ((dwValueOffset < m_dwDataSize) && (m_lpData[dwValueOffset] != XML_ESC) && (m_lpData[dwValueOffset] != XML_SOT) &&
		(m_lpData[dwValueOffset] != XML_EOT) && (m_lpData[dwValueOffset] != XML_PRT) && (m_lpData[dwValueOffset] != XML_SPT))
	{
		if ((m_lpData[dwValueOffset] != XML_SQT) && (m_lpData[dwValueOffset] != XML_DQT))
		{
			lpszAttributeValue[k++] = m_lpData[dwValueOffset];
		}
		dwValueOffset++;
	}
	lpszAttributeValue[k] = '\0';*/

	dwResult = dwValueOffset;
	return dwResult;
}


CXMLElement* CXMLFile::GetRoot()
{
	// Return XML file root element
	return m_lpXMLElement;
}


CXMLElement::CXMLElement(void)
{
	// Init members
	m_Type = XET_INVALID;
	m_lpszName = NULL;
	m_iChildNumber = 0;
	m_lpXMLChildArray = NULL;
	m_lpXMLChildPointer = NULL;
	m_lpXMLLastChild = NULL;
	m_lpszValue = NULL;
}

CXMLElement::~CXMLElement(void)
{
	// Destroy XML element
	Destroy();
}

void CXMLElement::Destroy()
{
	// Destroy child elements
	XMLElementArray *pPrev, *pCurr;
	pCurr = m_lpXMLChildArray;
	pPrev = NULL;
	while (pCurr != NULL)
	{
		pPrev = pCurr;
		pCurr = pCurr->pNext;
		delete pPrev->pCurrent;
		delete pPrev;
	}

	// Destroy XML element
	m_Type = XET_INVALID;
	if (m_lpszName != NULL)
	{
		free(m_lpszName);
		m_lpszName = NULL;
	}

	// Destroy value
	if (m_lpszValue != NULL)
	{
		free(m_lpszValue);
		m_lpszValue = NULL;
	}
}

void CXMLElement::Create(LPTSTR lpszElementName, XML_ELEMENT_TYPE type)
{
	// Check for valid XML element name
	if (lpszElementName != NULL)
	{
		// Create new XML element
		m_Type = type;
		int iNameLen = (int)_tcslen(lpszElementName);
		m_lpszName = (LPTSTR)malloc((iNameLen+1)*sizeof(_TCHAR));
		_tcsncpy(m_lpszName, lpszElementName, iNameLen);
		m_lpszName[iNameLen] = '\0';
	}
}

void CXMLElement::AppendChild(CXMLElement* lpXMLChild)
{
	// Check for valid XML child element
	if (lpXMLChild != NULL)
	{
		// Create new XMLElementArray element
		XMLElementArray* pNewXMLChild = new XMLElementArray;
		pNewXMLChild->pCurrent = lpXMLChild;
		pNewXMLChild->pNext = NULL;

		// Add new XML child element
		m_iChildNumber++;
		if (m_lpXMLChildArray == NULL)
		{
			// Add root element
			m_lpXMLChildArray = pNewXMLChild;
			m_lpXMLChildPointer = m_lpXMLChildArray;
			m_lpXMLLastChild = m_lpXMLChildArray;
		}
		else
		{
			// Append to last element
			m_lpXMLLastChild->pNext = pNewXMLChild;
			m_lpXMLLastChild = pNewXMLChild;
		}
	}
}

LPTSTR CXMLElement::GetElementName()
{
	// Return XML element name
	return m_lpszName;
}

XML_ELEMENT_TYPE CXMLElement::GetElementType()
{
	// Return XML element type
	return m_Type;
}

int CXMLElement::GetChildNumber()
{
	// Return XML element child number
	return m_iChildNumber;
}

CXMLElement* CXMLElement::GetFirstChild()
{
	CXMLElement* pResult = NULL;

	// Check for valid child collection pointer
	m_lpXMLChildPointer = m_lpXMLChildArray;
	if (m_lpXMLChildPointer != NULL)
	{
		// Return first XML element child collection element
		pResult = m_lpXMLChildPointer->pCurrent;
	}

	return pResult;
}

CXMLElement* CXMLElement::GetCurrentChild()
{
	CXMLElement* pResult = NULL;

	// Check for valid child collection pointer
	if (m_lpXMLChildPointer != NULL)
	{
		// Return current XML element child collection element
		pResult = m_lpXMLChildPointer->pCurrent;
	}

	return pResult;
}

CXMLElement* CXMLElement::GetNextChild()
{
	CXMLElement* pResult = NULL;

	// Check for valid child collection pointer
	if (m_lpXMLChildPointer != NULL)
	{
		// Get next XML element child collection element
		m_lpXMLChildPointer = m_lpXMLChildPointer->pNext;

		// Return XML element child collection next element
		if (m_lpXMLChildPointer != NULL)
		{
			pResult = m_lpXMLChildPointer->pCurrent;
		}
	}

	return pResult;
}

CXMLElement* CXMLElement::GetLastChild()
{
	CXMLElement* pResult = NULL;

	// Check for valid child collection pointer
	if (m_lpXMLLastChild != NULL)
	{
		// Get last XML element child collection element
		pResult = m_lpXMLLastChild->pCurrent;
	}

	return pResult;
}

void CXMLElement::SetValue(LPTSTR lpszValue)
{
	// Check for valie value
	if (lpszValue != NULL)
	{
		// Delete previous value
		if (m_lpszValue != NULL)
		{
			free(m_lpszValue);
			m_lpszValue = NULL;
		}

		// Set element value
		int iValueLen = (int)_tcslen(lpszValue);
		m_lpszValue = (LPTSTR)malloc((iValueLen+1)*sizeof(_TCHAR));
		_tcsncpy(m_lpszValue, lpszValue, iValueLen);
		m_lpszValue[iValueLen] = '\0';
	}
}

LPTSTR CXMLElement::GetValue()
{
	// Return element value
	return m_lpszValue;
}
