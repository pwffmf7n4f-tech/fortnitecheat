// wallhack.c
#include <ntddk.h>

#define DEVICE_TAG '4htW'

typedef struct _GAME_DATA {
	ULONG pid;
	PEPROCESS proc;
	BOOLEAN wh_active;
	BOOLEAN aim_active;
} GAME_DATA;

static GAME _target = { 0 };

NTSTATUS on_irp_complete(PDEVICE_OBJECT, PIRP Irp, PVOID buf, PIO_STATUS_BLOCK io) {
	if (io->Information != sizeof(GAME_DATA)) io->Information = 0;
	return STATUS_SUCCESS;
}

NTSTATUS ctl_ioctl(PDEVICE_OBJECT, PIRP Irp) {
	PIO_STACK required = (PIO_STACK)IoCurrentIrpStackLocationIrp;
	switch (required->Parameters.DeviceIoControl.IoControlCode) {
	case IOCTL_SET_PID: {
		ULONG pid = *(ULONG*)Irp->AssociatedIrp.SystemBuffer;
		if (NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)pid, &_target.proc))) {
			_target.pid = pid;
			Irp->IoStatus.Information = sizeof(ULONG);
		}
		break;
	}
	case IOCTL_SET_FEATURES: {
		GAME_DATA *in = (GAME_DATA*)Irp->AssociatedIrp.SystemBuffer;
		_target.wh_active = in->wh_active;
		_target.aim_active = in->aim_active;
		Irp->IoStatus.Information = sizeof(GAME_DATA);
		break;
	}
	case IOCTL_GET_FEATURES: {
		if (_target.proc) {
			GAME_DATA *out = (GAME_DATA*)Irp->AssociatedIrp.SystemBuffer;
			*out = _target;
			Irp->IoStatus.Information = sizeof(GAME_DATA);
		}
		break;
	}
	default: return STATUS_INVALID_DEVICE_REQUEST;
	}
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

void DriverUnload(PDRIVER_OBJECT) { DbgPrint("[WH] bye\n"); }

NTSTATUS DriverEntry(PDRIVER_OBJECT drv, PUNICODE_STRING) {
	drv->MajorUnload = DriverUnload;
	for (int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) drv->MajorFunction[i] = ctl_ioctl;
	return STATUS_SUCCESS;
}
// wallhack.c
#include <ntddk.h>

#define DEVICE_TAG '4htW'

typedef struct _GAME_DATA {
	ULONG pid;
	PEPROCESS proc;
	BOOLEAN wh_active;
	BOOLEAN aim_active;
} GAME_DATA;

static GAME _target = { 0 };

NTSTATUS on_irp_complete(PDEVICE_OBJECT, PIRP Irp, PVOID buf, PIO_STATUS_BLOCK io) {
	if (io->Information != sizeof(GAME_DATA)) io->Information = 0;
	return STATUS_SUCCESS;
}

NTSTATUS ctl_ioctl(PDEVICE_OBJECT, PIRP Irp) {
	PIO_STACK required = (PIO_STACK)IoCurrentIrpStackLocationIrp;
	switch (required->Parameters.DeviceIoControl.IoControlCode) {
	case IOCTL_SET_PID: {
		ULONG pid = *(ULONG*)Irp->AssociatedIrp.SystemBuffer;
		if (NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)pid, &_target.proc))) {
			_target.pid = pid;
			Irp->IoStatus.Information = sizeof(ULONG);
		}
		break;
	}
	case IOCTL_SET_FEATURES: {
		GAME_DATA *in = (GAME_DATA*)Irp->AssociatedIrp.SystemBuffer;
		_target.wh_active = in->wh_active;
		_target.aim_active = in->aim_active;
		Irp->IoStatus.Information = sizeof(GAME_DATA);
		break;
	}
	case IOCTL_GET_FEATURES: {
		if (_target.proc) {
			GAME_DATA *out = (GAME_DATA*)Irp->AssociatedIrp.SystemBuffer;
			*out = _target;
			Irp->IoStatus.Information = sizeof(GAME_DATA);
		}
		break;
	}
	default: return STATUS_INVALID_DEVICE_REQUEST;
	}
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

void DriverUnload(PDRIVER_OBJECT) { DbgPrint("[WH] bye\n"); }

NTSTATUS DriverEntry(PDRIVER_OBJECT drv, PUNICODE_STRING) {
	drv->MajorUnload = DriverUnload;
	for (int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) drv->MajorFunction[i] = ctl_ioctl;
	return STATUS_SUCCESS;
}
