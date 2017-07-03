#include "Filter.h"
#include "Ntstrsafe.h "
#define  BUFFER_SIZE 1024

VOID
readRestrictions()
{
	UNICODE_STRING     uniName;
	OBJECT_ATTRIBUTES  objAttr;
	HANDLE   handle;
	NTSTATUS ntstatus;
	IO_STATUS_BLOCK    ioStatusBlock;
	int cb = 0;
	char *errorBuffer = (char *)ExAllocatePoolWithTag(NonPagedPool, BUFFER_SIZE, 'buf1');

	RtlInitUnicodeString(&uniName, L"\\DosDevices\\C:\\restrictions.txt");  // or L"\\SystemRoot\\example.txt"
	InitializeObjectAttributes(&objAttr, &uniName,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL, NULL);

	// Do not try to perform any file operations at higher IRQL levels.
	// Instead, you may use a work item or a system worker thread to perform file operations.

	if (KeGetCurrentIrql() != PASSIVE_LEVEL)
		return STATUS_INVALID_DEVICE_STATE;

	ntstatus = ZwCreateFile
	(
		&handle,
		FILE_APPEND_DATA,
		&objAttr,
		&ioStatusBlock,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ,
		FILE_OPEN_IF,
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0
	);

	if (NT_SUCCESS(ntstatus))
	{
		ntstatus = ZwReadFile(handle, NULL, NULL, NULL, &ioStatusBlock,
			errorBuffer, BUFFER_SIZE, NULL, NULL);

		for (; cb < BUFFER_SIZE && errorBuffer[cb] != '#'; cb++);
		ban = (char *)ExAllocatePoolWithTag(NonPagedPool, cb, 'ban1');
		for (int i = 0; i < cb; i++) {
			ban[i] = errorBuffer[i];
		}
		DbgPrint("BAN: %s", ban);
		ZwClose(handle);
	}

	ExFreePoolWithTag(errorBuffer, 'buf1');
}


VOID
logData
(
	PUNICODE_STRING fileName,
	const char *data
)
{
	UNICODE_STRING     uniName;
	OBJECT_ATTRIBUTES  objAttr;
	HANDLE   handle;
	NTSTATUS ntstatus;
	IO_STATUS_BLOCK    ioStatusBlock;
	int cb = 0;
	char *errorBuffer = (char *)ExAllocatePoolWithTag(NonPagedPool, BUFFER_SIZE, 'buf1');
	sprintf(errorBuffer, "%s => %wZ \r\n", data, &(*fileName));

	if (strstr(errorBuffer, "myfilterlog") != NULL || strstr(errorBuffer, "restrictions"))
		goto Exit;

	for (; cb < BUFFER_SIZE && errorBuffer[cb] != '\r'; cb++);
	errorBuffer[cb++] = '\r';
	errorBuffer[cb++] = '\n';

	RtlInitUnicodeString(&uniName, L"\\DosDevices\\C:\\myfilterlog.txt");  // or L"\\SystemRoot\\example.txt"
	InitializeObjectAttributes(&objAttr, &uniName,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL, NULL);

	// Do not try to perform any file operations at higher IRQL levels.
	// Instead, you may use a work item or a system worker thread to perform file operations.

	if (KeGetCurrentIrql() != PASSIVE_LEVEL)
		return STATUS_INVALID_DEVICE_STATE;

	ntstatus = ZwCreateFile
	(
		&handle,
		FILE_APPEND_DATA,
		&objAttr,
		&ioStatusBlock,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ,
		FILE_OPEN_IF,
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0
	);

	if (NT_SUCCESS(ntstatus))
	{
		ntstatus = ZwWriteFile(handle, NULL, NULL, NULL, &ioStatusBlock,
			errorBuffer, cb, NULL, NULL);
		ZwClose(handle);
	}
Exit:
	ExFreePoolWithTag(errorBuffer, 'buf1');
}



BOOLEAN
DoesContain
(
	PUNICODE_STRING fileName,
	const char* bannedString
)
{
	char *buffer = (char *)ExAllocatePoolWithTag(NonPagedPool, fileName->Length, 'buf1');
	sprintf(buffer, "%wZ", &(*fileName));

	if (strstr(buffer, bannedString) != NULL)
	{
		ExFreePoolWithTag(buffer, 'buf1');
		return TRUE;
	}

	ExFreePoolWithTag(buffer, 'buf1');
	return FALSE;
}

NTSTATUS
FilterEvtIoDispatchPassThrough 
(
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP           Irp
)
{
	PFSFILTER_DEVICE_EXTENSION pDevExt = (PFSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
	IoSkipCurrentIrpStackLocation(Irp);
	return IoCallDriver(pDevExt->AttachedToDeviceObject, Irp);
}

NTSTATUS FilterEvtIoQueryInformation(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	return FilterEvtIoDispatchPassThrough(DeviceObject, Irp);
}

/* File Handle IRP */
NTSTATUS 
FilterEvtIoDispatchCreate 
(
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP           Irp
)
{
	PUNICODE_STRING fileName = &(IoGetCurrentIrpStackLocation(Irp)->FileObject->FileName);

	if (ban && DoesContain(fileName, ban))
	{
		if (DoesContain(fileName, ".png") ||
			DoesContain(fileName, ".bmp"))
		{
			DbgPrint("CREATE ACCESS DENIED => %s", *fileName);
			logData(fileName, "CREATE ACCESS DENIED");
			return STATUS_CANCELLED;
		}
	}

	return FilterEvtIoDispatchPassThrough(DeviceObject, Irp);
}

NTSTATUS 
FilterEvtIoClose 
(
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP           Irp
)
{
	return FilterEvtIoDispatchPassThrough(DeviceObject, Irp);
}

/* File File Information IRP */
NTSTATUS FilterEvtIoSetInformation(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	PUNICODE_STRING fileName = &(IoGetCurrentIrpStackLocation(Irp)->FileObject->FileName);

	if (ban && DoesContain(fileName, ban) || DoesContain(fileName,"myfilterlog"))
	{
		if (DoesContain(fileName, ".txt"))
		{
			logData(fileName, "INFORMATION CHANGE DENIED");
			return STATUS_CANCELLED;
		}
		logData(fileName, "INFORMATION CHANGE");
	}

	if (DoesContain(fileName, "myfilterlog.txt"))
		return STATUS_CANCELLED;

	return FilterEvtIoDispatchPassThrough(DeviceObject, Irp);
}

/* From device to file system */
NTSTATUS
FilterEvtIoRead
(
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP           Irp
)
{
	return FilterEvtIoDispatchPassThrough(DeviceObject, Irp);
}


/* from file system to device */
NTSTATUS
FilterEvtIoWrite
(
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP           Irp
)
{
	PUNICODE_STRING fileName = &(IoGetCurrentIrpStackLocation(Irp)->FileObject->FileName);
	return FilterEvtIoDispatchPassThrough(DeviceObject, Irp);
}