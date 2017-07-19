#include <3ds.h>
#include <string.h>
#include "cxiloader.h"
#include "ifile.h"
#include "strings.h"

bool loadCxiExHeader(exheader_header *exheader, u64 progId)
{
    char path[] = "/luma/titles/0000000000000000.cxi";
    progIdToStr(path + 28, progId);
    
    Result r;
    IFile file;
    
    r = fileOpen(&file, ARCHIVE_SDMC, path, FS_OPEN_READ);
    
    if(!R_SUCCEEDED(r))
         return 0;

    bool ret;
    u64 fileSize;

    if(R_FAILED(IFile_GetSize(&file, &fileSize)) || fileSize < sizeof(*exheader)) 
        ret = false; //fix file size check to account for NCCH header
    else
    {
        IFile_Skip(&file, 0x200); //skip NCCH header

        u64 total;
        
        ret = R_SUCCEEDED(IFile_Read(&file, &total, exheader, sizeof(*exheader))) && total == sizeof(*exheader);
    }

    IFile_Close(&file);

    return ret;
}

bool loadCxiCode(u8 *codeData, u64 *codeSize, u64 maxSize, u64 progId) {
    char path[] = "/luma/titles/0000000000000000.cxi";
    progIdToStr(path + 28, progId);
    
    Result r;
    IFile file;
    u64 total;
    
    r = fileOpen(&file, ARCHIVE_SDMC, path, FS_OPEN_READ);
    if(!R_SUCCEEDED(r))
         return 0;
    
    NcchHeader ncchHeader;
    
    //todo validate file size
    if (!R_SUCCEEDED(IFile_Read(&file, &total, &ncchHeader, sizeof(ncchHeader))) 
        || total != sizeof(ncchHeader))
        goto error;
    
    u32 exeoff = ncchHeader.offset_exefs * NCCH_MEDIA_UNIT;
    //u32 exefssize  = ncchHeader.size_exefs * NCCH_MEDIA_UNIT;
    u32 exefshashsize  = ncchHeader.size_exefs_hash * NCCH_MEDIA_UNIT;
    
    if (exefshashsize != NCCH_MEDIA_UNIT)
        goto error;
    
    IFile_Skip(&file, exeoff); //seek from beginning
    
    ExeFsHeader exeFsHeader;
    
    if (!R_SUCCEEDED(IFile_Read(&file, &total, &exeFsHeader, sizeof(exeFsHeader))) 
        || total != sizeof(exeFsHeader))
        goto error;
		
    u32 codebinoffset = 0;
    u32 codebinsize = 0;
    for(int i=0; i<10; i++) {
        if (!strcmp(exeFsHeader.files[i].name, ".code")) {
            codebinoffset = exeFsHeader.files[i].offset;
            codebinsize = exeFsHeader.files[i].size;
            break;
        }
    }
    
    if (codebinsize == 0)
        goto error;
    
    if (codebinsize > maxSize)
        goto error;
    
    codebinoffset = exeoff + sizeof(exeFsHeader) + codebinoffset;
    
    IFile_Skip(&file, codebinoffset); //seek from beginning
    
    *codeSize = codebinsize;
    
    if (!R_SUCCEEDED(IFile_Read(&file, &total, codeData, codebinsize)) 
        || total != codebinsize)
        goto error;
    
    IFile_Close(&file);
    return 1;
    
error:
    IFile_Close(&file);
    
    return 0;
}