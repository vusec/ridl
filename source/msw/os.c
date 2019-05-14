#include <stdlib.h>
#include <string.h>

#include <asprintf.h>

#define WIN32_MEAN_AND_LEAN
#include <windows.h>

#include <info/os.h>

char *
get_os_name(void)
{
	TCHAR data[1024];
	HKEY key;
	DWORD size = sizeof data;
	DWORD status;
	char *name;
	int len;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0,
		KEY_QUERY_VALUE, &key) != ERROR_SUCCESS)
		return strdup("Microsoft Windows");

	status = RegQueryValueEx(key, L"ProductName", 0, 0, (LPBYTE)data, &size);
	RegCloseKey(key);

	if (status != ERROR_SUCCESS)
		return strdup("Microsoft Windows");

	len = WideCharToMultiByte(CP_UTF8, 0, data, size, NULL, 0, NULL, NULL);
	name = (char *)malloc(len);
	WideCharToMultiByte(CP_UTF8, 0, data, size, name, len, NULL, NULL);

	return name;
}
