#include "Filter.h"
#include "Ntstrsafe.h "
#define  BUFFER_SIZE 1024

NTSTATUS 
logData
(
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

	RtlInitUnicodeString(&uniName, L"\\DosDevices\\C:\\myfilterlog.txt");  // or L"\\SystemRoot\\example.txt"
	InitializeObjectAttributes(&objAttr, &uniName,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL, NULL);

	// Do not try to perform any file operations at higher IRQL levels.
	// Instead, you may use a work item or a system worker thread to perform file operations.

	if (KeGetCurrentIrql() != PASSIVE_LEVEL)
		return STATUS_INVALID_DEVICE_STATE;

	ntstatus = ZwCreateFile(&handle,
		FILE_APPEND_DATA,
		&objAttr, &ioStatusBlock, NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ,
		FILE_OPEN_IF,
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL, 0);



	if (NT_SUCCESS(ntstatus)) {
		ntstatus = RtlStringCbPrintfA(buffer, sizeof(buffer), data, 0x0);
		if (NT_SUCCESS(ntstatus)) {
			ntstatus = RtlStringCbLengthA(buffer, sizeof(buffer), &cb);
			if (NT_SUCCESS(ntstatus)) {
				ntstatus = ZwWriteFile(handle, NULL, NULL, NULL, &ioStatusBlock,
					buffer, cb, NULL, NULL);
			}
		}
		ZwClose(handle);
	}
	return STATUS_SUCCESS;
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

NTSTATUS 
FilterEvtIoDispatchCreate 
(
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP           Irp
)
{
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


NTSTATUS
FilterEvtIoRead
(
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP           Irp
)
{
	PFILE_OBJECT pFileObject = IoGetCurrentIrpStackLocation(Irp)->FileObject;
	char buffer[BUFFER_SIZE];
	sprintf(buffer, "READ REQUEST: %wZ \n", &pFileObject->FileName);
	if (strstr(buffer, "Desktop") != NULL) {
		logData(buffer);
		DbgPrint(" %wZ => OPENED FOR READING \n", &pFileObject->FileName);
	}
	
	return FilterEvtIoDispatchPassThrough(DeviceObject, Irp);
}

NTSTATUS
FilterEvtIoWrite
(
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP           Irp
)
{
	PFILE_OBJECT pFileObject = IoGetCurrentIrpStackLocation(Irp)->FileObject;
	char buffer[BUFFER_SIZE];
	sprintf(buffer, "WRITE REQUEST: %wZ \n", &pFileObject->FileName);
	if (strstr(buffer, "Desktop") != NULL) {
		logData(buffer);
		DbgPrint(" %wZ => OPENED FOR WRITING \n", &pFileObject->FileName);
	}
	return FilterEvtIoDispatchPassThrough(DeviceObject, Irp);
}