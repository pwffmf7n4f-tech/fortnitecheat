// loader.c
#include <windows.h>
#include <stdio.h>
#define IOCTL_SET_PID CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, 0)
#define IOCTL_SET_FEATURES CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, 0)
#define IOCTL_GET_FEATURES CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, 0)
typedef struct { ULONG pid; HANDLE h; BOOLEAN wh, aim; } GD;

int main() {
	SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!scm) { puts("Need admin"); return 1; }
	SC_HANDLE svc = CreateServiceA(scm,"FortWall","FortWall",
		SERVICE_ALL_ACCESS,SERVICE_KERNEL_DRIVER,
		SERVICE_DEMAND_START,SERVICE_ERROR_NORMAL,
		"D:\\build\\Release\\wallhack.sys",NULL,NULL,NULL,NULL,NULL);
	if (!svc && GetLastError()==ERROR_SERVICE_EXISTS)
		svc = OpenServiceA(scm,"FortWall",SERVICE_ALL_ACCESS);
	StartService(svc,0,0);
	HANDLE h = CreateFileA("\\\\\\\\.\\\\FortWall",GENERIC_READ|GENERIC_WRITE,
		0,NULL,OPEN_EXISTING,0,NULL);
	if (h==INVALID_HANDLE_VALUE) { puts("open dev"); return 2;}
	DWORD pid;
	printf("PID fortnite: "); scanf("%lu",&pid);
	DeviceIoControl(h,IOCTL_SET_PID,&pid,sizeof(pid),NULL,0,&(BytesReturned),NULL);
	puts("Driver ready – launch overlay.exe for menu");
	CloseHandle(h); CloseServiceHandle(svc); CloseServiceHandle(scm);
	return 0;
}
