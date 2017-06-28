#pragma once
#include <ntifs.h>
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
// Defines

#define DELAY_ONE_MICROSECOND   (-10)
#define DELAY_ONE_MILLISECOND   (DELAY_ONE_MICROSECOND * 1000)
#define DELAY_ONE_SECOND        (DELAY_ONE_MILLISECOND * 1000)

#define DEVOBJ_LIST_SIZE        64

//  Macro to test if FAST_IO_DISPATCH handling routine is valid
#define VALID_FAST_IO_DISPATCH_HANDLER(_FastIoDispatchPtr, _FieldName) \
    (((_FastIoDispatchPtr) != NULL) && \
    (((_FastIoDispatchPtr)->SizeOfFastIoDispatch) >= \
    (FIELD_OFFSET(FAST_IO_DISPATCH, _FieldName) + sizeof(void *))) && \
    ((_FastIoDispatchPtr)->_FieldName != NULL))

//////////////////////////////////////////////////////////////////////////
// Structures

typedef struct _FSFILTER_DEVICE_EXTENSION
{
	PDEVICE_OBJECT AttachedToDeviceObject;
} FSFILTER_DEVICE_EXTENSION, *PFSFILTER_DEVICE_EXTENSION;

//////////////////////////////////////////////////////////////////////////
// General Functions
VOID logData(
	const char *data
);

NTSTATUS FilterEvtIoDispatchPassThrough(
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP           Irp
);

NTSTATUS FilterEvtIoDispatchCreate(
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP           Irp
);

NTSTATUS FilterEvtIoRead(
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP           Irp
);

NTSTATUS FilterEvtIoClose(
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP           Irp
);


NTSTATUS FilterDeviceEvtAttachToDevice(
	__in PDEVICE_OBJECT         DeviceObject,
	__out_opt PDEVICE_OBJECT*   pFilterDeviceObject
);

VOID FilterDeviceEvtDetachFromDevice(
	__in PDEVICE_OBJECT DeviceObject
);

BOOLEAN FilterDeviceEvtIsAttachedToDevice(
	__in PDEVICE_OBJECT DeviceObject
);

BOOLEAN FilterDeviceEvtIsMyDeviceObject(
	__in PDEVICE_OBJECT DeviceObject
);

VOID FilterNotificationCallback(
	__in PDEVICE_OBJECT DeviceObject,
	__in BOOLEAN        FsActive
);

//////////////////////////////////////////////////////////////////////////
// Fast-IO Handlers

BOOLEAN FilterEvtFastIoCheckIfPossible(
	__in PFILE_OBJECT       FileObject,
	__in PLARGE_INTEGER     FileOffset,
	__in ULONG              Length,
	__in BOOLEAN            Wait,
	__in ULONG              LockKey,
	__in BOOLEAN            CheckForReadOperation,
	__out PIO_STATUS_BLOCK  IoStatus,
	__in PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FilterEvtFastIoRead(
	__in PFILE_OBJECT       FileObject,
	__in PLARGE_INTEGER     FileOffset,
	__in ULONG              Length,
	__in BOOLEAN            Wait,
	__in ULONG              LockKey,
	__out PVOID             Buffer,
	__out PIO_STATUS_BLOCK  IoStatus,
	__in PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FilterEvtFastIoWrite(
	__in PFILE_OBJECT       FileObject,
	__in PLARGE_INTEGER     FileOffset,
	__in ULONG              Length,
	__in BOOLEAN            Wait,
	__in ULONG              LockKey,
	__in PVOID              Buffer,
	__out PIO_STATUS_BLOCK  IoStatus,
	__in PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FilterEvtFastIoQueryBasicInfo(
	__in PFILE_OBJECT       FileObject,
	__in BOOLEAN            Wait,
	__out PFILE_BASIC_INFORMATION Buffer,
	__out PIO_STATUS_BLOCK  IoStatus,
	__in PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FilterEvtFastIoQueryStandardInfo(
	__in PFILE_OBJECT       FileObject,
	__in BOOLEAN            Wait,
	__out PFILE_STANDARD_INFORMATION Buffer,
	__out PIO_STATUS_BLOCK  IoStatus,
	__in PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FilterEvtFastIoLock(
	__in PFILE_OBJECT       FileObject,
	__in PLARGE_INTEGER     FileOffset,
	__in PLARGE_INTEGER     Length,
	__in PEPROCESS          ProcessId,
	__in ULONG              Key,
	__in BOOLEAN            FailImmediately,
	__in BOOLEAN            ExclusiveLock,
	__out PIO_STATUS_BLOCK  IoStatus,
	__in PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FilterEvtFastIoUnlockSingle(
	__in PFILE_OBJECT       FileObject,
	__in PLARGE_INTEGER     FileOffset,
	__in PLARGE_INTEGER     Length,
	__in PEPROCESS          ProcessId,
	__in ULONG              Key,
	__out PIO_STATUS_BLOCK  IoStatus,
	__in PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FilterEvtFastIoUnlockAll(
	__in PFILE_OBJECT       FileObject,
	__in PEPROCESS          ProcessId,
	__out PIO_STATUS_BLOCK  IoStatus,
	__in PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FilterEvtFastIoUnlockAllByKey(
	__in PFILE_OBJECT       FileObject,
	__in PVOID              ProcessId,
	__in ULONG              Key,
	__out PIO_STATUS_BLOCK  IoStatus,
	__in PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FilterEvtFastIoDeviceControl(
	__in PFILE_OBJECT       FileObject,
	__in BOOLEAN            Wait,
	__in_opt PVOID          InputBuffer,
	__in ULONG              InputBufferLength,
	__out_opt PVOID         OutputBuffer,
	__in ULONG              OutputBufferLength,
	__in ULONG              IoControlCode,
	__out PIO_STATUS_BLOCK  IoStatus,
	__in PDEVICE_OBJECT     DeviceObject
);

VOID FilterEvtFastIoDetachDevice(
	__in PDEVICE_OBJECT     SourceDevice,
	__in PDEVICE_OBJECT     TargetDevice
);

BOOLEAN FilterEvtFastIoQueryNetworkOpenInfo(
	__in PFILE_OBJECT       FileObject,
	__in BOOLEAN            Wait,
	__out PFILE_NETWORK_OPEN_INFORMATION Buffer,
	__out PIO_STATUS_BLOCK  IoStatus,
	__in PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FilterEvtFastIoMdlRead(
	__in PFILE_OBJECT       FileObject,
	__in PLARGE_INTEGER     FileOffset,
	__in ULONG              Length,
	__in ULONG              LockKey,
	__out PMDL*             MdlChain,
	__out PIO_STATUS_BLOCK  IoStatus,
	__in PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FilterEvtFastIoMdlReadComplete(
	__in PFILE_OBJECT       FileObject,
	__in PMDL               MdlChain,
	__in PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FilterEvtFastIoPrepareMdlWrite(
	__in PFILE_OBJECT       FileObject,
	__in PLARGE_INTEGER     FileOffset,
	__in ULONG              Length,
	__in ULONG              LockKey,
	__out PMDL*             MdlChain,
	__out PIO_STATUS_BLOCK  IoStatus,
	__in PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FilterEvtFastIoMdlWriteComplete(
	__in PFILE_OBJECT       FileObject,
	__in PLARGE_INTEGER     FileOffset,
	__in PMDL               MdlChain,
	__in PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FilterEvtFastIoReadCompressed(
	__in PFILE_OBJECT       FileObject,
	__in PLARGE_INTEGER     FileOffset,
	__in ULONG              Length,
	__in ULONG              LockKey,
	__out PVOID             Buffer,
	__out PMDL*             MdlChain,
	__out PIO_STATUS_BLOCK  IoStatus,
	__out struct _COMPRESSED_DATA_INFO* CompressedDataInfo,
	__in ULONG              CompressedDataInfoLength,
	__in PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FilterEvtFastIoWriteCompressed(
	__in PFILE_OBJECT       FileObject,
	__in PLARGE_INTEGER     FileOffset,
	__in ULONG              Length,
	__in ULONG              LockKey,
	__in PVOID              Buffer,
	__out PMDL*             MdlChain,
	__out PIO_STATUS_BLOCK  IoStatus,
	__in struct _COMPRESSED_DATA_INFO*  CompressedDataInfo,
	__in ULONG              CompressedDataInfoLength,
	__in PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FilterEvtFastIoMdlReadCompleteCompressed(
	__in PFILE_OBJECT       FileObject,
	__in PMDL               MdlChain,
	__in PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FilterEvtFastIoMdlWriteCompleteCompressed(
	__in PFILE_OBJECT       FileObject,
	__in PLARGE_INTEGER     FileOffset,
	__in PMDL               MdlChain,
	__in PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FilterEvtFastIoQueryOpen(
	__in PIRP               Irp,
	__out PFILE_NETWORK_OPEN_INFORMATION NetworkInformation,
	__in PDEVICE_OBJECT     DeviceObject
);

NTSTATUS FilterEvtIoWrite(
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP           Irp
);

VOID FilterEvtUnload(
	__in PDRIVER_OBJECT DriverObject
);

NTSTATUS FilterDeviceEvtAttachToFileSystemDevice(
	__in PDEVICE_OBJECT DeviceObject
);

VOID FilterDeviceEvtDetachFromFileSystemDevice(
	__in PDEVICE_OBJECT DeviceObject
);

NTSTATUS EnumerateFileSystemsAndAttach(
	__in PDEVICE_OBJECT DeviceObject
);

//////////////////////////////////////////////////////////////////////////
// Global data

extern PDRIVER_OBJECT myFilterObject;

#pragma once
