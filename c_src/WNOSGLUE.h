/*
	WNOSGLUE.h

	Copyright (C) 2002 Philip Cummins, Weston Pawlowski,
	Bradford L. Barrett, Paul Pratt

	You can redistribute this file and/or modify it under the terms
	of version 2 of the GNU General Public License as published by
	the Free Software Foundation.  You should have received a copy
	of the license along with this file; see the file COPYING.

	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	license for more details.
*/

/*
	microsoft WiNdows Operating System GLUE.

	All operating system dependent code for the
	Microsoft Windows platform should go here.

	This code is descended from Weston Pawlowski's Windows
	port of vMac, by Philip Cummins.

	The main entry point 'WinMain' is at the end of this file.
*/

#include "RESIDWIN.h"

#define InstallFileIcons 0

/*--- some simple utilities ---*/

#define PowOf2(p) ((unsigned long)1 << (p))
#define TestBit(i, p) (((unsigned long)(i) & PowOf2(p)) != 0)

GLOBALPROC MyMoveBytes(anyp srcPtr, anyp destPtr, si5b byteCount)
{
/*
	must work even if blocks overlap in memory
*/
	(void) memcpy((char *)destPtr, (char *)srcPtr, byteCount);
}

/*--- basic dialogs ---*/

LOCALVAR HWND MainWnd;

GLOBALPROC MacMsg(char *briefMsg, char *longMsg, blnr fatal)
{
	UnusedParam(fatal);
	MessageBox(MainWnd, longMsg, briefMsg, MB_APPLMODAL|MB_OK);
}

GLOBALFUNC blnr OkCancelAlert(char *briefMsg, char *longMsg)
{
	return (IDOK == MessageBox(MainWnd, longMsg, briefMsg, MB_APPLMODAL|MB_OKCANCEL|MB_ICONWARNING|MB_DEFBUTTON2));
}

/*--- main window ---*/

LOCALVAR HINSTANCE AppInstance;
LOCALVAR HDC MainWndDC;

LOCALVAR si5b CmdShow;

LOCALVAR int WndX;
LOCALVAR int WndY;

LOCALVAR char *WndTitle = "Mini vMac";
LOCALVAR char *WndClassName = "minivmac";

LOCALFUNC blnr CreateMainWindow(void)
{
	int XBorder = GetSystemMetrics(SM_CXFIXEDFRAME);
	int YBorder = GetSystemMetrics(SM_CYFIXEDFRAME);
	int YCaption = GetSystemMetrics(SM_CYCAPTION);
	int YMenu = GetSystemMetrics(SM_CYMENU);
	int XSize = XBorder + vMacScreenWidth + XBorder;
	int YSize = YBorder + YCaption + YMenu + vMacScreenHeight + YBorder;
	int ScreenX = GetSystemMetrics(SM_CXSCREEN);
	int ScreenY = GetSystemMetrics(SM_CYSCREEN);

	WndX = (ScreenX - XSize) / 2;
	WndY = (ScreenY - YSize) / 2;

	if (WndX < 0) {
		WndX = 0;
	}
	if (WndY < 0) {
		WndY = 0;
	}

	MainWnd = CreateWindow(WndClassName, WndTitle,
		WS_VISIBLE|WS_SYSMENU|WS_MINIMIZEBOX,
		WndX, WndY, XSize, YSize, NULL,
		LoadMenu(AppInstance, MAKEINTRESOURCE(IDR_MAINMENU)),
		AppInstance, NULL);
	if (MainWnd == NULL) {
		MacMsg("CreateWindow failed", "Sorry, vMac encountered errors and cannot continue.", trueblnr);
	} else {
		ShowWindow(MainWnd, CmdShow);
		MainWndDC = GetDC(MainWnd);
		if (MainWndDC == NULL) {
			MacMsg("GetDC failed", "Sorry, vMac encountered errors and cannot continue.", trueblnr);
		} else {
			return trueblnr;
		}
	}
	return falseblnr;
}

LOCALFUNC blnr AllocateScreenCompare(void)
{
	screencomparebuff = (char *)GlobalAlloc(GMEM_FIXED, vMacScreenNumBytes);
	if (screencomparebuff == NULL) {
		MacMsg("Not enough memory", "There is not enough memory available to allocate the screencomparebuff.", trueblnr);
		return falseblnr;
	} else {
		return trueblnr;
	}
}

typedef struct BITMAPINFOHEADER256 {
	BITMAPINFOHEADER bmi;
	RGBQUAD colors[2];
} BITMAPINFOHEADER256;

LOCALPROC Screen_DrawAll(void)
{
	BITMAPINFOHEADER256 bmh;

	memset (&bmh, sizeof (bmh), 0);
	bmh.bmi.biSize = sizeof(BITMAPINFOHEADER);
	bmh.bmi.biWidth = vMacScreenWidth;
	bmh.bmi.biHeight = -vMacScreenHeight;
	bmh.bmi.biPlanes = 1;
	bmh.bmi.biBitCount = 1;
	bmh.bmi.biCompression= BI_RGB;
	bmh.bmi.biSizeImage = 0;
	bmh.bmi.biXPelsPerMeter = 0;
	bmh.bmi.biYPelsPerMeter = 0;
	bmh.bmi.biClrUsed = 0;
	bmh.bmi.biClrImportant = 0;
	bmh.colors[0].rgbRed = 255;
	bmh.colors[0].rgbGreen = 255;
	bmh.colors[0].rgbBlue = 255;
	bmh.colors[0].rgbReserved = 0;
	bmh.colors[1].rgbRed = 0;
	bmh.colors[1].rgbGreen = 0;
	bmh.colors[1].rgbBlue = 0;
	bmh.colors[1].rgbReserved = 0;
	if (SetDIBitsToDevice(
		MainWndDC, /* handle of device context */
		0, /* x-coordinate of upper-left corner of dest. rect. */
		0, /* y-coordinate of upper-left corner of dest. rect. */
		vMacScreenWidth, /* source rectangle width */
		vMacScreenHeight, /* source rectangle height */
		0, /* x-coordinate of lower-left corner of source rect. */
		0, /* y-coordinate of lower-left corner of source rect. */
		0, /* first scan line in array */
		vMacScreenHeight, /* number of scan lines */
		(CONST VOID *)screencomparebuff, /* address of array with DIB bits */
		(const struct tagBITMAPINFO *)&bmh, /* address of structure with bitmap info. */
		DIB_RGB_COLORS /* RGB or palette indices */
	) == 0) {
		/* ReportWinLastError(); */
	}
}

GLOBALPROC HaveChangedScreenBuff(si4b top, si4b left, si4b bottom, si4b right)
{
	BITMAPINFOHEADER256 bmh;
	ui3b *p = ((ui3b *)screencomparebuff) + top * (vMacScreenWidth >> 3);

	UnusedParam(left);
	UnusedParam(right);
#if 0
	{ /* testing code */
		if (PatBlt(MainWndDC,
			(int)left - 1,
			(int)top - 1,
			(int)right-left + 2,
			(int)bottom-top + 2, PATCOPY)) {
		}
	}
#endif
	memset (&bmh, sizeof (bmh), 0);
	bmh.bmi.biSize = sizeof(BITMAPINFOHEADER);
	bmh.bmi.biWidth = vMacScreenWidth;
	bmh.bmi.biHeight = -(bottom - top);
	bmh.bmi.biPlanes = 1;
	bmh.bmi.biBitCount = 1;
	bmh.bmi.biCompression= BI_RGB;
	bmh.bmi.biSizeImage = 0;
	bmh.bmi.biXPelsPerMeter = 0;
	bmh.bmi.biYPelsPerMeter = 0;
	bmh.bmi.biClrUsed = 0;
	bmh.bmi.biClrImportant = 0;
	bmh.colors[0].rgbRed = 255;
	bmh.colors[0].rgbGreen = 255;
	bmh.colors[0].rgbBlue = 255;
	bmh.colors[0].rgbReserved = 0;
	bmh.colors[1].rgbRed = 0;
	bmh.colors[1].rgbGreen = 0;
	bmh.colors[1].rgbBlue = 0;
	bmh.colors[1].rgbReserved = 0;
	if (SetDIBitsToDevice(
		MainWndDC, /* handle of device context */
		0, /* x-coordinate of upper-left corner of dest. rect. */
		top, /* y-coordinate of upper-left corner of dest. rect. */
		vMacScreenWidth, /* source rectangle width */
		(bottom - top), /* source rectangle height */
		0, /* x-coordinate of lower-left corner of source rect. */
		0, /* y-coordinate of lower-left corner of source rect. */
		0, /* first scan line in array */
		(bottom - top), /* number of scan lines */
		(CONST VOID *)p, /* address of array with DIB bits */
		(const struct tagBITMAPINFO *)&bmh, /* address of structure with bitmap info. */
		DIB_RGB_COLORS /* RGB or palette indices */
	) == 0) {
		/* ReportWinLastError(); */
	}
}

LOCALVAR blnr CurTrueMouseButton = falseblnr;

LOCALVAR blnr HaveCursorHidden = falseblnr;

LOCALFUNC blnr InitTheCursor(void)
{
	SetCursor(LoadCursor(NULL, IDC_ARROW));
	return trueblnr;
}

LOCALPROC ForceShowCursor(void)
{
	if (HaveCursorHidden) {
		HaveCursorHidden = falseblnr;
		(void) ShowCursor(TRUE);
		SetCursor(LoadCursor(NULL, IDC_ARROW));
	}
}

LOCALPROC CheckMouseState(void)
{
	blnr ShouldHaveCursorHidden;
	blnr NewTrueMouseButton;
	POINT NewMousePos;

	GetCursorPos(&NewMousePos);
	NewMousePos.x -= WndX;
	NewMousePos.y -= WndY;

	NewTrueMouseButton = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

	ShouldHaveCursorHidden = trueblnr;
	if (NewMousePos.x < 0) {
		NewMousePos.x = 0;
		ShouldHaveCursorHidden = falseblnr;
	} else if (NewMousePos.x > vMacScreenWidth) {
		NewMousePos.x = vMacScreenWidth - 1;
		ShouldHaveCursorHidden = falseblnr;
	}
	if (NewMousePos.y < 0) {
		NewMousePos.y = 0;
		ShouldHaveCursorHidden = falseblnr;
	} else if (NewMousePos.y > vMacScreenHeight) {
		NewMousePos.y = vMacScreenHeight - 1;
		ShouldHaveCursorHidden = falseblnr;
	}

	if (NewTrueMouseButton != CurTrueMouseButton) {
		CurTrueMouseButton = NewTrueMouseButton;
		CurMouseButton = CurTrueMouseButton && ShouldHaveCursorHidden;
	}

	/* if (ShouldHaveCursorHidden || CurMouseButton) */
	/* for a game like arkanoid, would like mouse to still
	move even when outside window in one direction */
	{
		CurMouseH = NewMousePos.x;
		CurMouseV = NewMousePos.y;
	}

	if (HaveCursorHidden != ShouldHaveCursorHidden) {
		HaveCursorHidden = ShouldHaveCursorHidden;
		if (HaveCursorHidden) {
			(void) ShowCursor(FALSE);
		} else {
			(void) ShowCursor(TRUE);
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		}
	}
}

#define NotAfileRef NULL

LOCALVAR HANDLE Drives[NumDrives]; /* open disk image files */

LOCALPROC InitDrives(void)
{
	si4b i;

	for (i = 0; i < NumDrives; ++i) {
		Drives[i] = NotAfileRef;
	}
}

GLOBALFUNC si4b vSonyRead(void *Buffer, ui4b Drive_No, ui5b Sony_Start, ui5b *Sony_Count)
{
	si4b result;
	HANDLE refnum;
	DWORD newL;
	DWORD BytesRead = 0;

	if (Drive_No < NumDrives) {
		refnum = Drives[Drive_No];
		if (refnum != NotAfileRef) {
			newL = SetFilePointer(
				refnum, /* handle of file */
				Sony_Start, /* number of bytes to move file pointer */
				nullpr, /* address of high-order word of distance to move */
				FILE_BEGIN /* how to move */
			);
			if (newL == 0xFFFFFFFF) {
				result = -1; /*& figure out what really to return &*/
			} else if (Sony_Start != (ui5b)newL) {
				/* not supposed to get here */
				result = -1; /*& figure out what really to return &*/
			} else {
				if (! ReadFile(refnum, /* handle of file to read */
					(LPVOID)Buffer, /* address of buffer that receives data */
					(DWORD)*Sony_Count, /* number of bytes to read */
					&BytesRead, /* address of number of bytes read */
					nullpr)) /* address of structure for data */
				{
					result = -1; /*& figure out what really to return &*/
				} else if ((ui5b)BytesRead != *Sony_Count) {
					result = -1; /*& figure out what really to return &*/
				} else {
					result = 0;
				}
			}
		} else {
			result = 0xFFBF; // Say it's offline (-65)
		}
	} else {
		result = 0xFFC8; // No Such Drive (-56)
	}
	*Sony_Count = BytesRead;
	return result;
}

GLOBALFUNC si4b vSonyWrite(void *Buffer, ui4b Drive_No, ui5b Sony_Start, ui5b *Sony_Count)
{
	si4b result;
	HANDLE refnum;
	DWORD newL;
	DWORD BytesWritten = 0;

	if (Drive_No < NumDrives) {
		refnum = Drives[Drive_No];
		if (refnum != NotAfileRef) {
			newL = SetFilePointer(
				refnum, /* handle of file */
				Sony_Start, /* number of bytes to move file pointer */
				nullpr, /* address of high-order word of distance to move */
				FILE_BEGIN /* how to move */
			);
			if (newL == 0xFFFFFFFF) {
				result = -1; /*& figure out what really to return &*/
			} else if (Sony_Start != (ui5b)newL) {
				/* not supposed to get here */
				result = -1; /*& figure out what really to return &*/
			} else {
				if (! WriteFile(refnum, /* handle of file to read */
					(LPVOID)Buffer, /* address of buffer that receives data */
					(DWORD)*Sony_Count, /* number of bytes to read */
					&BytesWritten, /* address of number of bytes read */
					nullpr)) /* address of structure for data */
				{
					result = -1; /*& figure out what really to return &*/
				} else if ((ui5b)BytesWritten != *Sony_Count) {
					result = -1; /*& figure out what really to return &*/
				} else {
					result = 0;
				}
			}
		} else {
			result = 0xFFBF; // Say it's offline (-65)
		}
	} else {
		result = 0xFFC8; // No Such Drive (-56)
	}
	*Sony_Count = BytesWritten;
	return result;
}

GLOBALFUNC blnr vSonyDiskLocked(ui4b Drive_No)
{
	UnusedParam(Drive_No);
	return falseblnr;
}

GLOBALFUNC si4b vSonyGetSize(ui4b Drive_No, ui5b *Sony_Count)
{
	si4b result;
	HANDLE refnum;
	DWORD L;

	if (Drive_No < NumDrives) {
		refnum = Drives[Drive_No];
		if (refnum != NotAfileRef) {
			L = GetFileSize (refnum, nullpr);
			if (L == 0xFFFFFFFF) {
				result = -1; /*& figure out what really to return &*/
			} else {
				*Sony_Count = L;
				result = 0;
			}
		} else {
			result = 0xFFBF; // Say it's offline (-65)
		}
	} else {
		result = 0xFFC8; // No Such Drive (-56)
	}
	return result;
}

GLOBALFUNC si4b vSonyEject(ui4b Drive_No)
{
	si4b result;
	HANDLE refnum;

	if (Drive_No < NumDrives) {
		refnum = Drives[Drive_No];
		if (refnum != NotAfileRef) {
			(void) FlushFileBuffers(refnum);
			(void) CloseHandle(refnum);
			result = 0;
			Drives[Drive_No] = NotAfileRef;
		}
		result = 0x0000;
	} else {
		result = 0xFFC8; // No Such Drive (-56)
	}
	return result;
}

GLOBALFUNC si4b vSonyVerify(ui4b Drive_No)
{
	si4b result;

	if (Drive_No < NumDrives) {
		if (Drives[Drive_No] != NotAfileRef) {
			result = 0x0000; // No Error (0)
		} else {
			result = 0xFFBF; // Say it's offline (-65)
		}
	} else {
		result = 0xFFC8; // No Such Drive (-56)
	}
	return result;
}

GLOBALFUNC si4b vSonyFormat(ui4b Drive_No)
{
	si4b result;

	if (Drive_No < NumDrives) {
		if (Drives[Drive_No] != NotAfileRef) {
			result = 0xFFD4; // Write Protected (-44)
		} else {
			result = 0xFFBF; // Say it's offline (-65)
		}
	} else {
		result = 0xFFC8; // No Such Drive (-56)
	}
	return result;
}

GLOBALFUNC blnr vSonyInserted (ui4b Drive_No)
{
	if (Drive_No >= NumDrives) {
		return falseblnr;
	} else {
		return (Drives[Drive_No] != NotAfileRef);
	}
}

LOCALFUNC blnr FirstFreeDisk(ui4b *Drive_No)
{
	si4b i;

	for (i = 0; i < NumDrives; ++i) {
		if (Drives[i] == NotAfileRef) {
			*Drive_No = i;
			return trueblnr;
		}
	}
	return falseblnr;
}

GLOBALFUNC blnr AnyDiskInserted(void)
{
	si4b i;

	for (i = 0; i < NumDrives; ++i) {
		if (Drives[i] != NotAfileRef) {
			return trueblnr;
		}
	}
	return falseblnr;
}

LOCALPROC Sony_Insert0(HANDLE refnum)
{
	ui4b Drive_No;

	if (! FirstFreeDisk(&Drive_No)) {
		(void) CloseHandle(refnum);
		MacMsg(kStrTooManyImagesTitle, kStrTooManyImagesMessage, falseblnr);
	} else {
		Drives[Drive_No] = refnum;
		MountPending |= ((ui5b)1 << Drive_No);
	}
}

LOCALFUNC blnr Sony_Insert1(char *drivepath)
{
	HANDLE refnum = CreateFile(
		drivepath, /* pointer to name of the file */
		GENERIC_READ + GENERIC_WRITE, /* access (read-write) mode */
		0, /* share mode */
		nullpr, /* pointer to security descriptor */
		OPEN_EXISTING, /* how to create */
		FILE_ATTRIBUTE_NORMAL, /* file attributes */
		nullpr /* handle to file with attributes to copy */
	);
	if (refnum != INVALID_HANDLE_VALUE) {
		Sony_Insert0(refnum);
		return trueblnr;
	}
	return falseblnr;
}

LOCALPROC InsertADisk(void)
{
	OPENFILENAME ofn;
	char szDirName[256];
	char szFile[256], szFileTitle[256];
	UINT  i, cbString;
	char  chReplace;
	char  szFilter[256];

	szDirName[0] = '\0';
	szFile[0] = '\0';
	strcpy(szFilter,"Disk images|*.DSK;*.HF?;*.IMG;*.IMA;*.IMAGE|All files (*.*)|*.*|\0");

	cbString = strlen(szFilter);

	chReplace = szFilter[cbString - 1];

	for (i = 0; szFilter[i] != '\0'; ++i)
	{
		if (szFilter[i] == chReplace)
			szFilter[i] = '\0';
	}

	memset(&ofn, 0, sizeof(OPENFILENAME));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = MainWnd;
	ofn.lpstrFilter = szFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile= szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = sizeof(szFileTitle);
	ofn.lpstrInitialDir = szDirName;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

	if(!GetOpenFileName(&ofn)) {
		/* report error */
	} else {
		(void) Sony_Insert1(ofn.lpstrFile);
	}
}

LOCALFUNC blnr GetAppDir (char* pathName)
/* be sure at least _MAX_PATH long! */
{
	if (GetModuleFileName (AppInstance, pathName, _MAX_PATH) == 0) {
		MacMsg("error", "GetModuleFileName failed", falseblnr);
	} else {
		char *p0 = pathName;
		char *p = nullpr;
		char c;

		while ((c = *p0++) != 0) {
			if (c == '\\') {
				p = p0;
			}
		}
		if (p == nullpr) {
			MacMsg("error", "strrchr failed", falseblnr);
		} else {
			*p = '\0';
			return trueblnr;
		}
	}
	return falseblnr;
}

LOCALFUNC blnr LoadInitialImageFromName(char* ImageName)
{
	char ImageFile[_MAX_PATH];

	if (GetAppDir(ImageFile)) {
		strcat(ImageFile, ImageName);
		if (Sony_Insert1(ImageFile)) {
			return trueblnr;
		}
	}
	return falseblnr;
}

LOCALFUNC blnr LoadInitialImages(void)
{
	if (LoadInitialImageFromName("disk1.dsk"))
	if (LoadInitialImageFromName("disk2.dsk"))
	if (LoadInitialImageFromName("disk3.dsK"))
	{
	}
	return trueblnr;
}

LOCALFUNC blnr MyReadDat(HANDLE refnum, ui5b L, void *p)
{
	DWORD BytesRead;

	if (! ReadFile(refnum, /* handle of file to read */
		(LPVOID)p, /* address of buffer that receives data */
		(DWORD)L, /* number of bytes to read */
		&BytesRead, /* address of number of bytes read */
		nullpr)) /* address of structure for data */
	{
		/*& should report the error *\&*/
		return falseblnr;
	} else if ((ui5b)BytesRead != L) {
		/*& should report the error *\&*/
		return falseblnr;
	} else {
		return trueblnr;
	}
}

LOCALFUNC blnr AllocateMacROM(void)
{
	ROM = (ui4b *)GlobalAlloc(GMEM_FIXED, kROM_Size);
	if (ROM == NULL) {
		MacMsg("Not enough memory", "There is not enough memory available to allocate the ROM.", trueblnr);
		return falseblnr;
	} else {
		return trueblnr;
	}
}

LOCALFUNC blnr LoadMacRom(void)
{
	char ROMFile[_MAX_PATH];
	HANDLE refnum;
	blnr IsOk = falseblnr;

	if (! GetAppDir(ROMFile)) {
		MacMsg("vMac error", "Sorry, vMac encountered errors and cannot continue.", trueblnr);
	} else {
		strcat(ROMFile, "vMac.ROM");

		refnum = CreateFile(
			ROMFile, /* pointer to name of the file */
			GENERIC_READ, /* access (read-write) mode */
			FILE_SHARE_READ, /* share mode */
			nullpr, /* pointer to security descriptor */
			OPEN_EXISTING, /* how to create */
			FILE_ATTRIBUTE_NORMAL, /* file attributes */
			nullpr /* handle to file with attributes to copy */
		);
		if (refnum == INVALID_HANDLE_VALUE) {
			MacMsg("ROM not found", "The file \"vMac.ROM\" could not be found. Please read the documentation.", trueblnr);
		} else {
			if (! MyReadDat(refnum, kROM_Size, ROM)) {
				MacMsg("vMac error", "Couldn't read the ROM image file.", trueblnr);
			} else {
				IsOk = trueblnr;
			}
			(void) CloseHandle(refnum);
		}
	}
	return IsOk;
}

LOCALFUNC blnr AllocateMacRAM(void)
{
	kRAM_Size = 0x00400000; // Try 4 MB
	RAM = (ui4b *)GlobalAlloc(GMEM_FIXED, kRAM_Size + RAMSafetyMarginFudge);
	if (RAM == NULL) {
		kRAM_Size = 0x00200000; // Try 2 MB
		RAM = (ui4b *)GlobalAlloc(GMEM_FIXED, kRAM_Size + RAMSafetyMarginFudge);
		if (RAM == NULL) {
			kRAM_Size = 0x00100000; // Try 1 MB
			RAM = (ui4b *)GlobalAlloc(GMEM_FIXED, kRAM_Size + RAMSafetyMarginFudge);
			if (RAM == NULL) {
				MacMsg("Not enough memory", "There is not enough memory available to allocate 1Mb of RAM for the emulated Mac.", trueblnr);
				return falseblnr;
			}
		}
	}

	return trueblnr;
}

#include "DATE2SEC.h"

GLOBALFUNC ui5b GetMacDateInSecond(void)
{
	SYSTEMTIME s;

	GetLocalTime(&s);
	return Date2MacSeconds(s.wSecond, s.wMinute, s.wHour,
		s.wDay, s.wMonth, s.wYear);
}

#if InstallFileIcons
LOCALPROC MySetRegKey(HKEY hKeyRoot, char *strRegKey, char *strRegValue)
{
	HKEY RegKey;
	DWORD dwDisposition;

	if (RegCreateKeyEx(hKeyRoot, strRegKey, 0, NULL,
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
		NULL, &RegKey, &dwDisposition) == ERROR_SUCCESS)
	{
		RegSetValueEx(RegKey, NULL, 0, REG_SZ, (CONST si3b *)strRegValue, strlen(strRegValue) + sizeof(char));
		RegCloseKey(RegKey);
	}
}
#endif

#if InstallFileIcons
LOCALPROC RegisterShellFileType (char *AppPath, char *strFilterExt,
	char *strFileTypeId, char *strFileTypeName,
	char *strIconId, blnr CanOpen)
{
	char strRegKey[_MAX_PATH];
	char strRegValue[_MAX_PATH + 2]; /* extra room for ","{strIconId} */

	MySetRegKey(HKEY_CLASSES_ROOT, strFileTypeId, strFileTypeName);
	MySetRegKey(HKEY_CLASSES_ROOT, strFilterExt, strFileTypeId);

	strcpy(strRegKey, strFileTypeId);
	strcat(strRegKey, "\\DefaultIcon");
	strcpy(strRegValue, AppPath);
	strcat(strRegValue, ",");
	strcat(strRegValue, strIconId);
	MySetRegKey(HKEY_CLASSES_ROOT, strRegKey, strRegValue);

	if (CanOpen) {
		strcpy(strRegKey, strFileTypeId);
		strcat(strRegKey, "\\shell\\open\\command");
		strcpy(strRegValue, AppPath);
		strcat(strRegValue, " \"%1\"");
		MySetRegKey(HKEY_CLASSES_ROOT, strRegKey, strRegValue);
	}
}
#endif

#if InstallFileIcons
LOCALFUNC blnr RegisterInRegistry(void)
{
	char AppPath[_MAX_PATH];

	GetModuleFileName(NULL, AppPath, _MAX_PATH);
	GetShortPathName(AppPath, AppPath, _MAX_PATH);

	RegisterShellFileType(AppPath, ".DSK", "minivmac.DSK", "Mini vMac Disk Image", "1", trueblnr);
	RegisterShellFileType(AppPath, ".ROM", "minivmac.ROM", "Mini vMac ROM Image", "2", falseblnr);

	return trueblnr;
}
#endif

LOCALVAR char *CommandLine;

LOCALFUNC blnr ScanCommandLine(void)
{
	char fileName[_MAX_PATH];
	char *filePtr;
	char *p = CommandLine;

	while (*p != 0) {
		if (*p == ' ') {
			p++;
		} else {
			filePtr = fileName;
			if (*p == '\"') {
				p++;
				while (*p != '\"' && *p != 0) {
					*filePtr++ = *p++;
				}
				if (*p == '\"') {
					p++;
				}
			} else {
				while (*p != ' ' && *p != 0) {
					*filePtr++ = *p++;
				}
			}
			*filePtr = (char)0;
			(void) Sony_Insert1(fileName);
		}
	}

	return trueblnr;
}

static BOOL APIENTRY MyWinDlgProc(
	HWND hDlg,
	si4b message,
	si4b wParam,
	si5b lParam)
{
	if (message == WM_COMMAND) {
		if (LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, TRUE);
			return TRUE;
		} else {
			return FALSE;
		}
	} else if (message == WM_INITDIALOG) {
		char s[_MAX_PATH];

		strcpy(s, kAppVariationStr);
		strcat(s, ", Copyright ");
		strcat(s, kStrCopyrightYear);
		strcat(s, ".");
		SetDlgItemText(hDlg, ID_ABOUT_VERSION, s);
		SetDlgItemText(hDlg, ID_ABOUT_AUTHORS, "Including or based upon code by Bernd Schmidt, Philip Cummins, Richard F. Bannister, Weston Pawlowski, Michael Hanni, Paul Pratt, and others.");
		SetDlgItemText(hDlg, ID_ABOUT_LICENSE, "Mini vMac is distributed under the terms of the GNU Public License, version 2.");
		SetDlgItemText(hDlg, ID_ABOUT_WARRNTY, "Mini vMac is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.");
		SetDlgItemText(hDlg, ID_ABOUT_FORINFO, "For more information, see:");
		SetDlgItemText(hDlg, ID_ABOUT_WEBPAGE, kStrHomePage);
		return TRUE;
	} else {
		return FALSE;
	}
	UNREFERENCED_PARAMETER(lParam);
}

LOCALPROC ShowAboutMessage(void)
{
	(void) DialogBox(AppInstance,
		MAKEINTRESOURCE(IDD_MYABOUTDIALOG),
		MainWnd,
		(DLGPROC)MyWinDlgProc);
}

LOCALVAR blnr SpeedLimit = falseblnr;

LOCALVAR blnr gBackgroundFlag = falseblnr;

LOCALVAR blnr CapsLockState = falseblnr;

#include "POSTKEYS.h"

/*
	Which key configuration do you want to use?
	1: Left control is command, right control is option.
	2: Alt is command, and control is option.
	3: (default) Left alt is command, right alt is option, and
		control is control.
	4: control is command, and Alt is option.
*/
#define KeyConfig 4

/* these constants weren't in the header files I have */
#define myVK_Subtract 0xBD
#define myVK_Equal 0xBB
#define myVK_BackSlash 0xDC
#define myVK_Comma 0xBC
#define myVK_Period 0xBE
#define myVK_Slash 0xBF
#define myVK_SemiColon 0xBA
#define myVK_SingleQuote 0xDE
#define myVK_LeftBracket 0xDB
#define myVK_RightBracket 0xDD
#define myVK_Grave 0xC0

LOCALPROC DoVirtualKey(WPARAM wparam, LPARAM lparam, blnr down)
{
	int v;

	switch(wparam) {
		case VK_CONTROL:
#if KeyConfig == 1
			if(TestBit(lparam, 24)) {
				//Right Control
				v = MKC_Option;
			} else {
				//Left Control
				v = MKC_Command;
			}
#endif
#if KeyConfig == 2
			v = MKC_Option;
#endif
#if KeyConfig == 3
			v = MKC_Control;
#endif
#if KeyConfig == 4
			v = MKC_Command;
#endif

			break;

		case VK_MENU:
#if KeyConfig == 1
			return;
#endif
#if KeyConfig == 2
			v = MKC_Command;
#endif
#if KeyConfig == 3
			if(TestBit(lparam, 24)) {
				//Right Alt
				v = MKC_Option;
			} else {
				//Left Alt
				v = MKC_Command;
			}
#endif
#if KeyConfig == 4
			v = MKC_Option;
#endif
			break;

		case VK_CAPITAL:
			v = MKC_CapsLock;
			if (! down) {
				return;
			} else {
				CapsLockState = ! CapsLockState;
				down = CapsLockState;
			}
			break;

		case VK_SHIFT: v = MKC_Shift; break;

		case 'A': v = MKC_A; break;
		case 'B': v = MKC_B; break;
		case 'C': v = MKC_C; break;
		case 'D': v = MKC_D; break;
		case 'E': v = MKC_E; break;
		case 'F': v = MKC_F; break;
		case 'G': v = MKC_G; break;
		case 'H': v = MKC_H; break;
		case 'I': v = MKC_I; break;
		case 'J': v = MKC_J; break;
		case 'K': v = MKC_K; break;
		case 'L': v = MKC_L; break;
		case 'M': v = MKC_M; break;
		case 'N': v = MKC_N; break;
		case 'O': v = MKC_O; break;
		case 'P': v = MKC_P; break;
		case 'Q': v = MKC_Q; break;
		case 'R': v = MKC_R; break;
		case 'S': v = MKC_S; break;
		case 'T': v = MKC_T; break;
		case 'U': v = MKC_U; break;
		case 'V': v = MKC_V; break;
		case 'W': v = MKC_W; break;
		case 'X': v = MKC_X; break;
		case 'Y': v = MKC_Y; break;
		case 'Z': v = MKC_Z; break;

		case '0': v = MKC_0; break;
		case '1': v = MKC_1; break;
		case '2': v = MKC_2; break;
		case '3': v = MKC_3; break;
		case '4': v = MKC_4; break;
		case '5': v = MKC_5; break;
		case '6': v = MKC_6; break;
		case '7': v = MKC_7; break;
		case '8': v = MKC_8; break;
		case '9': v = MKC_9; break;

		case VK_NUMPAD1: v = MKC_KP1; break;
		case VK_NUMPAD2: v = MKC_KP2; break;
		case VK_NUMPAD3: v = MKC_KP3; break;
		case VK_NUMPAD4: v = MKC_KP4; break;
		case VK_NUMPAD5: v = MKC_KP5; break;
		case VK_NUMPAD6: v = MKC_KP6; break;
		case VK_NUMPAD7: v = MKC_KP7; break;
		case VK_NUMPAD8: v = MKC_KP8; break;
		case VK_NUMPAD9: v = MKC_KP9; break;
		case VK_NUMPAD0: v = MKC_KP0; break;

		case VK_DECIMAL: v = MKC_Decimal; break;
		case VK_DELETE: v = MKC_Decimal; break;

		case VK_RETURN:
			if (TestBit(lparam, 24)) {
				//Right Alt
				v = MKC_Enter;
			} else {
				//Left Alt
				v = MKC_Return;
			}
			break;

		case VK_SPACE: v = MKC_Space; break;
		case VK_BACK: v = MKC_BackSpace; break;
		case VK_TAB: v = MKC_Tab; break;
		case VK_LEFT: v = MKC_Left; break;
		case VK_UP: v = MKC_Up; break;
		case VK_RIGHT: v = MKC_Right; break;
		case VK_DOWN: v = MKC_Down; break;

		case VK_DIVIDE: v = MKC_KPDevide; break;
		case VK_MULTIPLY: v = MKC_KPMultiply; break;
		case VK_SUBTRACT: v = MKC_KPSubtract; break;
		case VK_ADD: v = MKC_KPAdd; break;
		case VK_NUMLOCK: v = MKC_Clear; break;
		case VK_SEPARATOR: v = MKC_KPEqual; break;

		case VK_ESCAPE: v = MKC_Escape; break;

		case myVK_Subtract: v = MKC_Minus; break;
		case myVK_Equal: v = MKC_Equal; break;
		case myVK_BackSlash: v = MKC_BackSlash; break;
		case myVK_Comma: v = MKC_Comma; break;
		case myVK_Period: v = MKC_Period; break;
		case myVK_Slash: v = MKC_Slash; break;
		case myVK_SemiColon: v = MKC_SemiColon; break;
		case myVK_SingleQuote: v = MKC_SingleQuote; break;
		case myVK_LeftBracket: v = MKC_LeftBracket; break;
		case myVK_RightBracket: v = MKC_RightBracket; break;
		case myVK_Grave: v = MKC_Grave; break;

		default:
			return; /* not a key on the MacPlus */
			break;

	}
	Keyboard_UpdateKeyMap(v, down);
}

LRESULT CALLBACK Win32WMProc(HWND MainWnd, UINT uMessage, WPARAM wparam, LPARAM lparam);

LRESULT CALLBACK Win32WMProc(HWND MainWnd, UINT uMessage, WPARAM wparam, LPARAM lparam)
{
	switch (uMessage)
	{
		case WM_MOUSEMOVE:
			/* windows may have messed up cursor */
			/*
				there is no notification when the mouse moves
				outside the window, and the cursor is automatically
				changed
			*/
			if (! HaveCursorHidden) {
				/* SetCursor(LoadCursor(NULL, IDC_ARROW)); */
			}
			break;
		case WM_PAINT:
			{
				PAINTSTRUCT ps;

				BeginPaint(MainWnd, (LPPAINTSTRUCT)&ps);
				Screen_DrawAll();
				EndPaint(MainWnd, (LPPAINTSTRUCT)&ps);
			}
			break;

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			DoVirtualKey(wparam, lparam, trueblnr);
			break;

		case WM_SYSKEYUP:
		case WM_KEYUP:
			DoVirtualKey(wparam, lparam, falseblnr);
			break;
		case WM_CLOSE:
			RequestMacOff = trueblnr;
			break;
		case WM_ACTIVATE:
			{
				blnr NewBackgroundFlag = (LOWORD(wparam) == WA_INACTIVE);
				if (NewBackgroundFlag != gBackgroundFlag) {
					gBackgroundFlag = NewBackgroundFlag;
					if (gBackgroundFlag) {
						ForceShowCursor();
					}
				}
			}
			break;
		case WM_COMMAND:
			switch(LOWORD(wparam))
			{
				case ID_FILE_INSERTDISK1:
					InsertADisk();
					break;
				case ID_FILE_QUIT:
					RequestMacOff = trueblnr;
					break;
				case ID_CONTROL_LIMITSPEED:
					SpeedLimit = ! SpeedLimit;
					break;
				case ID_CONTROL_RESET:
					RequestMacReset = trueblnr;
					break;
				case ID_CONTROL_INTERRUPT:
					RequestMacInterrupt = trueblnr;
					break;
				case ID_HELP_ABOUT:
					ShowAboutMessage();
					break;
			}
			break;
		case WM_INITMENU:
			{
				HMENU CurMenuHdl = GetMenu(MainWnd);
				(void) CheckMenuItem(CurMenuHdl,
					ID_CONTROL_LIMITSPEED,
					MF_BYCOMMAND + (SpeedLimit ? MF_CHECKED : MF_UNCHECKED));
			}
			break;
		case WM_MOVE:
			WndX = (si5b) LOWORD(lparam);
			WndY = (si5b) HIWORD(lparam);
			break;
		case WM_SYSCHAR:
		case WM_CHAR:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
			/* prevent any further processing */
			break;
		default:
			return DefWindowProc(MainWnd, uMessage, wparam, lparam);
	}
	return 0;
}

LOCALFUNC blnr RegisterOurClass(void)
{
	WNDCLASS wc;

	wc.style         = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
	wc.lpfnWndProc   = (WNDPROC)Win32WMProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = AppInstance;
	wc.hIcon         = LoadIcon(AppInstance, MAKEINTRESOURCE(IDI_VMAC));
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (SS_WHITERECT);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = WndClassName;

	if (! RegisterClass((LPWNDCLASS)&wc.style)) {
		MacMsg("RegisterClass failed", "Sorry, vMac encountered errors and cannot continue.", trueblnr);
		return falseblnr;
	} else {
		return trueblnr;
	}
}

LOCALPROC DoOnEachSixtieth(void)
{
	MSG msg;
	blnr GotMessage;

	CheckMouseState();
	do {

		if (gBackgroundFlag) {
			GotMessage = (GetMessage(&msg, NULL, 0, 0) != -1);
		} else {
			GotMessage = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
		}
		if (GotMessage) {
			DispatchMessage(&msg);
		}
	} while (gBackgroundFlag);
		/*
			When in background, halt the emulator by
			continuously running the event loop
		*/
}

LOCALVAR DWORD LastTime;
LOCALVAR ui5b TimeCounter = 0;

LOCALFUNC blnr Init60thCheck(void)
{
	LastTime = GetTickCount();
	return trueblnr;
}

GLOBALFUNC blnr CheckIntSixtieth(blnr overdue)
{
	DWORD LatestTime;

	do {
		LatestTime = GetTickCount();
		if (LatestTime != LastTime) {
			TimeCounter += 60 * (LatestTime - LastTime);
			LastTime = LatestTime;
			if (TimeCounter > 1000) {
				TimeCounter %= 1000;
				/*
					Idea is to get here every
					1000/60 milliseconds, on average.
					Unless emulation has been interupted
					too long, which is why use '%='
					and not '-='.

				*/
				DoOnEachSixtieth();
				return trueblnr;
			}
		}
	} while (SpeedLimit && overdue);
	return falseblnr;
}

LOCALPROC ZapOSGLUVars(void)
{
	ROM = NULL;
	RAM = NULL;
	screencomparebuff = NULL;
	MainWnd = NULL;
	MainWndDC = NULL;
	InitDrives();
}

LOCALFUNC blnr InitOSGLU(void)
{
	if (RegisterOurClass())
	if (LoadInitialImages())
	if (ScanCommandLine())
#if InstallFileIcons
	if (RegisterInRegistry())
#endif
	if (AllocateMacROM())
	if (LoadMacRom())
	if (AllocateScreenCompare())
	if (CreateMainWindow())
	if (AllocateMacRAM()) /* do near end, so can have idea of how much space is left */
	if (InitTheCursor())
	if (Init60thCheck())
	{
		return trueblnr;
	}
	return falseblnr;
}

LOCALPROC UnInitOSGLU(void)
{
	ForceShowCursor();

	if (RAM != NULL) {
		if (GlobalFree(RAM) != NULL) {
			MacMsg("error", "GlobalFree failed", falseblnr);
		}
	}

	if (MainWndDC != NULL) {
		ReleaseDC(MainWnd, MainWndDC);
	}
	if (MainWnd != NULL) {
		DestroyWindow(MainWnd);
	}
	if (screencomparebuff != NULL) {
		if (GlobalFree(screencomparebuff) != NULL) {
			MacMsg("error", "GlobalFree failed", falseblnr);
		}
	}
	if (ROM != NULL) {
		if (GlobalFree(ROM) != NULL) {
			MacMsg("error", "GlobalFree failed", falseblnr);
		}
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	UnusedParam(hPrevInstance);
	AppInstance = hInstance;
	CmdShow = nCmdShow;
	CommandLine = lpCmdLine;

	ZapOSGLUVars();
	if (InitOSGLU()) {
		ProgramMain();
	}
	UnInitOSGLU();

	return(0);
}
