#include "stdafx.h"
#include "FileObject.h"

FileObject::FileObject() : _type(TYPE_NONE) 
{
    _fileName[0] = 0;
    _fullName[0] = 0;
    _fileName_i[0] = 0;
    _fullName_i[0] = 0;
}

FileObject::FileObject(const CFileFind& ff)
    : _type(ff.IsDirectory() ? TYPE_DIR : TYPE_FILE) 
    , _parent(NULL)
{
    SetFileName((LPCTSTR)ff.GetFileName());
    SetFullName((LPCTSTR)ff.GetFilePath());
}

//FileObject::FileObject(const FileObject& rhs)
//{
//    this->_type = rhs._type;
//    _tcscpy(this->_fileName, rhs._fileName);
//    _tcscpy(this->_fullName, rhs._fullName);
//}

FileObject::~FileObject()
{
    for (FileObjectArray::iterator it = _children.begin(); it != _children.end(); ++it) {
        delete *it;
    }
}

void FileObject::SetFileName(LPCTSTR str)
{
    _tcscpy(_fileName, str);
    _tcscpy(_fileName_i, str);
    strupr(_fileName_i);
}

void FileObject::SetFullName(LPCTSTR str)
{
    _tcscpy(_fullName, str);
    _tcscpy(_fullName_i, str);
    strupr(_fullName_i);
}

void FileObject::AddChild(FileObject* child) 
{ 
    _children.push_back(child); 
    child->_parent = this;
}

void FileObject::Output()
{
    if (_type != TYPE_NONE) {
        _cprintf("%s : %s\n", _type == TYPE_DIR ? "dir" : "file", _fileName);
    }

    for (FileObjectArray::iterator it = _children.begin(); it != _children.end(); ++it) {
        (*it)->Output();
    }
}

//template <class Work>
//void FileObject::Rounds(Work work)
//{
//    work(this);
//
//    for (FileObjectArray::iterator it = _children.begin(); it != _children.end(); ++it) {
//        (*it)->Rounds(work);
//    }
//}

int LoadImple(FILE* fp, FileObject* object)
{
    int total = 1;

    fread(&object->_type, sizeof(object->_type), 1, fp);

    int count;

    fread(&count, sizeof(count), 1, fp);
    fread(object->_fileName, count, 1, fp);
    _tcscpy(object->_fileName_i, object->_fileName);
    strupr(object->_fileName_i);

    fread(&count, sizeof(count), 1, fp);
    fread(object->_fullName, count, 1, fp);
    _tcscpy(object->_fullName_i, object->_fullName);
    strupr(object->_fullName_i);

    int childrenCount;
    fread(&childrenCount, sizeof(childrenCount), 1, fp);

    while(childrenCount--) {
        FileObject* child = new FileObject;
        object->AddChild(child);
        total += LoadImple(fp, child);
    }

    return total;
}

int LoadTree(LPCTSTR file, FileObject* parent)
{
    FILE* fp = fopen(file, "rb");
    if (!fp) {
        return FALSE;
    }

    int count = LoadImple(fp, parent);
    fclose(fp);

    return count - 1;
}

void SaveTree(LPCTSTR file, FileObject* parent)
{
    FILE* fp = fopen(file, "wb");
    if (!fp) {
        return;
    }

    class FileObjectWriter
    {
    public:
        FileObjectWriter(FILE* fp) : _fp(fp) {}
        void operator() (FileObject* object)
        {
            fwrite(&object->_type, sizeof(object->_type), 1, _fp);

            int count;

            count = _tcslen(object->_fileName) + 1;
            fwrite(&count, sizeof(count), 1, _fp);
            fwrite(object->_fileName, count, 1, _fp);

            count = _tcslen(object->_fullName) + 1;
            fwrite(&count, sizeof(count), 1, _fp);
            fwrite(object->_fullName, count, 1, _fp);

            count = (int)object->_children.size();
            fwrite(&count, sizeof(count), 1, _fp);
        }

    private:
        FILE* _fp;
    };

    FileObjectWriter writer(fp);
    parent->Rounds(writer);

    fclose(fp);
}

static BOOL CheckExt(const CStringArray& exts, const CString& filePath)
{
    LPCTSTR ext = PathFindExtension((LPCTSTR)filePath);
    if (*ext == 0) {
        return TRUE;
    }

    ++ext;
    int count = exts.GetSize();
    for (int i = 0; i < count; ++i) {
        if (exts[i].CompareNoCase(ext) == 0) {
            return TRUE;
        }
    }

    return FALSE;
}

static BOOL CheckPath(const CStringArray& paths, const CString& filePath)
{
    int count = paths.GetCount();
    for (int i = 0; i < count; ++i) {
        if (paths[i].CompareNoCase(filePath) == 0) {
            return TRUE;
        }
    }

    return FALSE;
}

static int MakeTreeImpl(LPCTSTR curDir, const CStringArray& includeExts, FileObject* parent)
{
    int count = 0;
    CFileFind ff;
    std::string str = curDir;
    str += "\\*";

    BOOL next = ff.FindFile(str.c_str());

    while (next) {
        next = ff.FindNextFile();

        if (ff.IsDots())
            continue;

        if (!CheckExt(includeExts, ff.GetFileName()))
            continue;

        FileObject* object = new FileObject(ff);
        if (ff.IsDirectory()) {
            count += MakeTreeImpl((LPCTSTR)ff.GetFilePath(), includeExts, object);
        }

        parent->AddChild(object);
        ++count;
    };

    return count;
}

int MakeTree(const CStringArray& includePaths, const CStringArray& includeExts, FileObject* parent)
{
    int count = 0;
    for (int i = 0; i < includePaths.GetSize(); ++i) {
        const CString& currentPath = includePaths[i];
        FileObject* object = new FileObject;
        object->_type = FileObject::TYPE_DIR;
        object->SetFileName(PathFindFileName((LPCTSTR)currentPath));
        object->SetFullName((LPCTSTR)currentPath);
        parent->AddChild(object);

        count += MakeTreeImpl(currentPath, includeExts, object);
    }

    return count;
}

IntegratedFileObject::IntegratedFileObject()
: _buffer(new BYTE[1]), _size(1), _offset(0)
{
}

IntegratedFileObject::~IntegratedFileObject()
{
    delete [] _buffer;
    _size = 0;
    _offset = 0;
}

void IntegratedFileObject::Add(const CString& pathName)
{
    int stringLen = pathName.GetLength();
    int byteSize = FileData::CalcByteSize(stringLen);
    if (byteSize > (_size - _offset)) {
        _size *= 2;
        _size += byteSize;
        BYTE* newBuffer = new BYTE[_size];
        memcpy(newBuffer, _buffer, _offset);
        delete [] _buffer;
        _buffer = newBuffer;
    }

    FileData* fileData = (FileData *)(&_buffer[_offset]);
    _offset += fileData->SetName((LPCTSTR)pathName, stringLen);
}

void IntegratedFileObject::Find(LPCTSTR name, CStringArray* nameList)
{
    FileData* fileData = (FileData*)_buffer;
    
    int count = 250;

    while (fileData->NameLength() && count) {
        if (strstr(fileData->Name(), name)) {
            nameList->Add(fileData->Name());
            --count;
        }
        fileData = fileData->Next();
    }
}

void IntegratedFileObject::FindOffsets(LPCTSTR name, CDWordArray* offsetList)
{
    FileData* fileData = (FileData*)_buffer;

    int count = 250;

    while (fileData->NameLength() && count) {
        if (strstr(fileData->Name(), name)) {
            offsetList->Add((LPBYTE)fileData->Name() - _buffer);
            --count;
        }
        fileData = fileData->Next();
    }
}

void IntegratedFileObject::NameFromOffsets(const CDWordArray& offsetList, CStringArray* nameList)
{
    int count = offsetList.GetSize();
    for (int i = 0; i < count; ++i) {
        DWORD offset = offsetList.GetAt(i);
        nameList->Add((LPCTSTR)(&_buffer[offset]));
    }
}

bool IntegratedFileObject::Load(LPCTSTR file)
{
    FILE* fp = fopen(file, "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        _size = ftell(fp);
        _offset = _size;

        delete [] _buffer;
        _buffer = new BYTE[_size];
        fseek(fp, -(_size), SEEK_END);
        fread(_buffer, _size, 1, fp);
        fclose(fp);
        return true;
    }

    return false;
}

bool IntegratedFileObject::Save(LPCTSTR file)
{
    FILE* fp = fopen(file, "wb");
    if (fp) {
        fwrite(_buffer, _offset, 1, fp);
        fclose(fp);
        return true;
    }

    return false;
}

IntegratedFileObject* IntegratedFileObject::Clone()
{
    IntegratedFileObject* copy = new IntegratedFileObject;
    copy->_size = _size;
    copy->_offset = _offset;
    copy->_buffer = new BYTE[_size];
    memcpy(copy->_buffer, _buffer, _offset);

    return copy;
}

static int MakeFileObjectImpl(LPCTSTR curDir, const CStringArray& includeExts, const CStringArray& excludePaths, IntegratedFileObject* fullFile, IntegratedFileObject* fullDir)
{
    int count = 0;
    CFileFind ff;
    std::string str = curDir;
    str += "\\*";

    BOOL next = ff.FindFile(str.c_str());

    CStringArray childDirs;

    while (next) {
        next = ff.FindNextFile();

        if (ff.IsDots())
            continue;
        
        IntegratedFileObject* fullObj;

        if (ff.IsDirectory()) {
            if (CheckPath(excludePaths, ff.GetFilePath())) {
                continue;;
            }

            fullObj = fullDir;
            childDirs.Add(ff.GetFilePath());
        } else {
            if (!CheckExt(includeExts, ff.GetFilePath()))
                continue;

            fullObj = fullFile;
        }

        fullObj->Add(ff.GetFilePath());
        ++count;
    };

    int children = childDirs.GetCount();
    for (int i = 0; i < children; ++i) {
        count += MakeFileObjectImpl((LPCTSTR)childDirs[i], includeExts, excludePaths, fullFile, fullDir);
    }

    return count;
}

int MakeFileObject(const CStringArray& includePaths, const CStringArray& includeExts, const CStringArray& excludePaths, IntegratedFileObject* fullFile, IntegratedFileObject* fullDir)
{
    int count = 0;
    for (int i = 0; i < includePaths.GetSize(); ++i) {
        const CString& currentPath = includePaths[i];
        fullDir->Add(currentPath);
        ++count;
        count += MakeFileObjectImpl(currentPath, includeExts, excludePaths, fullFile, fullDir);
    }

    fullFile->Add(""); 
    fullDir->Add(""); 

    return count;
}
