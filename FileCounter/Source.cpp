#include <Windows.h>
#include <stdio.h>
#include <iostream>

HANDLE gheap;
UINT gBegPathDepth = 0;
UINT gLines = 0;

typedef void(*FILE_CALLBACK)(LPCWSTR);

UINT countFileLines(LPCWSTR path){
	HANDLE hfile = CreateFile(path, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
	if (hfile == INVALID_HANDLE_VALUE)
		return 0;
	SIZE_T fileSize = (SIZE_T)GetFileSize(hfile, 0);
	UINT count = 0;
	BYTE* buf = (BYTE*)HeapAlloc(gheap, 0, fileSize);
	if (buf != 0){
		ReadFile(hfile, buf, fileSize, 0, 0);
		count = 1;
		for (UINT i = 0; i < fileSize; ++i) if (buf[i] == '\n') ++count;
		HeapFree(GetProcessHeap(), 0, buf);
	}
	CloseHandle(hfile);
	return count;
}

VOID findAllFilesFromDirection(LPCWSTR path, FILE_CALLBACK dir_callback, FILE_CALLBACK file_callback){
	WCHAR pathBuf[MAX_PATH];
	SIZE_T pathLen = lstrlen(path) - 1;
	memcpy(pathBuf, path, pathLen * 2);
	WIN32_FIND_DATA fnddata;
	ZeroMemory(&fnddata, sizeof(fnddata));
	HANDLE hfnd = FindFirstFile(path, &fnddata);
	if (hfnd != INVALID_HANDLE_VALUE){
		do{
			if (!(fnddata.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN)) && fnddata.cFileName[0] != '.'){
				memcpy(&pathBuf[pathLen], fnddata.cFileName, (lstrlen(fnddata.cFileName) + 1) * 2);
				if (fnddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
					if (dir_callback) dir_callback(pathBuf);
					lstrcat(pathBuf, L"/*");
					findAllFilesFromDirection(pathBuf, dir_callback, file_callback);
				}
				else if (file_callback) file_callback(pathBuf);
			}
		} while (FindNextFile(hfnd, &fnddata));
	}
}

UINT pathDepth(LPCWSTR path){
	UINT i = 0;
	for (; path[i]; path[i] == wchar_t('/') ? ++i : (UINT)++path);
	return i;
}

void dirCallback(LPCWSTR a){
	LPCWSTR name = wcsrchr(a, '/') + 1;;
	UINT depth = pathDepth(a);
	for (UINT i = depth; i > gBegPathDepth; --i) putchar(' ');
	wprintf(L"[%s]\n", name);
}

void fileCallback(LPCWSTR a){
	LPCWSTR name = wcsrchr(a, '/') + 1;;
	UINT lines = countFileLines(a);
	gLines += lines;
	UINT depth = pathDepth(a);
	for (UINT i = depth; i > gBegPathDepth; --i) putchar(' ');
	wprintf(L"%s [%d]\n", name, lines);
}

int main(){
	gheap = GetProcessHeap();
	wprintf(L"FILE NAME [LINES]\n-----------------\n");
	gBegPathDepth = pathDepth(L"C:/*");
	findAllFilesFromDirection(L"C:/*", dirCallback, fileCallback);
	wprintf(L"-----------------\nTOTAL: %d\n", gLines);
	/*UINT64 lines = 0;
	WIN32_FIND_DATA fnddata = { 0 };
	WCHAR pathBuf[MAX_PATH] = L"C:/testloc/*";
	SIZE_T pathLen = lstrlen(pathBuf);
	HANDLE hfnd = FindFirstFile(pathBuf, &fnddata);
	if (hfnd != INVALID_HANDLE_VALUE){
	wprintf(L"FILE NAME\t\t\tLINES\n------------------------------------------\n");
	do{
	if (!(fnddata.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN))){
	memcpy(&pathBuf[pathLen - 1], fnddata.cFileName, (lstrlen(fnddata.cFileName) + 1) * 2);
	UINT64 a = countFileLines(pathBuf);
	lines += a;
	wprintf(L"%-32s%d\n", fnddata.cFileName, a);
	}
	} while (FindNextFile(hfnd, &fnddata));
	wprintf(L"------------------------------------------\nTOTAL:\t\t\t\t%d\n\n", lines);
	}*/
}