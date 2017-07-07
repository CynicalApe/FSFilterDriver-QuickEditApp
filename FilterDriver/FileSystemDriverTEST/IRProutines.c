#include "Filter.h"
#include "Ntstrsafe.h "
#define  BUFFER_SIZE 1024

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
	char *buffer = (char *)ExAllocatePoolWithTag(NonPagedPool, BUFFER_SIZE, 'buf1');
	
	for (int i = 0; i < BUFFER_SIZE; i++)
		buffer[i] = '\0';
	noAccCount = 0;
	readAccCount = 0;
	writeAccCount = 0;
	completeAccCount = 0;

	DbgPrint("UPDATE10");
	noAccess = (char**)ExAllocatePoolWithTag(NonPagedPool, 10 * sizeof(char*), 'buf2');
	readOnlyAccess = (char**)ExAllocatePoolWithTag(NonPagedPool, 10 * sizeof(char*), 'buf3');
	writeOnlyAccess = (char**)ExAllocatePoolWithTag(NonPagedPool, 10 * sizeof(char*), 'buf4');
	completeAccess = (char**)ExAllocatePoolWithTag(NonPagedPool, 10 * sizeof(char*), 'buf5');

	for (int i = 0; i < 10; i++)
		noAccess[i] = NULL;

	for (int i = 0; i < 10; i++)
		readOnlyAccess[i] = NULL;

	for (int i = 0; i < 10; i++)
		writeOnlyAccess[i] = NULL;

	for (int i = 0; i < 10; i++)
		completeAccess[i] = NULL;

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

	char **destinationBuffer = NULL;
	

	char currentbuffer[BUFFER_SIZE];
	char ch = "=";
	int filepathend = 0;
	if (NT_SUCCESS(ntstatus))
	{
		ntstatus = ZwReadFile(handle, NULL, NULL, NULL, &ioStatusBlock,
			buffer, BUFFER_SIZE, 0, NULL);
		for (int i = 0; ch != NULL && i < BUFFER_SIZE; i++)
		{
			ch = buffer[i];
			if (ch == ':')
			{
				ch = buffer[++i];
				switch (ch)
				{
				case('0'):
					destinationBuffer = &noAccess[noAccCount++];
					break;
				case('1'):
					destinationBuffer = &readOnlyAccess[readAccCount++];
					break;
				case('5'):
					destinationBuffer = &writeOnlyAccess[writeAccCount++];
					break;
				case('7'):
					destinationBuffer = &completeAccess[completeAccCount++];
					break;
				default:
					break;
				}
				i++;
			}
			else if (ch == ';')
			{
				//file path finished;
				char *temp = (char*)ExAllocatePoolWithTag(NonPagedPool, filepathend + 1, 'buf2');
				temp[filepathend] = '\0';
				for (int q = 0; q < filepathend; q++)
				{
					temp[q] = currentbuffer[q];
				}
				*destinationBuffer = temp;
				filepathend = 0;
			}
			else if (ch != '\n' && ch != ' ' && ch != '\r')
			{
				currentbuffer[filepathend++] = ch;
			}
		}
		for (; cb < BUFFER_SIZE && buffer[cb] != '#'; cb++);
		ban = (char *)ExAllocatePoolWithTag(NonPagedPool, cb, 'ban1');
		for (int i = 0; i < cb; i++) {
			ban[i] = buffer[i];
		}
		DbgPrint("RESTRICTION LIST: \n %s", ban);
		ZwClose(handle);
	}

	ExFreePoolWithTag(buffer, 'buf1');
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
	const char** bannedStringArray,
	const short int arraySize
)
{
	char *buffer = (char *)ExAllocatePoolWithTag(NonPagedPool, fileName->Length, 'buf1');
	sprintf(buffer, "%wZ", &(*fileName));

	for (int i = 0; i < arraySize; i++)
	{
		if (strstr(buffer, bannedStringArray[i]) != NULL)
		{
			ExFreePoolWithTag(buffer, 'buf1');
			return TRUE;
		}
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

	if (DoesContain(fileName, noAccess, noAccCount))
	{
			DbgPrint("CREATE ACCESS DENIED => %wZ", *fileName);
			logData(fileName, "CREATE ACCESS DENIED");
			return STATUS_CANCELLED;
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
	if (DoesContain(fileName, writeOnlyAccess, writeAccCount))
	{
		DbgPrint("INFORMATION CHANGE DENIED => %wZ", *fileName);
		logData(fileName, "INFORMATION CHANGE DENIED");
		return STATUS_CANCELLED;
	}
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
	if (DoesContain(fileName, readOnlyAccess, readAccCount))
	{
		DbgPrint("WRITE ACCESS DENIED => %wZ", *fileName);
		logData(fileName, "WRITE ACCESS DENIED");
		return STATUS_CANCELLED;
	}
	return FilterEvtIoDispatchPassThrough(DeviceObject, Irp);
}