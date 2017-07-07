# KDMF File System Filter Driver && Quick Edit APP

## The Driver
* Currenly can deny access to individual files or volumes as a whole. 
* Logs every denied request to a text file (which can't be edited or deleted or updated in any other way)
* It can categorize between read-only, write-only and no access restrictions
* log file can be found in C:\mylogfile.txt.
* uses :permission_enum:filtepath; style for each entry such as (:5:C:\random.txt where 5 is write-only access)
### References
* https://www.codeproject.com/Articles/43586/File-System-Filter-Driver-Tutorial
* https://docs.microsoft.com/en-us/windows-hardware/drivers/ifs/file-system-filter-drivers

## The APP
Universal Windows App that provides a minimalistic (and most certainly ugly..) GUI for updating the restictions file that
the kernel driver uses for handling the incoming filesystem requests.

![alt text](https://github.com/CynicalApe/FileSystemDriver/blob/master/FilterApp/appgeneral.png?raw=true)

### Folder and File selection modes for easy updating

![alt text](https://github.com/CynicalApe/FileSystemDriver/blob/master/FilterApp/folderselection.png?raw=true)

### Also supports good old manual typing

![alt text](https://github.com/CynicalApe/FileSystemDriver/blob/master/FilterApp/manual%20entry.png?raw=true)

* aditionaly if the user wishes to edit the restrictions file by themselves, they can use the "Refresh Permissions" button
to reload the interface
