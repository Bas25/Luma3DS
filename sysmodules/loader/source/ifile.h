#pragma once

#include <3ds/types.h>

typedef struct
{
  Handle handle;
  u64 pos;
  u64 size;
} IFile;

Result IFile_Open(IFile *file, FS_ArchiveID archiveId, FS_Path archivePath, FS_Path filePath, u32 flags);
Result IFile_Close(IFile *file);
Result IFile_GetSize(IFile *file, u64 *size);
Result IFile_Read(IFile *file, u64 *total, void *buffer, u32 len);
Result IFile_Write(IFile *file, const void *buffer, u32 len);
Result IFile_Truncate(IFile *file);
Result IFile_Skip(IFile *file, u64 pos);
Result fileOpen(IFile *file, FS_ArchiveID archiveId, const char *path, int flags);
bool dirCheck(FS_ArchiveID archiveId, const char *path);