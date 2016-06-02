#pragma once
#include <vector>
#include <map>

typedef unsigned int FileObjectHash;

class FileObject;
typedef std::vector<FileObject*> FileObjectArray;
typedef std::multimap<FileObjectHash, FileObject*> FileObjectMultiMap;

class FileObject 
{
public:
    enum ObjectType {
        TYPE_NONE = 0xFFFFFFFF,
        TYPE_DIR = 1,
        TYPE_FILE = 1 << 1,
    };

public:
    FileObject();
    FileObject(const CFileFind& ff);
//    FileObject(const FileObject& rhs);
    ~FileObject();

    void SetFileName(LPCTSTR str);
    void SetFullName(LPCTSTR str);

    void AddChild(FileObject* child);
    void Output();

    template <class Work>
    void Rounds(Work& work)
    {
        work(this);

        for (FileObjectArray::iterator it = _children.begin(); it != _children.end(); ++it) {
            (*it)->Rounds(work);
        }
    }

public:
    ObjectType _type;
    char _fileName[MAX_PATH];
    char _fullName[MAX_PATH];

    char _fileName_i[MAX_PATH];
    char _fullName_i[MAX_PATH];

    FileObject* _parent;
    FileObjectArray _children;
};

int MakeTree(const CStringArray& includePaths, const CStringArray& includeExts, FileObject* parent);
void SaveTree(LPCTSTR file, FileObject* parent);
int LoadTree(LPCTSTR file, FileObject* parent);

class IntegratedFileObject
{
public:
    class FileData
    {
    public:
        UINT SetName(LPCTSTR str, UINT len)
        {
            nameLength = len;
            memcpy(name, str, len + 1);
            return CalcByteSize(len);
        }

        FileData* Next() { return reinterpret_cast<FileData*>(reinterpret_cast<BYTE*>(this) + CalcByteSize(nameLength)); }
        LPCTSTR Name() const { return name; }
        LPTSTR Name() { return name; }
        UINT NameLength() const { return nameLength; }

    private:
        UINT nameLength;
        char name[0];

    public:
        static UINT CalcByteSize(UINT nameLength) { return nameLength + 1 + sizeof(FileData); }
    };

public:
    IntegratedFileObject();
    ~IntegratedFileObject();

    void Add(const CString& pathName);
    void Find(LPCTSTR name, CStringArray* nameList);
    void FindOffsets(LPCTSTR name, CDWordArray* offsetList);
    void NameFromOffsets(const CDWordArray& offsetList, CStringArray* nameList);

    template <class Work> void Rounds(Work& work)
    {
        FileData* fileData = (FileData*)_buffer;

        while (fileData->NameLength()) {
            work(fileData->Name());
            fileData = fileData->Next();
        }
    }

    bool Load(LPCTSTR file);
    bool Save(LPCTSTR file);

    IntegratedFileObject* Clone();

private:
    BYTE* _buffer;
    UINT _size;
    UINT _offset;
};

int MakeFileObject(const CStringArray& includePaths, const CStringArray& includeExts, const CStringArray& excludePaths, IntegratedFileObject* fullFile, IntegratedFileObject* fullDir);