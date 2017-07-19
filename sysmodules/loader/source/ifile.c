#include <3ds.h>
#include "ifile.h"
#include "fsldr.h"

int strnlen(const char *path, int max);

Result IFile_Open(IFile *file, FS_ArchiveID archiveId, FS_Path archivePath, FS_Path filePath, u32 flags)
{
  Result res;

  res = FSLDR_OpenFileDirectly(&file->handle, archiveId, archivePath, filePath, flags, 0);
  file->pos = 0;
  file->size = 0;
  return res;
}

Result IFile_Close(IFile *file)
{
  return FSFILE_Close(file->handle);
}

Result IFile_GetSize(IFile *file, u64 *size)
{
  Result res;

  res = FSFILE_GetSize(file->handle, size);
  file->size = *size;
  return res;
}

Result IFile_Skip(IFile *file, u64 pos) {
    file->pos = pos;
    return 0;
}

Result IFile_Read(IFile *file, u64 *total, void *buffer, u32 len)
{
  u32 read;
  u32 left;
  char *buf;
  u64 cur;
  Result res;

  if (len == 0)
  {
    *total = 0;
    return 0;
  }

  buf = (char *)buffer;
  cur = 0;
  left = len;
  while (1)
  {
    res = FSFILE_Read(file->handle, &read, file->pos, buf, left);
    if (R_FAILED(res))
    {
      break;
    }

    cur += read;
    file->pos += read;
    if (read == left)
    {
      break;
    }
    buf += read;
    left -= read;
  }

  *total = cur;
  return res;
}


Result IFile_Write(IFile *file, const void *buffer, u32 len)
{
    u64 size;
    u32 wrote;
    Result r;

    // Get current size.f
    r = FSFILE_GetSize(file->handle, &size);
    if (R_FAILED(r))
        return r;

    // Expand file size.
    r = FSFILE_SetSize(file->handle, size + len);
    if (R_FAILED(r))
        return r;

    // Write data.
    FSFILE_Write(file->handle, &wrote, size, buffer, len, FS_WRITE_FLUSH);
    
  return 0;
}

Result IFile_Truncate(IFile *file) {
    return FSFILE_SetSize(file->handle, 0);
}

Result fileOpen(IFile *file, FS_ArchiveID archiveId, const char *path, int flags)
{
    FS_Path filePath = {PATH_ASCII, strnlen(path, 255) + 1, path},
            archivePath = {PATH_EMPTY, 1, (u8 *)""};

    return IFile_Open(file, archiveId, archivePath, filePath, flags);
}

bool dirCheck(FS_ArchiveID archiveId, const char *path)
{
    bool ret;
    Handle handle;
    FS_Archive archive;
    FS_Path dirPath = {PATH_ASCII, strnlen(path, 255) + 1, path},
            archivePath = {PATH_EMPTY, 1, (u8 *)""};

    if(R_FAILED(FSLDR_OpenArchive(&archive, archiveId, archivePath))) ret = false;
    else
    {
        ret = R_SUCCEEDED(FSLDR_OpenDirectory(&handle, archive, dirPath));
        if(ret) FSDIR_Close(handle);
        FSLDR_CloseArchive(archive);
    }

    return ret;
}
