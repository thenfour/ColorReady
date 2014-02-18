/*
written in 1.5 hours before:
- adding coloring to listview
*/

#include "stdafx.h"
#include "AggressiveOptimize.h"
#include <commdlg.h>

//#define SHOWCOLORPICKERONLY

#ifndef SHOWCOLORPICKERONLY

#include "resource.h"
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")
#include "shlwapi.h"
#include "shlobj.h"
#include "..\..\libcc\libcc\StringUtil.hpp"
#include "..\..\libcc\libcc\winapi.hpp"
#include "layoutTool.h"
#include <vector>

HIMAGELIST g_imageList = 0;
AutoPlacement::Manager mgr;
AutoPlacement::Win32GUIMetrics metrics;
LibCC::Log* LibCC::g_pLog = 0;


class ListView
{
	HWND m_hwnd;
public:
	ListView(HWND h) :
		m_hwnd(h)
	{
	}
	void Clear()
	{
		ListView_DeleteAllItems(m_hwnd);
	}
	void Insert(const std::wstring& text, COLORREF clr)
	{
		// create a new icon
		HBRUSH hbr = CreateSolidBrush(clr);
		HBITMAP hbm = 0;
		HDC dc = GetDC(0);
		HDC dc2 = CreateCompatibleDC(dc);
		hbm = CreateCompatibleBitmap(dc, 32, 32);
		HGDIOBJ old = SelectObject(dc2, hbm);
		RECT rc;
		rc.top = 0;
		rc.bottom = 32;
		rc.left = 0;
		rc.right = 32;
		FillRect(dc2, &rc, hbr);
		SelectObject(dc2, old);
		DeleteDC(dc2);
		ReleaseDC(0, dc);
		int imageID = ImageList_Add(g_imageList, hbm, 0);
		DeleteObject(hbm);
		DeleteObject(hbr);

		LVITEMW lvi = {0};
		lvi.mask = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE;
		lvi.iImage = imageID;
		lvi.pszText = (PWSTR)text.c_str();
		lvi.lParam = (LPARAM)clr;
		lvi.iItem = ListView_GetItemCount(m_hwnd) + 1;
		ListView_InsertItem(m_hwnd, &lvi);
	}
};

bool IsDigit(wchar_t ch, bool isHex)
{
	if(isHex)
		return std::wstring(L"0123456789ABCDEFabcdef").find(ch) != std::wstring::npos;

	return ch >= '0' && ch <= '9';
}

void ParseScript(const std::wstring& src, HWND hlist)
{
	ListView list(hlist);
	list.Clear();

	if(g_imageList)
	{
		ImageList_Destroy(g_imageList);
	}
	g_imageList = ImageList_Create(32, 32, ILC_COLOR32, 0, 1);
	ListView_SetImageList(hlist, g_imageList, LVSIL_NORMAL);

	std::vector<std::wstring> lines;

	LibCC::StringSplitByString(src, L"\r", std::back_inserter(lines));

	for(std::vector<std::wstring>::iterator it = lines.begin(); it != lines.end(); ++ it)
	{
		// for each line, just extract 3 numbers.
		std::wstring currentToken;
		std::vector<int> numbers;
		std::wstring runningText;
		bool isInNumber = false;
		bool isPound = false;
		bool isHex = false;
		it->append(L" 0");
		for(std::wstring::iterator itch = it->begin(); itch != it->end(); ++ itch)
		{
			wchar_t ch = *itch;
			if(ch == L'\n')
				continue;

			if(isPound)
			{
				if(IsDigit(ch, true))
				{
					currentToken.push_back(ch);
				}
				else
				{
					// not pound stuff anymore.
					isPound = false;
					if(currentToken.size() == 3)
					{
						numbers.push_back(16 * wcstol(currentToken.substr(0, 1).c_str(), 0, 16));
						numbers.push_back(16 * wcstol(currentToken.substr(1, 1).c_str(), 0, 16));
						numbers.push_back(16 * wcstol(currentToken.substr(2, 1).c_str(), 0, 16));
					}
					else if(currentToken.size() == 6)
					{
						numbers.push_back(wcstol(currentToken.substr(0, 2).c_str(), 0, 16));
						numbers.push_back(wcstol(currentToken.substr(2, 2).c_str(), 0, 16));
						numbers.push_back(wcstol(currentToken.substr(4, 2).c_str(), 0, 16));
					}
					else
					{
						numbers.push_back(wcstol(currentToken.c_str(), 0, 16));
					}
					currentToken.clear();
				}
			}
			else if(isInNumber)
			{
				if((ch == 'x' || ch == 'X') && !isHex)
				{
					isHex = true;
					currentToken.clear();
				}
				else
				{
					if(IsDigit(ch, isHex))
					{
						currentToken.push_back(ch);
					}
					else
					{
						// switch to letters.
						isInNumber = false;
						numbers.push_back(wcstol(currentToken.c_str(), 0, isHex ? 16 : 10));
						currentToken.clear();
					}
				}
			}
			else
			{
				if(IsDigit(ch, false))
				{
					// switch to digits.
					runningText += currentToken;
					currentToken.clear();
					isInNumber = true;
					isHex = false;
				}

				if(ch == '#')
				{
					runningText += currentToken;
					currentToken.clear();
					isPound = true;
				}
				else
				{
					currentToken.push_back(ch);
				}
			}
		}

		// now we have our info.
		runningText = LibCC::StringTrim(runningText, L"\t \r\n");
		if(runningText.empty() && numbers.size())
		{
			runningText = L"Color";
		}

		if(!runningText.empty())
		{
			numbers.push_back(0);
			numbers.push_back(0);
			numbers.push_back(0);
			runningText = LibCC::Format(L"% (%,%,%)")(runningText)(numbers[0])(numbers[1])(numbers[2]).Str();
			list.Insert(runningText, RGB(numbers[0], numbers[1], numbers[2]));
		}
	}
};

HINSTANCE g_hInstance = 0;

bool haveR = false;
bool haveG = false;
bool haveB = false;
bool haveD = false;

BOOL CALLBACK EnumChildWindowsProc(HWND hwnd, LPARAM _h)
{
	wchar_t str[100] = {0};
	GetWindowText(hwnd, str, 99);
	std::wstring t(str);
	if(t == L"&Red:")
	{
		haveR = true;
	}
	if(t == L"&Green:")
	{
		haveG = true;
	}
	if(t == L"Bl&ue:")
	{
		haveB = true;
	}
	if(t == L"&Define Custom Colors >>")
	{
		haveD = true;
	}
	return TRUE;
}

// find the color dialog.
BOOL CALLBACK EnumDialogsProc(HWND hwnd, LPARAM _h)
{
	wchar_t str[100] = {0};
	GetClassName(hwnd, str, 99);
	std::wstring cn(str);
	if(cn == L"#32770")
	{
		// test if this is a color dialog.
		haveR = false;
		haveG = false;
		haveB = false;
		haveD = false;
		EnumChildWindows(hwnd, EnumChildWindowsProc, 0);
		if(haveR && haveG && haveB && haveD)
		{
			*((HWND*)_h) = hwnd;
			return FALSE;
		}
	}
	return TRUE;
}

void SetColor(COLORREF x)
{
	HWND h = 0;
	EnumWindows(EnumDialogsProc, (LPARAM)&h);
	if(!h)
		return;

	HWND hr = GetDlgItem(h, 0x2c2);
	HWND hg = GetDlgItem(h, 0x2c3);
	HWND hb = GetDlgItem(h, 0x2c4);

	// this is it.
	std::wstring r = LibCC::Format()(GetRValue(x)).Str();
	std::wstring g = LibCC::Format()(GetGValue(x)).Str();
	std::wstring b = LibCC::Format()(GetBValue(x)).Str();

	SendMessage(hr, WM_SETTEXT, 0, (LPARAM)r.c_str());
	SendMessage(hg, WM_SETTEXT, 0, (LPARAM)g.c_str());
	SendMessage(hb, WM_SETTEXT, 0, (LPARAM)b.c_str());
}

std::wstring GetPathRelativeToApp(const std::wstring& extra)
{
  wchar_t sz[MAX_PATH];
  wchar_t sz2[MAX_PATH];
  GetModuleFileName(0, sz, MAX_PATH);
	wcscpy(PathFindFileName(sz), extra.c_str());
  PathCanonicalize(sz2, sz);
  return sz2;
}

bool GetSpecialFolderPath(std::wstring& path, int folder)
{
	wchar_t pathBuffer[MAX_PATH];
	if (SHGetSpecialFolderPath(0, pathBuffer, folder, FALSE))
	{
		path = pathBuffer;
		return true;
	}
	return false;
}

std::wstring GenerateAppDataFileName(bool createDirectories)
{
	std::wstring ret;
	GetSpecialFolderPath(ret, CSIDL_APPDATA);
	ret = LibCC::PathJoin(ret, std::wstring(L"Lite Colors"));
	if(createDirectories)
	{
		DWORD attrib = GetFileAttributes(ret.c_str());
		if((attrib == 0xffffffff) || (!(attrib & FILE_ATTRIBUTE_DIRECTORY)))
		{
			CreateDirectory(ret.c_str(), 0);
		}
	}
	ret = LibCC::PathJoin(ret, std::wstring(L"palette.txt"));
	return ret;
}

std::wstring LoadPalette2(const std::wstring& fileName)
{
	HANDLE hfile = CreateFile(fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if(LibCC::IsBadHandle(hfile))
		return L"";
	LibCC::Blob<BYTE> buf;
	DWORD size = GetFileSize(hfile, 0);
	if(size == INVALID_FILE_SIZE || size < 3)// <2 so i don't buffer overrun later in this function. <2 sized files are useless anyway.
	{
		CloseHandle(hfile);
		return L"";
	}
	buf.Alloc(size);
	DWORD br = 0;
	ReadFile(hfile, buf.GetBuffer(), size, &br, 0);
	CloseHandle(hfile);

	// ONLY SUPPORT UTF-8.
	std::wstring ret;
	if(buf[0] == 0xEF && buf[1] == 0xBB && buf[2] == 0xBF)
	{
		// utf-8.
		LibCC::ToUTF16(buf.GetBuffer() + 3, size - 3, ret, CP_UTF8);
	}
	else
	{
		// assume utf-8
		LibCC::ToUTF16(buf.GetBuffer(), size, ret, CP_UTF8);
	}
	return ret;
}

bool loadedFromAppData = true;// set to true because this is the default behavior.

std::wstring LoadPalette(std::wstring& loadedFrom)
{
	// check first in app directory.
	std::wstring localFileName = GetPathRelativeToApp(L"palette.txt");
	if(PathFileExists(localFileName.c_str()))
	{
		loadedFromAppData = false;
		loadedFrom = LibCC::Format(L"Loaded from file: %")(localFileName).Str();
		return LoadPalette2(localFileName);
	}
	// check in appdata
	std::wstring appDataFileName = GenerateAppDataFileName(true);
	if(PathFileExists(appDataFileName.c_str()))
	{
		loadedFrom = LibCC::Format(L"Loaded from file: %")(appDataFileName).Str();
		return LoadPalette2(appDataFileName);
	}

	// default.
	loadedFrom = LibCC::Format(L"Default palette loaded").Str();
	return L"red 255,0 0x0\r\ngreen #00ff00\r\nblue #00f";
}

void SavePalette(const std::wstring& s)
{
	std::wstring fileName = GetPathRelativeToApp(L"palette.txt");;
	if(loadedFromAppData)
		fileName = GenerateAppDataFileName(true);

	HANDLE hfile = CreateFile(fileName.c_str(), GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
	if(LibCC::IsBadHandle(hfile))
		return;
	DWORD br = 0;
	std::string buf;
	buf = LibCC::ToUTF8(s);
	WriteFile(hfile, buf.c_str(), buf.size(), &br, 0);
	CloseHandle(hfile);
}

BOOL CALLBACK DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_SIZE:
		mgr.OnSize(&metrics);
		return TRUE;
	case WM_INITDIALOG:
		{
			std::wstring loadedFrom;
			std::wstring script = LoadPalette(loadedFrom);

			SetDlgItemText(hwnd, IDC_SCRIPT, script.c_str());
			SetWindowText(hwnd, LibCC::Format(L"Color palette utility - %")(loadedFrom).CStr());

			ParseScript(script, GetDlgItem(hwnd, IDC_LIST1));

			// set up autoplacement
			mgr.RegisterSymbol(L"IDC_SCRIPT", AutoPlacement::Win32Control(GetDlgItem(hwnd, IDC_SCRIPT)));
			mgr.RegisterSymbol(L"IDC_LIST1", AutoPlacement::Win32Control(GetDlgItem(hwnd, IDC_LIST1)));

			mgr.RunResource(g_hInstance, MAKEINTRESOURCE(IDR_TXT1), L"TXT", &metrics);
			mgr.OnSize(&metrics);

			return TRUE;
		}
	case WM_NOTIFY:
		{
			NMHDR* nmhdr = (NMHDR*)lParam;
			HWND hlist = GetDlgItem(hwnd, IDC_LIST1);
			if(hlist == nmhdr->hwndFrom)
			{
				if(nmhdr->code == NM_CLICK)
				{
					NMITEMACTIVATE* p = (NMITEMACTIVATE*)lParam;

					LVITEM i = {0};
					i.iItem = p->iItem;
					i.iSubItem = 0;
					i.mask = LVIF_PARAM;
					ListView_GetItem(hlist, &i);

					SetColor((COLORREF)i.lParam);
				}
			}
			return TRUE;
		}
	case WM_CLOSE:
		{
			LibCC::Blob<wchar_t> s;
			GetDlgItemText(hwnd, IDC_SCRIPT, s.GetBuffer(5000), 5000);
			SavePalette(s.GetBuffer());
			EndDialog(hwnd, 0);
			return TRUE;
		}
	case WM_COMMAND:
		if(LOWORD(wParam) == IDC_SCRIPT && HIWORD(wParam) == EN_CHANGE)
		{
			LibCC::Blob<wchar_t> s;
			GetDlgItemText(hwnd, IDC_SCRIPT, s.GetBuffer(5000), 5000);
			ParseScript(s.GetBuffer(), GetDlgItem(hwnd, IDC_LIST1));
		}
		return TRUE;
	}
	return FALSE;
}
#endif


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
#ifdef SHOWCOLORPICKERONLY
	COLORREF cust[16];
	CHOOSECOLOR cc = {0};
	cc.lpCustColors = cust;
	cc.Flags = CC_ANYCOLOR | CC_FULLOPEN;
	cc.lStructSize = sizeof(cc);
	ChooseColor(&cc);
#else
	g_hInstance = hInstance;
	InitCommonControls();

	LibCC::g_pLog = new LibCC::Log(L"LiteColors", hInstance, true, false, false);

	DialogBoxW(hInstance, MAKEINTRESOURCE(IDD_MAIN), 0, DlgProc);

	delete LibCC::g_pLog;

	if(g_imageList)
		ImageList_Destroy(g_imageList);
#endif

	return 0;
}


