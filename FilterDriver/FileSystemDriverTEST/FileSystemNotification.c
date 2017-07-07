#include "Filter.h"

/*This routine is invoked whenever a file system has either registered or
unregistered itself as an active file system. */

VOID FilterNotificationCallback
(
	__in PDEVICE_OBJECT DeviceObject,
	__in BOOLEAN        FsActive
)
{

	if (FsActive)
	{
		FilterDeviceEvtAttachToFileSystemDevice(DeviceObject);
		return;
	}

	FilterDeviceEvtDetachFromFileSystemDevice(DeviceObject);
}

NTSTATUS FilterDeviceEvtAttachToFileSystemDevice(
	__in PDEVICE_OBJECT DeviceObject
)
{
	NTSTATUS        status = STATUS_SUCCESS;
	PDEVICE_OBJECT  filterDeviceObject = NULL;

	if (!FilterDeviceEvtIsAttachedToDevice(DeviceObject))
	{
		status = FilterDeviceEvtAttachToDevice(DeviceObject, &filterDeviceObject);

		if (!NT_SUCCESS(status))
		{
			return status;
		}

		status = EnumerateFileSystemsAndAttach(DeviceObject);

		if (!NT_SUCCESS(status))
		{
			FilterDeviceEvtDetachFromDevice(filterDeviceObject);
			return status;
		}
	}

	return STATUS_SUCCESS;
}

VOID FilterDeviceEvtDetachFromFileSystemDevice(
	__in PDEVICE_OBJECT DeviceObject
)
{
	PDEVICE_OBJECT device = NULL;

	for (device = DeviceObject->AttachedDevice; NULL != device; device = device->AttachedDevice)
	{
		if (FilterDeviceEvtIsMyDeviceObject(device))
		{
			FilterDeviceEvtDetachFromDevice(device);
			break;
		}
	}
}

NTSTATUS EnumerateFileSystemsAndAttach(
	__in PDEVICE_OBJECT DeviceObject
)
{
	NTSTATUS        status = STATUS_SUCCESS;
	ULONG           numDevices = 0;
	ULONG           i = 0;
	PDEVICE_OBJECT  devList[DEVOBJ_LIST_SIZE];

	/* Get the connected device list*/
	status = IoEnumerateDeviceObjectList(
		DeviceObject->DriverObject,
		devList,
		sizeof(devList),
		&numDevices);

	if (!NT_SUCCESS(status))
		return status;

	numDevices = min(numDevices, RTL_NUMBER_OF(devList));

	/* Go through the list and attach to the appropriate devices */
	for (i = 0; i < numDevices; i++) 
	{
		if (devList[i] != DeviceObject &&
			devList[i]->DeviceType == DeviceObject->DeviceType &&
			!FilterDeviceEvtIsAttachedToDevice(devList[i]))
		{
			status = FilterDeviceEvtAttachToDevice(devList[i], NULL);
		}

		ObDereferenceObject(devList[i]);
	}

	return STATUS_SUCCESS;
}