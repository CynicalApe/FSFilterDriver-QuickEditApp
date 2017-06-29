#include "Filter.h"


/*File systems and file system filters are required to support IRPs,
but they are not required to support fast I/O. However, file systems
and file system filters must implement fast I/O routines. Even if file
systems and file system filters do not support fast I/O, they must define
a fast I/O routine that returns FALSE (that is, the fast I/O routine
does not implement any functionality). */

FAST_IO_DISPATCH FilterFastIo =
{
	sizeof(FAST_IO_DISPATCH),
	FilterEvtFastIoCheckIfPossible,
	FilterEvtFastIoRead,
	FilterEvtFastIoWrite,
	FilterEvtFastIoQueryBasicInfo,
	FilterEvtFastIoQueryStandardInfo,
	FilterEvtFastIoLock,
	FilterEvtFastIoUnlockSingle,
	FilterEvtFastIoUnlockAll,
	FilterEvtFastIoUnlockAllByKey,
	FilterEvtFastIoDeviceControl,
	NULL,
	NULL,
	FilterEvtFastIoDetachDevice,
	FilterEvtFastIoQueryNetworkOpenInfo,
	NULL,
	FilterEvtFastIoMdlRead,
	FilterEvtFastIoMdlReadComplete,
	FilterEvtFastIoPrepareMdlWrite,
	FilterEvtFastIoMdlWriteComplete,
	FilterEvtFastIoReadCompressed,
	FilterEvtFastIoWriteCompressed,
	FilterEvtFastIoMdlReadCompleteCompressed,
	FilterEvtFastIoMdlWriteCompleteCompressed,
	FilterEvtFastIoQueryOpen,
	NULL,
	NULL,
	NULL,
};

PDRIVER_OBJECT   myFilterObject = NULL;

NTSTATUS DriverEntry(
	__inout PDRIVER_OBJECT  DriverObject,
	__in    PUNICODE_STRING RegistryPath
)
{
	NTSTATUS status = STATUS_SUCCESS;
	ULONG    counter = 0;

	myFilterObject = DriverObject;

	/* To register I/O request packet (IRP) dispatch routines, you must
	store the entry points of these routines into the MajorFunction member
	of the driver object. */
	for (counter = 0; counter <= IRP_MJ_MAXIMUM_FUNCTION; ++counter)
		DriverObject->MajorFunction[counter] = FilterEvtIoDispatchPassThrough;

	DriverObject->MajorFunction[IRP_MJ_CREATE] = FilterEvtIoDispatchCreate;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = FilterEvtIoClose;
	DriverObject->MajorFunction[IRP_MJ_READ] = FilterEvtIoRead;
	DriverObject->MajorFunction[IRP_MJ_WRITE] = FilterEvtIoWrite;
	DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION] = FilterEvtIoSetInformation;
	DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION] = FilterEvtIoQueryInformation;

	/* For loop assigns a default dispatch routine for all IRP major function
	codes. I allows us to stop I/O manager from completing any unrecognized
	request with STATUS_INVALID_DEVICE_REQUEST by default. File system
	filter drivers should never reject any request that they are not interested
	in handling since those requests could be for another device. */



	/*To register the file system filter driver's fast I/O dispatch routines,
	you must allocate and initialize a fast I/O dispatch table, store the
	entry points of the fast I/O dispatch routines into the table, and store
	the address of the table in the FastIoDispatch member of the driver
	object.*/

	DriverObject->FastIoDispatch = &FilterFastIo;

	status = IoRegisterFsRegistrationChange(DriverObject, FilterNotificationCallback);
	
	if (!NT_SUCCESS(status))
		return status;
	
	/* Unlaod routine*/
	DriverObject->DriverUnload = FilterEvtUnload;
	return STATUS_SUCCESS;
}

/* Unload Prototype */

VOID FilterEvtUnload (
	__in PDRIVER_OBJECT DriverObject
)
{
	ULONG           numOfAttachedDevices = 0;
	ULONG           counter = 0;
	LARGE_INTEGER   interval;
	PDEVICE_OBJECT  deviceList[DEVOBJ_LIST_SIZE];

	interval.QuadPart = (5 * DELAY_ONE_SECOND); //delay 5 seconds
	
	/* Unregister callback */
	IoUnregisterFsRegistrationChange(DriverObject, FilterNotificationCallback);

	/* Detach every device */
	do {
		IoEnumerateDeviceObjectList (
			DriverObject,
			deviceList,
			sizeof(deviceList),
			&numOfAttachedDevices
		);

		numOfAttachedDevices = min(numOfAttachedDevices, RTL_NUMBER_OF(deviceList));

		for (counter = 0; counter < numOfAttachedDevices; counter++) {
			FilterDeviceEvtDetachFromDevice(deviceList[counter]);
			ObDereferenceObject(deviceList[counter]);
		}

		KeDelayExecutionThread(KernelMode, FALSE, &interval);
	} while (numOfAttachedDevices != 0);
}

BOOLEAN FilterDeviceEvtIsMyDeviceObject(
	__in PDEVICE_OBJECT DeviceObject
)
{
	return DeviceObject->DriverObject == myFilterObject;
}
