/*
 * Copyright (c) 2020 Gxin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "gx/gfile.h"

#include <sys/stat.h>
#include <fstream>


#if GX_PLATFORM_WINDOWS

#include <windows.h>


#else
#include <dirent.h>
#endif

#if !(GX_COMPILER_MSVC)

#include <cerrno>

#endif

#if GX_PLATFORM_LINUX || GX_PLATFORM_BSD

#include <unistd.h>

#elif GX_PLATFORM_OSX || GX_GX_PLATFORM_IOS

#include <mach-o/dyld.h>

#endif

#define MAX_ABS_PATH 2048


GX_NS_BEGIN

GFile::GFile(const std::string &path)
{
    mPath = formatPath(path);
}

GFile::GFile(const GFile &parent, const std::string &child)
{
    mPath = parent.mPath;
    mPath.append(FILE_SEPARATOR).append(child);
    mPath = formatPath(mPath);
}

GFile::GFile(const GFile &other)
{
    mPath = formatPath(other.mPath);
}

GFile::GFile(GFile &&other) noexcept
{
    swap(other);
}

GFile::~GFile()
{
    close();
}

GFile &GFile::operator=(GFile &&other) noexcept
{
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void GFile::setFile(const std::string &path)
{
    GFile f(path);
    this->swap(f);
}

void GFile::setFile(const GFile &parent, const std::string &child)
{
    GFile f(parent, child);
    this->swap(f);
}

void GFile::swap(GFile &file)
{
    std::swap(this->mFilePtr, file.mFilePtr);
    mPath.swap(file.mPath);
}

std::string GFile::toString() const
{
    return absoluteFilePath();
}

bool GFile::exists() const
{
    return GFile::exists(mPath);
}

bool GFile::isFile() const
{
    return GFile::isFile(mPath);
}

bool GFile::isDirectory() const
{
    return GFile::isDirectory(mPath);
}

bool GFile::isRelative() const
{
#if GX_PLATFORM_WINDOWS || GX_PLATFORM_WINRT
    return mPath.at(1) != ":";
#else
    return !mPath.startWith(FILE_SEPARATOR);
#endif
}

bool GFile::isRoot() const
{
    if (!isDirectory()) {
        return false;
    }
#if GX_PLATFORM_WINDOWS || GX_PLATFORM_WINRT
    return mPath.length() == 3 && mPath.at(1) == ":";
#else
    return mPath.length() == 1 && mPath.at(0) == FILE_SEPARATOR;
#endif
}

bool GFile::isReadOnly() const
{
    return GFile::isReadOnly(mPath);
}

bool GFile::setReadOnly(bool isReadOnly)
{
    return GFile::setReadOnly(mPath, isReadOnly);
}

time_t GFile::lastModified()
{
    if (!exists()) {
        return 0;
    }
#if GX_PLATFORM_WINDOWS
    struct _stat fstat{};
    _wstat(mPath.toUtf16().data(), &fstat);
    return fstat.st_mtime;
#else
    struct stat fstat{};
    stat(mPath.c_str(), &fstat);
    return fstat.st_mtime;
#endif
}

time_t GFile::lastAccess()
{
    if (!exists()) {
        return 0;
    }
#if GX_PLATFORM_WINDOWS
    struct _stat fstat{};
    _wstat(mPath.toUtf16().data(), &fstat);
    return fstat.st_atime;
#else
    struct stat fstat{};
    stat(mPath.c_str(), &fstat);
    return fstat.st_atime;
#endif
}

std::string GFile::filePath() const
{
    return this->mPath.toStdString();
}

std::string GFile::fileName() const
{
    if (isRoot()) {
        return this->mPath.toStdString();
    }
    int lastSepIndex = this->mPath.lastIndexOf(FILE_SEPARATOR);
    if (lastSepIndex < 0) {
        return this->mPath.toStdString();
    }
    return this->mPath.substring(lastSepIndex + 1).toStdString();
}

std::string GFile::fileNameWithoutExtension() const
{
    GString fileName = this->fileName();
    int32_t findDot = fileName.lastIndexOf(".");
    if (findDot > 0) {
        return fileName.substring(0, findDot).toStdString();
    }
    return fileName.toStdString();
}

std::string GFile::fileSuffix() const
{
    if (isDirectory()) {
        return "";
    }
    GString fileName = this->fileName();

    int32_t findDot = fileName.lastIndexOf(".");
    int32_t findSep = fileName.lastIndexOf(FILE_SEPARATOR);
    if (findDot > findSep) {
        GString suffix = fileName.substring(findDot + 1, -1);
        return suffix.toStdString();
    }
    return "";
}

std::string GFile::absoluteFilePath() const
{
    if (mPath.isEmpty()) {
        return "";
    }
    GString absString;
#if GX_PLATFORM_WINDOWS || GX_PLATFORM_WINRT
    wchar_t wAbsPath[MAX_ABS_PATH];
    _wfullpath(wAbsPath, mPath.toUtf16().data(), MAX_ABS_PATH);
    absString = GString(GWString(wAbsPath));
#elif GX_PLATFORM_POSIX
    char absPath[PATH_MAX];
    if (realpath(mPath.c_str(), absPath)) {
        absString = absPath;
    } else {
        absString = mPath;
    }
#else
    #error "Unsupported platform!!"
#endif
    return formatPath(absString).toStdString();
}

std::string GFile::absolutePath() const
{
    GString absFilePath = this->absoluteFilePath();
    if (isDirectory()) {
        return absFilePath.toStdString();
    }
    int lastSepIndex = absFilePath.lastIndexOf(FILE_SEPARATOR);
    return absFilePath.left(lastSepIndex).toStdString();
}

GFile GFile::parent() const
{
    if (mPath.isEmpty()) {
        return GFile();
    }
    GString absPath = this->absolutePath();
    if (!isDirectory() || isRoot()) {
        return GFile(absPath.toStdString());
    }
    int lastSepIndex = absPath.lastIndexOf(FILE_SEPARATOR);
    return GFile(absPath.left(lastSepIndex).toStdString());
}

bool GFile::remove()
{
    if (!exists()) {
        return false;
    }
    errno = 0;
#if GX_PLATFORM_WINDOWS | GX_PLATFORM_WINRT
    if (isDirectory()) {
        return _wrmdir(mPath.toUtf16().data()) == 0;
    } else {
        return _wremove(mPath.toUtf16().data()) == 0;
    }
#else
    return ::remove(mPath.c_str()) == 0;
#endif
}

bool GFile::rename(GFile &newFile)
{
    if (!exists()) {
        return false;
    }
    if (newFile.exists()) {
        return false;
    }
    errno = 0;
#if GX_PLATFORM_WINDOWS | GX_PLATFORM_WINRT
    return _wrename(mPath.toUtf16().data(), newFile.mPath.toUtf16().data()) == 0;
#else
    return ::rename(mPath.c_str(), newFile.mPath.c_str()) == 0;
#endif
}

GFile GFile::concat(const GFile &b)
{
    if (!b.isRelative()) {
        return b;
    }
    if (b.mPath.isEmpty()) {
        return *this;
    }
    return GFile(*this, b.mPath.toStdString());
}

GFile GFile::concat(const std::string &b)
{
    return concat(GFile(b));
}

std::vector<GFile> GFile::listFiles(const FileFilter &filter) const
{
    if (!exists()) {
        return {};
    }
    std::vector<GFile> files;
    if (isDirectory()) {
#if GX_PLATFORM_WINDOWS
        WIN32_FIND_DATAW findData;
        GString strFindFile = absoluteFilePath() + FILE_SEPARATOR + "*";
        HANDLE hFind = FindFirstFileW(strFindFile.toUtf16().data(), &findData);
        if (hFind == INVALID_HANDLE_VALUE) {
            return {};
        }
        wchar_t wstr[MAX_PATH];
        do {
            memcpy(wstr, findData.cFileName, sizeof(wchar_t) * MAX_PATH);
            GString cFileName(wstr);
            if (cFileName == "." || cFileName == "..") {
                continue;
            }
            GFile nf(*this, cFileName.toStdString());
            if (!filter || filter(nf)) {
                files.emplace_back(nf);
            }
        } while (FindNextFileW(hFind, &findData));
        FindClose(hFind);
#else
        DIR *dir;
        struct dirent *_dirent;
        dir = opendir(filePath().c_str());
        if (!dir) {
            return files;
        } else {
            while ((_dirent = readdir(dir)) != nullptr) {
                GString name = _dirent->d_name;
                if (name != "." && name != "..") {
                    GFile nf(*this, name.toStdString());
                    if (!filter || filter(nf)) {
                        files.emplace_back(nf);
                    }
                }
            }
            closedir(dir);
        }
#endif
    }
    return files;
}

bool GFile::mkdir() const
{
    if (mPath.isEmpty()) {
        return false;
    }
    return GFile::mkdir(mPath);
}

bool GFile::mkdirs() const
{
    if (mPath.isEmpty()) {
        return false;
    }
    int begin = 0;
    while (begin >= 0) {
        int p = mPath.indexOf(FILE_SEPARATOR, begin);
        if (p < 0) {
            break;
        }
        GString pPath = mPath.left(p);

        if (!exists(pPath)) {
            mkdir(pPath);
        } else if (exists(pPath) && !isDirectory(pPath)) {
            return false;
        }

        begin = p + 1;
    }
    if (begin < mPath.length()) {
        return mkdir(mPath);
    }
    return exists() && isDirectory();
}

bool GFile::open(GFile::OpenModeFlags mode)
{
    std::string cMode = transOpenMode(mode);
    return open(cMode);
}

bool GFile::open(const std::string &mode)
{
    if (mFilePtr) {
        this->close();
    }
    if (mPath.isEmpty()) {
        return false;
    }
    errno = 0;

#if GX_PLATFORM_WINDOWS
    GString cMode(mode);
    _wfopen_s(&mFilePtr, this->mPath.toUtf16().data(), cMode.toUtf16().data());
#else
    mFilePtr = ::fopen(this->mPath.c_str(), mode.c_str());
#endif
    return mFilePtr != nullptr;
}

bool GFile::flush()
{
    if (mFilePtr) {
        errno = 0;
        return fflush(mFilePtr) == 0;
    }
    return false;
}

void GFile::close()
{
    if (mFilePtr) {
        errno = 0;
        ::fclose(mFilePtr);
        mFilePtr = nullptr;
    }
}

bool GFile::isOpen()
{
    return mFilePtr != nullptr;
}

int64_t GFile::fileSize() const
{
    if (!isFile()) {
        return 0;
    }
#if GX_PLATFORM_WINDOWS
    struct _stat fstat{};
    _wstat(mPath.toUtf16().data(), &fstat);
    return fstat.st_size;
#else
    struct stat fstat{};
    stat(mPath.c_str(), &fstat);
    return fstat.st_size;
#endif
}

int64_t GFile::read(void *buffer, int64_t elementSize, int64_t count)
{
    if (mFilePtr) {
        errno = 0;
        return ::fread(buffer, elementSize, count, mFilePtr);
    }
    return -1;
}

int64_t GFile::read(char *buffer, int64_t maxCount)
{
    return this->read(buffer, sizeof(char), maxCount);
}

GByteArray GFile::read(int64_t maxSize)
{
    if (!isOpen()) {
        return GByteArray();
    }
    maxSize = maxSize > 0 ? maxSize : fileSize();
    GByteArray buffer(maxSize);
    maxSize = read((char *) buffer.ptr(), (int64_t) maxSize);
    buffer.seekWritePos(SEEK_SET, (int32_t)maxSize);
    return buffer;
}

GString GFile::readAll()
{
    seek(0, SEEK_SET);

    GByteArray all = read();
    GString str((const char *)all.data(), all.size());

    return str;
}

GString GFile::readLine()
{
    int64_t bufferSize = 1024;
    char *buffer = (char *) malloc(bufferSize);

    int64_t len = 0;
    char c;
    while ((c = fgetc(mFilePtr)) != EOF && c != '\0' && c != '\n') {
        if (len >= bufferSize) {
            bufferSize = len + 64;
            buffer = (char *) realloc(buffer, bufferSize);
        }
        buffer[len++] = c;
    }

    buffer[len] = '\0';

    GString str(buffer);
    free(buffer);
    return str;
}

bool GFile::atEnd()
{
    return feof(mFilePtr) != 0;
}

int64_t GFile::write(const void *buffer, int64_t elementSize, int64_t count)
{
    if (mFilePtr) {
        errno = 0;
        return ::fwrite(buffer, elementSize, count, mFilePtr);
    }
    return -1;
}

int64_t GFile::write(const char *buffer, int64_t size)
{
    return write(buffer, sizeof(char), size);
}

int64_t GFile::write(const GByteArray &data)
{
    return write((const char *) data.data(), (int64_t) data.size());
}

int64_t GFile::write(const GString &str)
{
    return write(str.data(), str.count());
}

bool GFile::seek(int64_t offset, int origin)
{
    if (mFilePtr) {
        errno = 0;
        return ::fseek(mFilePtr, offset, origin) == 0;
    }
    return false;
}

bool GFile::seek(int64_t offset)
{
    return this->seek(offset, SEEK_CUR);
}

bool GFile::exists(const GString &path)
{
    if (path.isEmpty()) {
        return false;
    }
#if GX_PLATFORM_WINDOWS
    return _waccess(path.toUtf16().data(), 0) == 0;
#else
    struct stat fstat{};
    return stat(path.c_str(), &fstat) == 0;
#endif
}

bool GFile::isFile(const GString &path)
{
    if (path.isEmpty()) {
        return false;
    }
#if GX_PLATFORM_WINDOWS
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(path.toUtf16().data(), &findData);

    if (hFind != INVALID_HANDLE_VALUE && (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
        return true;
    }
    return false;
#else
    struct stat fstat{};
    stat(path.c_str(), &fstat);
    return S_ISREG(fstat.st_mode);
#endif
}

bool GFile::isDirectory(const GString &path)
{
    if (path.isEmpty()) {
        return false;
    }
#if GX_PLATFORM_WINDOWS
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(path.toUtf16().data(), &findData);

    if (hFind != INVALID_HANDLE_VALUE && (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        return true;
    }
    return false;
#else
    struct stat fstat{};
    stat(path.c_str(), &fstat);
    return S_ISDIR(fstat.st_mode);
#endif
}

bool GFile::isReadOnly(const GString &path)
{
    if (path.isEmpty()) {
        return false;
    }
#if GX_PLATFORM_WINDOWS
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(path.toUtf16().data(), &findData);

    if (hFind != INVALID_HANDLE_VALUE && (findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)) {
        return true;
    }
    return false;
#else
    struct stat fstat{};
    if (stat(path.c_str(), &fstat) != -1) {
        return (fstat.st_mode & S_IWRITE) != S_IWRITE && (fstat.st_mode & S_IREAD) == S_IREAD;
    }
    return false;
#endif
}

bool GFile::setReadOnly(const GString &path, bool isReadOnly)
{
    if (path.isEmpty()) {
        return false;
    }
#if GX_PLATFORM_WINDOWS
    return SetFileAttributesW(path.toUtf16().data(), isReadOnly ? FILE_ATTRIBUTE_READONLY : FILE_ATTRIBUTE_NORMAL) == 0;
#else
    struct stat fstat{};
    if (stat(path.c_str(), &fstat) != -1) {
        if (isReadOnly) {
            fstat.st_mode &= ~S_IWUSR;
        } else {
            fstat.st_mode |= S_IWUSR;
        }
        return chmod(path.c_str(), fstat.st_mode) == 0;
    }
    return false;
#endif
}

bool GFile::mkdir(const GString &path)
{
    if (path.isEmpty()) {
        return false;
    }
    errno = 0;
#if GX_PLATFORM_WINDOWS || GX_PLATFORM_WINRT
    return ::_wmkdir(path.toUtf16().data()) == 0;
#else
    return ::mkdir(path.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH) == 0;
#endif
}

std::string GFile::transOpenMode(GFile::OpenModeFlags mode)
{
    bool isBinary = (mode & GFile::OpenMode::Binary) == GFile::OpenMode::Binary;
    bool isAppend = (mode & GFile::OpenMode::Append) == GFile::OpenMode::Append;
    if ((mode & GFile::OpenMode::ReadWrite) == GFile::OpenMode::ReadWrite) {
        if (isAppend) {
            return isBinary ? "ab+" : "at+";
        } else {
            return isBinary ? "wb+" : "wt+";
        }
    } else if ((mode & GFile::OpenMode::ReadOnly) == GFile::OpenMode::ReadOnly) {
        return isBinary ? "rb" : "rt";
    } else if ((mode & GFile::OpenMode::WriteOnly) == GFile::OpenMode::WriteOnly) {
        if (isAppend) {
            return isBinary ? "ab" : "at";
        } else {
            return isBinary ? "wb" : "wt";
        }
    }
    return isBinary ? "rb" : "rt";
}


GString GFile::formatPath(GString path)
{
    if (path.isEmpty()) {
        return "";
    }
    path = path.replace("\\", FILE_SEPARATOR);
    if (path.endWith(FILE_SEPARATOR)) {
        path = path.left(path.length() - 1);
#if GX_PLATFORM_WINDOWS || GX_PLATFORM_WINRT
        if (path.endWith(":")) {
            path += FILE_SEPARATOR;
        }
#else
        if (path.isEmpty()) {
            path += FILE_SEPARATOR;
        }
#endif
    }
#if GX_PLATFORM_WINDOWS || GX_PLATFORM_WINRT
    if (path.length() == 2 && path.endWith(":")) {
        path += FILE_SEPARATOR;
    }
#endif
    return path;
}


#if !GX_PLATFORM_OSX && !GX_PLATFORM_IOS

GFile GFile::mainDirectory()
{
#if GX_PLATFORM_WINDOWS
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    GString path(buffer);
    GFile file(path.toStdString());
    return file.parent();
#elif GX_PLATFORM_LINUX || GX_PLATFORM_ANDROID
    char buffer[PATH_MAX];
    readlink("/proc/self/exe", buffer, PATH_MAX);
    GFile file(buffer);
    return file.parent();
#elif GX_PLATFORM_EMSCRIPTEN
    return GFile("/");
#else
    #error "Unsupported platform!!"
#endif
}

GFile GFile::temporaryDirectory()
{
#if GX_PLATFORM_WINDOWS
    wchar_t buffer[MAX_PATH];
    GetTempPathW(MAX_PATH, buffer);
    GString path(buffer);
    return GFile(path.toStdString());
#elif GX_PLATFORM_LINUX || GX_PLATFORM_ANDROID
    return GFile("/tmp/");
#elif GX_PLATFORM_EMSCRIPTEN
    return GFile("./tmp/");
#else
    #error "Unsupported platform!!"
#endif
}

#endif

GX_NS_END
