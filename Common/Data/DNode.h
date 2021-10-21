#pragma once

#include "DTypes.h"
#include "Base/DBuffer.h"


typedef enum tagDOpenFileMode
{
    DFILE_CREATE_NEW = 1,
    DFILE_CREATE_ALWAYS = 2,
    DFILE_OPEN_EXISTING = 3,
    DFILE_OPEN_ALWAYS = 4,
    DFILE_TRUNCATE_EXISTING = 5,
}DOpenFileMode;


typedef enum tagDFileMoveMethod
{
    DFILE_BEGIN = 0,
    DFILE_CURRENT = 1,
    DFILE_END = 2,
}DFileMoveMethod;


class DFile
{
public:
    DFile();
    explicit DFile(DFileHandle fh);
    ~DFile();
    DVoid Attach(DFileHandle fh);
    DFileHandle Detach();

    DBool	OpenFileRead(DCStr strPath);
    DBool	OpenFileWrite(DCStr strPath, DOpenFileMode nFlag = DFILE_OPEN_EXISTING);
    DBool	OpenFileRW(DCStr strPath, DOpenFileMode nFlag = DFILE_OPEN_EXISTING);

    DBuffer Read(DInt32 size, DInt32* result = NULL);
    DInt32  ReadTo(DByte* buf, DUInt32 bufsize);
    DBool	Write(DBuffer buf, DInt32* result = NULL);

    DBool	SeekToEnd();
    DInt64	GetSize();
    DInt64	GetPos();
    DBool	SetPos(DUInt64 pos, DFileMoveMethod dwMoveMethod = DFILE_BEGIN);
    DBool	SetEndOfFile();
    DBool	Flush();

    DBuffer IOControl(DInt32 controlCode, DUInt32 sizeofOut, DBuffer& inbuf);
    DVoid	Close();

public:
    static DBuffer FilePath2Buffer(DCStr strPath);

    D_DISALLOW_COPY_AND_ASSIGN(DFile)

public:
    DFileHandle m_hFile;
};
