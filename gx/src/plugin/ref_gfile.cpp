/*
 * Copyright (c) 2022 Gxin
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

#include "ref_gx.h"

#include "gx/gfile.h"


GX_NS_BEGIN

void refGFile()
{
    Class<GFile>("Gx", "GFile", "Gx file.")
            .construct<>()
            .construct<const std::string &>("Construct from file path.")
            .construct<const GFile &, const std::string &>("Construct from file path and child path.")
            .construct<const GFile &>("Construct from file.")
            .inherit<GObject>()
            .defEnum("OpenMode", {
                    {"ReadOnly",  (int32_t) GFile::OpenMode::ReadOnly},
                    {"WriteOnly", (int32_t) GFile::OpenMode::WriteOnly},
                    {"ReadWrite", (int32_t) GFile::OpenMode::ReadWrite},
                    {"Append",    (int32_t) GFile::OpenMode::Append},
                    {"Binary",    (int32_t) GFile::OpenMode::Binary}
            }, "gx file open mode.")
            .defEnum("SeekMode", {
                    {"SEEK_SET", SEEK_SET},
                    {"SEEK_CUR", SEEK_CUR},
                    {"SEEK_END", SEEK_END}
            }, "gx file seek mode.")
            .func("setFile", [](GFile &self, const std::string &path) {
                self.setFile(path);
            }, "Set file by path.")
            .func("setFile", [](GFile &self, const GFile &parent, const std::string &child) {
                self.setFile(parent, child);
            }, "Set the file through the file path and the path.")
            .func("swap", &GFile::swap, "Swap two objects.")
            .func("exists", [](const GFile &self) {
                return self.exists();
            }, "Check if the file exists.")
            .func("isFile", [](const GFile &self) {
                return self.isFile();
            }, "Check if the file is a file.")
            .func("isDirectory", [](const GFile &self) {
                return self.isDirectory();
            }, "Check if the file is a directory.")
            .func("isRelative", &GFile::isRelative, "Check if the directory is relative.")
            .func("isRoot", &GFile::isRoot, "Check if the directory is root.")
            .func("isReadOnly", [](const GFile &self) {
                return self.isReadOnly();
            }, "Check the file or directory is read-only.")
            .func("setReadOnly", [](GFile &self, bool isReadOnly) {
                return self.setReadOnly(isReadOnly);
            }, "Modify the read-only attribute of a file or directory")
            .func("lastModified", [](GFile &self) {
                return (int64_t) self.lastModified();
            }, "Get the last modified time of the file.")
            .func("lastAccess", [](GFile &self) {
                return (int64_t) self.lastAccess();
            }, "Get the last accessed time of the file.")
            .func("filePath", &GFile::filePath, "Get the file path.")
            .func("fileName", &GFile::fileName, "Get the file name.")
            .func("fileNameWithoutExtension", &GFile::fileNameWithoutExtension, "Get the file name without extension.")
            .func("fileSuffix", &GFile::fileSuffix, "Get the file suffix.")
            .func("absoluteFilePath", &GFile::absoluteFilePath, "Get the absolute file path.")
            .func("absolutePath", &GFile::absolutePath, "Get the absolute path.")
            .func("parent", &GFile::parent, "Get the parent directory.")
            .func("remove", &GFile::remove,
                 "Delete a file or directory. The directory must be empty when deleting a directory.")
            .func("rename", &GFile::rename, "Rename a file or directory.")
            .func("concat", [](GFile &self, const GFile &b) {
                return self.concat(b);
            }, "Concatenate two files.")
            .func("concat", [](GFile &self, const std::string &b) {
                return self.concat(b);
            }, "Concatenate two files.")
            .func("listFiles", [](GFile &self) {
                return self.listFiles();
            }, "List the files in the directory.")
            .func("listFiles", [](GFile &self, GAny &filterFunc) {
                if (filterFunc.isFunction()) {
                    return self.listFiles([&](const GFile &file) {
                        return filterFunc(file).as<bool>();
                    });
                }
                return self.listFiles();
            }, "Get all the files in the directory according to the conditions. "
               "The parameter is a function. The function parameter is GFile. The return value is bool. "
               "If true is returned, the file is returned.")
            .func("mkdir", [](GFile &self) {
                return self.mkdir();
            }, "Create a directory if it does not exist.")
            .func("mkdirs", [](GFile &self) {
                return self.mkdirs();
            }, "Create a directory recursively, if the directory does not exist.")
            .func("open", [](GFile &self, int mode) {
                return self.open((GFile::OpenModeFlags) mode);
            }, "Open the file, param: OpenModeFlags.")
            .func("open", [](GFile &self, const std::string &mode) {
                return self.open(mode);
            }, "Open the file, param: mode, C-style file opening mode.")
            .func("flush", &GFile::flush, "Flush the file.")
            .func("close", &GFile::close, "Close the file.")
            .func("isOpen", &GFile::isOpen, "Check if the file is open.")
            .func("fileSize", &GFile::fileSize, "Get the file size.")
            .func("read", [](GFile &self, GAnyBytePtr buffer, int64_t elementSize, int64_t count) {
                return self.read(buffer, elementSize, count);
            }, "Read file in bytes, params: buffer, elementSize, count.")
            .func("read", [](GFile &self, GAnyBytePtr buffer, int64_t maxSize) {
                return self.read(buffer, maxSize);
            }, "Read file in bytes, params: buffer, maxSize.")
            .func("read", [](GFile &self) {
                return self.read();
            }, "Read file as GByteArray.")
            .func("read", [](GFile &self, int64_t maxSize) {
                return self.read(maxSize);
            }, "Read file as GByteArray, param: maxSize.")
            .func("readAll", [](GFile &self) {
                return self.readAll().toStdString();
            }, "Read file as string.")
            .func("readLine", [](GFile &self) {
                return self.readLine().toStdString();
            }, "Read file line string.")
            .func("atEnd", &GFile::atEnd, "Check if the file is at end.")
            .func("write", [](GFile &self, GAnyConstBytePtr buffer, int64_t elementSize, int64_t count) {
                return self.write(buffer, elementSize, count);
            }, "Write file in bytes, params: buffer, elementSize, count.")
            .func("write", [](GFile &self, GAnyConstBytePtr buffer, int64_t size) {
                return self.write(buffer, size);
            }, "Write file in bytes, params: buffer, size.")
            .func("write", [](GFile &self, const GByteArray &data) {
                return self.write(data);
            }, "Write file, param: GByteArray.")
            .func("write", [](GFile &self, const GString &str) {
                return self.write(str);
            }, "Write file, param: GString.")
            .func("write", [](GFile &self, const std::string &str) {
                return self.write(str);
            }, "Write file, param: string.")
            .func("seek", [](GFile &self, int64_t offset, int32_t origin) {
                return self.seek(offset, origin);
            }, "Seek the file, params: offset, origin(SeekMode).")
            .func("seek", [](GFile &self, int64_t offset) {
                return self.seek(offset);
            }, "Seek the file, param: offset.")
            .staticFunc("formatPath", &GFile::formatPath)
            .staticFunc("formatPath", [](const std::string &path) {
                return GFile::formatPath(path);
            })
            .staticFunc("mainDirectory", &GFile::mainDirectory)
            .staticFunc("temporaryDirectory", &GFile::temporaryDirectory)
            .func(MetaFunction::Addition, [](const GFile &file, const std::string &child) {
                return file + child;
            }, "Concatenate two files.")
            .func(MetaFunction::Addition, [](const GFile &file, const GFile &child) {
                return file + child;
            }, "Concatenate two files.")
            .func(MetaFunction::Division, [](const GFile &file, const std::string &child) {
                return file / child;
            })
            .func(MetaFunction::Division, [](const GFile &file, const GFile &child) {
                return file / child;
            });
}

GX_NS_END
