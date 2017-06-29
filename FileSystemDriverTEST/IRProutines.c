#include "Filter.h"
#include "Ntstrsafe.h "
#define  BUFFER_SIZE 1024

NTSTATUS 
logData
(
	UNICODE_STRING fileName,
	const char *data
)
{
	UNICODE_STRING     uniName;
	OBJECT_ATTRIBUTES  objAttr;
	HANDLE   handle;
	NTSTATUS ntstatus;
	IO_STATUS_BLOCK    ioStatusBlock;
	CHAR     buffer[BUFFER_SIZE];
	size_t  cb;

	char errorBuffer[BUFFER_SIZE];
	sprintf(errorBuffer, "%s => %wZ \n", data, &fileName);


	RtlInitUnicodeString(&uniName, L"\\DosDevices\\C:\\myfilterlog.txt");  // or L"\\SystemRoot\\example.txt"
	InitializeObjectAttributes(&objAttr, &uniName,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL, NULL);

	// Do not try to perform any file operations at higher IRQL levels.
	// Instead, you may use a work item or a system worker thread to perform file operations.

	if (KeGetCurrentIrql() != PASSIVE_LEVEL)
		return STATUS_INVALID_DEVICE_STATE;

	ntstatus = ZwCreateFile(
		&handle,
		FILE_APPEND_DATA,
		&objAttr, 
		&ioStatusBlock,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ,
		FILE_OPEN_IF,
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL, 0);

	if (NT_SUCCESS(ntstatus)) 
	{
		ntstatus = RtlStringCbPrintfA(buffer, sizeof(buffer), errorBuffer, 0x0);
		if (NT_SUCCESS(ntstatus)) 
		{
			ntstatus = RtlStringCbLengthA(buffer, sizeof(buffer), &cb);
			if (NT_SUCCESS(ntstatus))
			{
				ntstatus = ZwWriteFile(handle, NULL, NULL, NULL, &ioStatusBlock,
					buffer, cb, NULL, NULL);
			}
		}
		ZwClose(handle);
	}
	return STATUS_SUCCESS;
}

BOOLEAN
DoesContain
(
	UNICODE_STRING fileName,
	const char* bannedString
)
{
	char buffer[BUFFER_SIZE];
	sprintf(buffer, "%wZ", &fileName);

	if (strstr(buffer, bannedString) != NULL)
		return TRUE;

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
	PFILE_OBJECT pFileObject = IoGetCurrentIrpStackLocation(Irp)->FileObject;
	UNICODE_STRING fileName = pFileObject->FileName;


	if (DoesContain(fileName, "Desktop"))
	{
		if (DoesContain(fileName, ".png") ||
			DoesContain(fileName, ".bmp"))
		{
			DbgPrint("CREATE ACCESS DENIED => %wZ \n", &fileName);
			logData(fileName, "CREATE ACCESS DENIED");
			return STATUS_CANCELLED;
		}
		DbgPrint("CREATE REQUEST => %wZ \n", &fileName);
		logData(pFileObject->FileName, "CREATE REQUEST");
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
	PFILE_OBJECT pFileObject = IoGetCurrentIrpStackLocation(Irp)->FileObject;

	UNICODE_STRING fileName = pFileObject->FileName;


	if (DoesContain(fileName, "Desktop"))
	{
		if (DoesContain(fileName, ".txt"))
		{
			DbgPrint("INFORMATION CHANGE DENIED => %wZ \n", &fileName);
			logData(fileName, "INFORMATION CHANGE DENIED");
			return STATUS_CANCELLED;
		}
		DbgPrint("INFORMATION CHANGE => %wZ \n", &fileName);
		logData(pFileObject->FileName, "INFORMATION CHANGE");
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
	PFILE_OBJECT pFileObject = IoGetCurrentIrpStackLocation(Irp)->FileObject;
	
	UNICODE_STRING fileName = pFileObject->FileName;

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
	PFILE_OBJECT pFileObject = IoGetCurrentIrpStackLocation(Irp)->FileObject;
	UNICODE_STRING fileName = pFileObject->FileName;

	/* Can read .txt files but can't edit. */
	if (DoesContain(fileName, "Desktop"))
	{
		if (DoesContain(fileName, ".txt"))
		{
			DbgPrint("WRITE ACCESS DENIED => %wZ \n", &fileName);
			logData(fileName, "WRITE ACCESS DENIED");
			return STATUS_CANCELLED;
		}
		DbgPrint("WRITE REQUEST => %wZ \n", &fileName);
		logData(pFileObject->FileName, "WRITE REQUEST");
	}

	return FilterEvtIoDispatchPassThrough(DeviceObject, Irp);
}