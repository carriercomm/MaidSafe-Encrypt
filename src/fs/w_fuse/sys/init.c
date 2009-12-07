/*
  Dokan : user-mode file system library for Windows

  Copyright (C) 2008 Hiroki Asakawa info@dokan-dev.net

  http://dokan-dev.net/en

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free
Software Foundation; either version 3 of the License, or (at your option) any
later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#include "dokan.h"
#include <wdmsec.h>


VOID
DokanInitIrpList(
	 __in PIRP_LIST		IrpList
	 )
{
	InitializeListHead(&IrpList->ListHead);
	KeInitializeSpinLock(&IrpList->ListLock);
	KeInitializeEvent(&IrpList->NotEmpty, NotificationEvent, FALSE);
}


NTSTATUS
DokanCreateGlobalDiskDevice(
	__in PDRIVER_OBJECT DriverObject
	)
{
	WCHAR	deviceNameBuf[] = NTDEVICE_NAME_STRING; 
	WCHAR	symbolicLinkBuf[] = SYMBOLIC_NAME_STRING;
	NTSTATUS		status;
	UNICODE_STRING	deviceName;
	UNICODE_STRING	symbolicLinkName;
	PDEVICE_OBJECT	deviceObject;
	PDOKAN_GLOBAL	dokanGlobal;

	RtlInitUnicodeString(&deviceName, deviceNameBuf);
	RtlInitUnicodeString(&symbolicLinkName, symbolicLinkBuf);

	status = IoCreateDeviceSecure(
				DriverObject, // DriverObject
				sizeof(DOKAN_GLOBAL),// DeviceExtensionSize
				&deviceName, // DeviceName
				FILE_DEVICE_UNKNOWN, // DeviceType
				0,			// DeviceCharacteristics
				FALSE,		// Not Exclusive
				&SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RW_RES_R, // Default SDDL String
				NULL, // Device Class GUID
				&deviceObject); // DeviceObject

	if (!NT_SUCCESS(status)) {
		DDbgPrint("  IoCreateDevice returned 0x%x\n", status);
		return status;
	}
	ObReferenceObject(deviceObject);

	status = IoCreateSymbolicLink(&symbolicLinkName, &deviceName);
	if (!NT_SUCCESS(status)) {
		DDbgPrint("  IoCreateSymbolicLink returned 0x%x\n", status);
		IoDeleteDevice(deviceObject);
		return status;
	}

	dokanGlobal = deviceObject->DeviceExtension;

	RtlZeroMemory(dokanGlobal, sizeof(DOKAN_GLOBAL));
	DokanInitIrpList(&dokanGlobal->PendingService);
	DokanInitIrpList(&dokanGlobal->NotifyService);

	dokanGlobal->Identifier.Type = DGL;
	dokanGlobal->Identifier.Size = sizeof(DOKAN_GLOBAL);

	return STATUS_SUCCESS;
}


NTSTATUS
DokanCreateDiskDevice(
	__in PDRIVER_OBJECT DriverObject,
	__in ULONG			MountId,
	__in PDOKAN_GLOBAL	DokanGlobal,
	__in DEVICE_TYPE	DeviceType,
	__out PDokanDCB*	Dcb
	)
{
	WCHAR				deviceNameBuf[MAXIMUM_FILENAME_LENGTH];
	WCHAR				symbolicLinkBuf[MAXIMUM_FILENAME_LENGTH];
	WCHAR				fsDeviceNameBuf[MAXIMUM_FILENAME_LENGTH];
	PDEVICE_OBJECT		diskDeviceObject;
	PDEVICE_OBJECT		fsDeviceObject;
	PDokanDCB			dcb;
	PDokanVCB			vcb;
	UNICODE_STRING		deviceName;
	UNICODE_STRING		fsDeviceName;
	UNICODE_STRING		symbolicLinkName;
	NTSTATUS			status;

	//FS_FILTER_CALLBACKS filterCallbacks;

	// make DeviceName and SymboliLink
	swprintf(deviceNameBuf, NTDEVICE_NAME_STRING L"%u", MountId);
	swprintf(symbolicLinkBuf, SYMBOLIC_NAME_STRING L"%u", MountId);

	swprintf(fsDeviceNameBuf, L"\\Device\\dokanf%u", MountId);

	RtlInitUnicodeString(&deviceName, deviceNameBuf);
	RtlInitUnicodeString(&symbolicLinkName, symbolicLinkBuf);

	RtlInitUnicodeString(&fsDeviceName, fsDeviceNameBuf);

	//
	// make a DeviceObject for Disk Device
	//
	status = IoCreateDevice(DriverObject,				// DriverObject
							sizeof(DokanDCB),			// DeviceExtensionSize
							NULL,//&deviceName,			// DeviceName
							//FILE_DEVICE_DISK,			// DeviceType
							FILE_DEVICE_VIRTUAL_DISK,
							0,							// DeviceCharacteristics
							FALSE,						// Not Exclusive
							&diskDeviceObject			// DeviceObject
							);


	if (!NT_SUCCESS(status)) {
		DDbgPrint("  IoCreateDevice returned 0x%x\n", status);
		return status;
	}


	//status = IoRegisterShutdownNotification(diskDeviceObject);
	//if (!NT_SUCCESS (status)) {
    //    IoDeleteDevice(diskDeviceObject);
    //    return status;
    //}

	//
	// Initialize the device extension.
	//
	dcb = diskDeviceObject->DeviceExtension;
	*Dcb = dcb;
	dcb->DeviceObject = diskDeviceObject;
	dcb->Global = DokanGlobal;

	dcb->Identifier.Type = DCB;
	dcb->Identifier.Size = sizeof(DokanDCB);

	dcb->MountId = MountId;

	//
	// Establish user-buffer access method.
	//
	diskDeviceObject->Flags |= DO_DIRECT_IO;

	// initialize Event and Event queue
	DokanInitIrpList(&dcb->PendingIrp);
	DokanInitIrpList(&dcb->PendingEvent);
	DokanInitIrpList(&dcb->NotifyEvent);

	DokanInitIrpList(&DokanGlobal->PendingService);
	DokanInitIrpList(&DokanGlobal->NotifyService);

	KeInitializeEvent(&dcb->ReleaseEvent, NotificationEvent, FALSE);

	// "0" means not mounted
	dcb->Mounted = 0;

	ExInitializeResourceLite(&dcb->Resource);

	dcb->CacheManagerNoOpCallbacks.AcquireForLazyWrite  = &DokanNoOpAcquire;
	dcb->CacheManagerNoOpCallbacks.ReleaseFromLazyWrite = &DokanNoOpRelease;
	dcb->CacheManagerNoOpCallbacks.AcquireForReadAhead  = &DokanNoOpAcquire;
	dcb->CacheManagerNoOpCallbacks.ReleaseFromReadAhead = &DokanNoOpRelease;


	// to pretend to be mounted, make File System Device object
	status = IoCreateDeviceSecure(
				DriverObject,		// DriverObject
				sizeof(DokanVCB),	// DeviceExtensionSize
				&deviceName,		// DeviceName
				DeviceType, // DeviceType
				0,			// DeviceCharacteristics
				FALSE,		// Not Exclusive
				&SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RW_RES_R, // Default SDDL String
				NULL,				// Device Class GUID
				&fsDeviceObject);	// DeviceObject

	if (!NT_SUCCESS(status)) {
		DDbgPrint("  IoCreateDevice returned 0x%x\n", status);
		IoDeleteDevice(diskDeviceObject);
		return status;
	}

	vcb = fsDeviceObject->DeviceExtension;

	vcb->Identifier.Type = VCB;
	vcb->Identifier.Size = sizeof(DokanVCB);

	vcb->DeviceObject = fsDeviceObject;
	vcb->Dcb = dcb;

	dcb->Vcb = vcb;
	
	InitializeListHead(&vcb->NextFCB);

	InitializeListHead(&vcb->DirNotifyList);
	FsRtlNotifyInitializeSync(&vcb->NotifySync);

	ExInitializeFastMutex(&vcb->AdvancedFCBHeaderMutex);
#if _WIN32_WINNT >= 0x0501
	FsRtlSetupAdvancedHeader(&vcb->VolumeFileHeader, &vcb->AdvancedFCBHeaderMutex);
#else
	if (DokanFsRtlTeardownPerStreamContexts) {
		FsRtlSetupAdvancedHeader(&vcb->VolumeFileHeader, &vcb->AdvancedFCBHeaderMutex);
	}
#endif

    //RtlZeroMemory(&filterCallbacks, sizeof(FS_FILTER_CALLBACKS));

	// only be used by filter driver?
	//filterCallbacks.SizeOfFsFilterCallbacks = sizeof(FS_FILTER_CALLBACKS);
	//filterCallbacks.PreAcquireForSectionSynchronization = FatFilterCallbackAcquireForCreateSection;

	//FsRtlRegisterFileSystemFilterCallbacks(DriverObject,
	//										&filterCallbacks);

	//
	// Establish user-buffer access method.
	//
	fsDeviceObject->Flags |= DO_DIRECT_IO;

	diskDeviceObject->Vpb->DeviceObject = fsDeviceObject;
	diskDeviceObject->Vpb->RealDevice = fsDeviceObject;
	diskDeviceObject->Vpb->Flags = VPB_MOUNTED;
	diskDeviceObject->Vpb->VolumeLabelLength = wcslen(VOLUME_LABEL) * sizeof(WCHAR);
	swprintf(diskDeviceObject->Vpb->VolumeLabel, VOLUME_LABEL);
	diskDeviceObject->Vpb->SerialNumber = 0x19831116;

	//IoRegisterFileSystem(fsDeviceObject);
	ObReferenceObject(fsDeviceObject);
	ObReferenceObject(diskDeviceObject);

	//
	// Create a symbolic link for userapp to interact with the driver.
	//
	status = IoCreateSymbolicLink(&symbolicLinkName, &deviceName);

	if (!NT_SUCCESS(status)) {
		IoDeleteDevice(diskDeviceObject);
		IoDeleteDevice(fsDeviceObject);
		DDbgPrint("  IoCreateSymbolicLink returned 0x%x\n", status);
		return status;
	}

	// Mark devices as initialized
	diskDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
	fsDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

	return STATUS_SUCCESS;
}
