#pragma once
#include "Filter.h"


NTSTATUS 
FilterDeviceEvtAttachToDevice (
	__in PDEVICE_OBJECT         DeviceObject,
	__out_opt PDEVICE_OBJECT*   pFilterDeviceObject
)
{
	NTSTATUS                    status = STATUS_SUCCESS;
	PFSFILTER_DEVICE_EXTENSION  deviceExtension = NULL;
	PDEVICE_OBJECT              filterDeviceObject = NULL;
	ULONG                       counter = 0;

	ASSERT(!FilterDeviceEvtIsAttachedToDevice(DeviceObject));

	/* Create new device object*/
	status = IoCreateDevice(
		myFilterObject,
		sizeof(FSFILTER_DEVICE_EXTENSION),
		NULL,
		DeviceObject->DeviceType,
		0,
		FALSE,
		&filterDeviceObject);
	
	/* Return if fails*/
	if (!NT_SUCCESS(status))
		return status;

	deviceExtension = (PFSFILTER_DEVICE_EXTENSION)filterDeviceObject->DeviceExtension;

	/*
	Propagating the DO_BUFFERED_IO, DO_DIRECT_IO and secure open flags 
	*/
	if (FlagOn(DeviceObject->Flags, DO_BUFFERED_IO))
		SetFlag(filterDeviceObject->Flags, DO_BUFFERED_IO);

	if (FlagOn(DeviceObject->Flags, DO_DIRECT_IO))
		SetFlag(filterDeviceObject->Flags, DO_DIRECT_IO);

	if (FlagOn(DeviceObject->Characteristics, FILE_DEVICE_SECURE_OPEN))
		SetFlag(filterDeviceObject->Characteristics, FILE_DEVICE_SECURE_OPEN);

	for (counter = 0; counter < 10; counter++)
	{
		LARGE_INTEGER delayInterval;

		/*The IoAttachDeviceToDeviceStackSafe routine attaches the caller's device
		object to the topmost device object in a driver stack.*/
		status = IoAttachDeviceToDeviceStackSafe(
			filterDeviceObject,  //SourceDevice
			DeviceObject,	//TargetDevice
			&deviceExtension->AttachedToDeviceObject); //AttachedToDeviceObject

		if (NT_SUCCESS(status))
			break;

		delayInterval.QuadPart = (1000 * DELAY_ONE_MILLISECOND);
		KeDelayExecutionThread(KernelMode, FALSE, &delayInterval);
	}

	if (!NT_SUCCESS(status)) {
		/* clean everything up if initializaiton fails*/
		IoDeleteDevice(filterDeviceObject);

		/* avoid dangling pointers */
		filterDeviceObject = NULL;
		return status;
	}

	ClearFlag(filterDeviceObject->Flags, DO_DEVICE_INITIALIZING);

	if (pFilterDeviceObject)
		*pFilterDeviceObject = filterDeviceObject;

	return status;
}

void
FilterDeviceEvtDetachFromDevice (
	__in PDEVICE_OBJECT DeviceObject
)
{
	IoDetachDevice(((PFSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject);
	IoDeleteDevice(DeviceObject);
}


BOOLEAN FilterDeviceEvtIsAttachedToDevice (
	__in PDEVICE_OBJECT DeviceObject
)
{
	PDEVICE_OBJECT nextDevObj = NULL;
	PDEVICE_OBJECT currentDevObj = IoGetAttachedDeviceReference(DeviceObject);

	do
	{
		if (FilterDeviceEvtIsMyDeviceObject(currentDevObj))
		{
			ObDereferenceObject(currentDevObj);
			return TRUE;
		}

		nextDevObj = IoGetLowerDeviceObject(currentDevObj);

		ObDereferenceObject(currentDevObj);
		currentDevObj = nextDevObj;
	} while (NULL != currentDevObj);

	return FALSE;
}
