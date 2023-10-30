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

#ifndef GX_GFILE_H
#define GX_GFILE_H

#include "gobject.h"
#include "gx/gstring.h"
#include "gbytearray.h"

#include <vector>
#include <functional>


GX_NS_BEGIN

#define FILE_SEPARATOR  "/"

/**
 * @class
 * @brief
 * 1 Provide file operations: information acquisition, continuous read and write, random read and write, create, delete, rename;
 * 2 Provide directory operations such as creating, deleting, renaming, and listing files
 */
class GX_API GFile final : public GObject
{
public:
    using FileFilter = std::function<bool(const GFile &)>;

    enum OpenMode : uint8_t
    {
        ReadOnly = 0x01,                    ///< Open in read-only mode
        WriteOnly = 0x02,                   ///< Open in write only mode
        Append = 0x04,                      ///< Open in append mode

        ReadWrite = ReadOnly | WriteOnly,   ///< Open in read-write mode

        Binary = 0x08                       ///< Open the file in binary format, default to text format, the difference lies in the way the end of the file is judged
    };
    typedef uint8_t OpenModeFlags;

public:
    explicit GFile(const std::string &path = "");

    explicit GFile(const GFile &parent, const std::string &child);

    GFile(const GFile &other);

    GFile(GFile &&other) noexcept;

    ~GFile();

    GFile &operator=(const GFile &other) = delete;

    GFile &operator=(GFile &&other) noexcept;

public:
    void setFile(const std::string &path);

    void setFile(const GFile &parent, const std::string &child);

    void swap(GFile &file);

    std::string toString() const override;

public:
    bool exists() const;

    bool isFile() const;

    bool isDirectory() const;

    /**
     * @brief Is it a relative path
     *
     * @return
     */
    bool isRelative() const;

    bool isRoot() const;

    bool isReadOnly() const;

    bool setReadOnly(bool isReadOnly);

    time_t lastModified();

    time_t lastAccess();

    std::string filePath() const;

    std::string fileName() const;

    /**
     * @brief Returns a file name without an extension.
     * @return
     */
    std::string fileNameWithoutExtension() const;

    std::string fileSuffix() const;

    std::string absoluteFilePath() const;

    std::string absolutePath() const;

    /**
     * @brief Get Parent Directory.
     * @return Return absolute path
     */
    GFile parent() const;

    /**
     * @brief When deleting files or directories, the directory must be empty.
     * @return Return failure or success
     */
    bool remove();

    bool rename(GFile &newFile);

    GFile concat(const GFile &b);

    GFile concat(const std::string &b);

    friend GFile operator+(const GFile &a, const GFile &b)
    {
        GFile temp(a);
        return temp.concat(b);
    }

    friend GFile operator+(const GFile &a, const std::string &b)
    {
        GFile temp(a);
        return temp.concat(b);
    }

    friend GFile operator/(const GFile &a, const GFile &b)
    {
        return a + b;
    }

    friend GFile operator/(const GFile &a, const std::string &b)
    {
        return a + b;
    }

public:
    std::vector<GFile> listFiles(const FileFilter &filter = nullptr) const;

    bool mkdir() const;

    bool mkdirs() const;

public:
    /**
     * @brief Open the file, errors will be written to errno
     * @param mode
     * @return Success or Failure
     */
    bool open(OpenModeFlags mode);

    /**
     * @brief Open File
     * @param mode  C-style opening mode (r/w/r+/w+/b/t)
     * @return
     */
    bool open(const std::string &mode);

    bool flush();

    /**
     * @brief Close File, errors will be written to errno
     */
    void close();

    /**
     * @brief Has the file been opened
     * @return
     */
    bool isOpen();

    /**
     * @brief File size in bytes
     * @return
     */
    int64_t fileSize() const;

    /**
     * @brief Read file
     * @param buffer
     * @param elementSize
     * @param count
     * @return
     */
    int64_t read(void *buffer, int64_t elementSize, int64_t count);

    /**
     * @brief Read file
     * @param buffer
     * @param maxSize
     * @return
     */
    int64_t read(char *buffer, int64_t maxSize);

    GByteArray read(int64_t maxSize = -1);

    GString readAll();

    GString readLine();

    bool atEnd();

    /**
     * @brief Writing file
     * @param buffer
     * @param elementSize
     * @param count
     * @return
     */
    int64_t write(const void *buffer, int64_t elementSize, int64_t count);

    /**
     * @brief Writing file
     * @param buffer
     * @param size
     * @return
     */
    int64_t write(const char *buffer, int64_t size);

    int64_t write(const GByteArray &data);

    int64_t write(const GString &str);

    /**
     * @brief Move file pointer position
     * @param offset Offset (in bytes), positive number moved back, negative number moved forward
     * @param origin Start position of offset: SEEK_ SET (from file header), SEEK_ CUR (from current position), SEEK_ END (from the end of the file)
     * @return Is it successful? If it fails, the file pointer position will not change
     */
    bool seek(int64_t offset, int origin);

    /**
     * @brief Starting from SEEK CUR, move the file pointer
     * @param offset Offset (in bytes), positive number moved back, negative number moved forward
     * @return Is it successful? If it fails, the file pointer position will not change
     */
    bool seek(int64_t offset);

public:
    static GString formatPath(GString path);

    /**
     * @brief Obtain the main program file directory
     * @return
     */
    static GFile mainDirectory();

    static GFile temporaryDirectory();

private:
    static bool exists(const GString &path);

    static bool isFile(const GString &path);

    static bool isDirectory(const GString &path);

    static bool isReadOnly(const GString &path);

    static bool setReadOnly(const GString &path, bool isReadOnly);

    static bool mkdir(const GString &path);

    static std::string transOpenMode(OpenModeFlags mode);

private:
    FILE *mFilePtr = nullptr;
    GString mPath;
};

GX_NS_END

#endif // GX_GFILE_H
