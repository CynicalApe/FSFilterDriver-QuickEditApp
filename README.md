# FileSystemDriver

#FilterOfOz

* Currenly can deny access to individual files or volumes as a whole. 
* Logs every read and write request.
* Logs every denied attempt. (for example deletion of .txt files)
* By default it blocks any access to the .bmp and .png files that are located on the desktop
* User can acces the .txt files but only in read-only mode. Can't edit them.
* It doesn't allow name change, or deletion
* log file can be found in C:\mylogfile.txt, it can be read, but can't be written on or deleted.

# References
* https://www.codeproject.com/Articles/43586/File-System-Filter-Driver-Tutorial
* https://docs.microsoft.com/en-us/windows-hardware/drivers/ifs/file-system-filter-drivers
