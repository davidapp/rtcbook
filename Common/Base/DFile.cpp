#include "DFile.h"
#include "DXP.h"

#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
// 把属性可选值都列在这里，要用的时候，就不需要看文档了
#define WIN_ACCESS 0|GENERIC_READ|GENERIC_WRITE|GENERIC_EXECUTE|GENERIC_ALL
#define WIN_SHARE  0|FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE
#define WIN_CREATE CREATE_NEW|CREATE_ALWAYS|OPEN_EXISTING|OPEN_ALWAYS|TRUNCATE_EXISTING
#define WIN_FLAGS  FILE_FLAG_NO_BUFFERING|FILE_FLAG_SEQUENTIAL_SCAN|FILE_FLAG_RANDOM_ACCESS|FILE_FLAG_WRITE_THROUGH| \
					FILE_FLAG_DELETE_ON_CLOSE|FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_POSIX_SEMANTICS|FILE_FLAG_OPEN_REPARSE_POINT| \
					FILE_FLAG_OPEN_NO_RECALL|FILE_FLAG_OVERLAPPED
#define WIN_ATTRS  FILE_ATTRIBUTE_ARCHIVE|FILE_ATTRIBUTE_ENCRYPTED|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_NORMAL|FILE_ATTRIBUTE_NOT_CONTENT_INDEXED \
					FILE_ATTRIBUTE_OFFLINE|FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM|FILE_ATTRIBUTE_TEMPORARY| \
					FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_DEVICE|FILE_ATTRIBUTE_SPARSE_FILE|FILE_ATTRIBUTE_COMPRESSED
#define DCloseFile CloseHandle
#else
#include <sys/types.h>	//for off_t
#include <sys/stat.h>	//for modes
#include <sys/fcntl.h>  //for open
#include <unistd.h>     //for read write close
#define LINUX_FLAGS O_RDONLY|O_WRONLY|O_RDWR|O_CREAT|O_TRUNC|O_APPEND
#define LINUX_MODES S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IW_GRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH
//permission bits = mode & ~umask
#define DCloseFile close
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// DFile

DFile::DFile()
{
    m_hFile = D_INVALID_FILE;
}

DFile::DFile(DFileHandle fh)
{
    m_hFile = fh;
}

DFile::~DFile()
{
    if (m_hFile != D_INVALID_FILE)
    {
        DCloseFile(m_hFile);
    }
    m_hFile = D_INVALID_FILE;
}

DVoid DFile::Attach(DFileHandle fh)
{
    m_hFile = fh;
}

DFileHandle DFile::Detach()
{
    DFileHandle old = m_hFile;
    m_hFile = D_INVALID_FILE;
    return old;
}

DBool DFile::OpenFileRead(DCStr strPath)
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    std::string strPathA(strPath);
    m_hFile = CreateFileW((LPCWSTR)DXP::s2ws(strPathA).c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
#else
    m_hFile = open(strPath, O_RDONLY);
#endif
    if (m_hFile == D_INVALID_FILE)
    {
        return false;
    }
    return true;
}

DBool DFile::OpenFileWrite(DCStr strPath, DOpenFileMode nFlag)
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    std::string strPathA(strPath);
    m_hFile = CreateFileW((LPCWSTR)DXP::s2ws(strPathA).c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, nFlag, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
#else
    if (nFlag == DFILE_OPEN_ALWAYS)
    {
        m_hFile = open(strPath, O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);
    }
    else if (nFlag == DFILE_OPEN_EXISTING)
    {
        m_hFile = open(strPath, O_WRONLY);
    }
#endif
    if (m_hFile == D_INVALID_FILE)
    {
        return false;
    }
    return true;
}

DBool DFile::OpenFileRW(DCStr strPath, DOpenFileMode nFlag)
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    std::string strPathA(strPath);
    if (nFlag == DFILE_OPEN_ALWAYS)
    {
        //create if not exist
        m_hFile = CreateFileW((LPCWSTR)DXP::s2ws(strPathA).c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    }
    else if (nFlag == DFILE_OPEN_EXISTING)
    {
        //failed if not exist
        m_hFile = CreateFileW((LPCWSTR)DXP::s2ws(strPathA).c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    }
#else
    if (nFlag == DFILE_OPEN_ALWAYS)
    {
        m_hFile = open(strPath, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);
    }
    else if (nFlag == DFILE_OPEN_EXISTING)
    {
        m_hFile = open(strPath, O_RDWR, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);
    }
#endif
    if (m_hFile == D_INVALID_FILE)
    {
        return false;
    }
    return true;
}

DBuffer DFile::Read(DInt32 size, DInt32* result)
{
    DBuffer buf;
    if (m_hFile == D_INVALID_FILE)
    {
        return buf;
    }
    buf.Reserve((DInt32)size);
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    DUInt32 readsize = 0;
    BOOL bOK = ReadFile(m_hFile, buf.GetBuf(), size, (LPDWORD)&readsize, (LPOVERLAPPED)NULL);
    if (!bOK) return buf;
#else
    ssize_t sRead = read(m_hFile, buf.GetBuf(), size);
    if (result != NULL)
    {
        if (sRead == -1)
        {
            //Errors
            *result = -1;
        }
        else if (sRead == 0)
        {
            //End
            *result = 0;
        }
        else
        {
            *result = (DInt32)sRead;
        }
    }
#endif
    return buf;
}

DInt32 DFile::ReadTo(DByte* buf, DUInt32 bufsize)
{
    DUInt32 readsize = 0;
    if (m_hFile == D_INVALID_FILE)
    {
        return 0;
    }

#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    BOOL bOK = ReadFile(m_hFile, buf, bufsize, (LPDWORD)&readsize, (LPOVERLAPPED)NULL);
    if (!bOK) return 0;
#else
    ssize_t sRead = read(m_hFile, buf, bufsize);
    if (sRead == -1 || sRead == 0) return 0;
    readsize = (DInt32)sRead;
#endif
    return readsize;
}

DBool DFile::Write(DBuffer buf, DInt32* result)
{
    DBool bOK = false;
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        DUInt32 dwWrite = 0;
        bOK = WriteFile(m_hFile, buf.GetBuf(), buf.GetSize(), (LPDWORD)&dwWrite, NULL) ? true : false;
        bOK = (dwWrite == buf.GetSize());
    }
#else
    if (m_hFile != -1)
    {
        ssize_t dwWrite = write(m_hFile, buf.GetBuf(), buf.GetSize());
        bOK = (dwWrite == buf.GetSize());
        if (result != NULL)
        {
            if (dwWrite == -1)
            {
                //Error
                *result = -1;
            }
            else if (dwWrite == 0)
            {
                //Never
                *result = 0;
            }
            else
            {
                *result = (DInt32)dwWrite;
            }
        }
    }
#endif
    return bOK;
}


DBool DFile::SeekToEnd()
{
    return SetPos(0, DFILE_END);
}

DInt64 DFile::GetSize()
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    LARGE_INTEGER lpSize;
    lpSize.QuadPart = 0;
    if (GetFileSizeEx(m_hFile, &lpSize))
    {
        return lpSize.QuadPart;
    }
    return 0;
#else
    off_t oldpos = lseek(m_hFile, 0, SEEK_CUR);
    off_t len = lseek(m_hFile, 0, SEEK_END);
    lseek(m_hFile, oldpos, SEEK_SET);
    return len;
#endif
}

DInt64 DFile::GetPos()
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    LARGE_INTEGER pos;
    pos.QuadPart = 0;
    LARGE_INTEGER newpos;
    DBool bOK = SetFilePointerEx(m_hFile, pos, &newpos, FILE_CURRENT) ? true : false;
    if (bOK)
    {
        return newpos.QuadPart;
    }
    else
    {
        return 0;
    }
#else
    off_t oldpos = lseek(m_hFile, 0, SEEK_CUR);
    return (DInt64)oldpos;
#endif
}

DBool DFile::SetPos(DUInt64 pos, DFileMoveMethod dwMoveMethod)
{
    DBool bOK = false;
    if (m_hFile == D_INVALID_FILE)
        return bOK;
#if defined(BUILD_FOR_WINDOWS)
    LARGE_INTEGER distance;
    distance.QuadPart = pos;
    LARGE_INTEGER newpos;
    //Each handle has a pointer
    bOK = SetFilePointerEx(m_hFile, distance, &newpos, dwMoveMethod) ? true : false;
#else
    //TODO
#endif
    return bOK;
}

DBool DFile::SetEndOfFile()
{
#if defined(BUILD_FOR_WINDOWS)
    return ::SetEndOfFile(m_hFile) ? true : false;
#else
    //TODO
#endif
    return false;
}

DBool DFile::Flush()
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    return FlushFileBuffers(m_hFile) ? true : false;
#else
    //TODO
#endif
    return false;
}

DBuffer DFile::IOControl(DInt32 controlCode, DUInt32 sizeofOut, DBuffer& inbuf)
{
    DBuffer buf;
    buf.Reserve((DInt32)sizeofOut);
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    DUInt32 readsize = 0;
    DeviceIoControl(m_hFile, controlCode, buf.GetBuf(), buf.GetSize(), buf.GetBuf(), buf.GetSize(), (LPDWORD)&readsize, (LPOVERLAPPED)NULL);
#else
    //TODO File Cache Using fd
    int suc = 0;//setvbuf((FILE *)m_hFile,(char *)inbuf.GetBuf(), controlCode, sizeofOut);
    if (suc == 0)
    {

    }
    else
    {

    }
#endif
    return buf;
}

void DFile::Close()
{
    if (m_hFile != D_INVALID_FILE)
    {
        DCloseFile(m_hFile);
    }
    m_hFile = D_INVALID_FILE;
}


DBuffer DFile::FilePath2Buffer(DCStr strPath)
{
    DFile file;
    file.OpenFileRead(strPath);
    DBuffer readBuf = file.Read((DInt32)file.GetSize());
    return readBuf;
}
