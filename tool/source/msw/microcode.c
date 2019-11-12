#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIN32_MEAN_AND_LEAN
#include <windows.h>

char *
get_microcode(void)
{
	HKEY key;
	BYTE *data;
	char *hex;
	DWORD size = 0;
	DWORD i;
	DWORD status;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0,
		KEY_QUERY_VALUE, &key) != ERROR_SUCCESS)
		return strdup("Unknown");

	status = RegQueryValueEx(key, L"Update Revision", 0, 0, NULL, &size);
	
	if (status != ERROR_SUCCESS) {
		RegCloseKey(key);
		return strdup("Unknown");
	}

	data = calloc(size, sizeof *data);

	if (!data) {
		RegCloseKey(key);
		return strdup("Unknown");
	}
	
	status = RegQueryValueEx(key, L"Update Revision", 0, 0, (LPBYTE)data, &size);

	if (status != ERROR_SUCCESS) {
		RegCloseKey(key);
		return strdup("Unknown");
	}

	RegCloseKey(key);

	hex = calloc(size + 2, sizeof *hex);

	if (!hex) {
		free(data);
		return strdup("Unknown");
	}

	hex[0] = '0';
	hex[1] = 'x';

	for (i = 0; i < size; ++i) {
		if (data[size - i - 1])
			break;
	}

	size -= i;

	for (i = 0; i < size; ++i) {
		char digits[] = "0123456789abcdef";

		hex[i * 2 + 2] = digits[(data[i] & 0xf0) >> 4];
		hex[i * 2 + 3] = digits[data[i] & 0xf];
	}

	hex[2 * size + 2] = '\0';

	return hex;
}
