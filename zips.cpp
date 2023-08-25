//
// Created by niuhuan on 2022/12/6.
//

#include "zips.h"
#include "common/common.h"

#include <zip.h>
#include <errno.h>
#include <sys/stat.h>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

static void safe_create_dir(const char *dir) {
    if (mkdir(dir, 0755) < 0) {
        if (errno != EEXIST) {
            perror(dir);
            exit(1);
        }
    }
}

int unzip(const char *zipFile, const char *folder) {
    int error(0);
    zip *za = zip_open(zipFile, 0, &error);
    if (za == NULL) {
        char buf[100];
        zip_error_to_str(buf, sizeof(buf), error, errno);
        ZLog::ErrorV("%s: can't open zip archive `%s': %s\n", folder, zipFile, buf);
        return 1;
    }
    auto entryCount = zip_get_num_entries(za, 0);
    ZLog::DebugV("entryCount : %lu\n", entryCount);
    struct zip_stat sb;
    struct zip_file *zf;
    int fd;
    long long sum;
    char buf[100];
    safe_create_dir(folder);
    ZLog::DebugV("create folder : %s\n", folder);
    for (int i = 0; i < entryCount; ++i) {
        if (zip_stat_index(za, i, 0, &sb) == 0) {
            auto len = strlen(sb.name);
            ZLog::DebugV("Name: [%s], ", sb.name);
            ZLog::DebugV("Size: [%lu], ", sb.size);
            ZLog::DebugV("mtime: [%u]\n", (unsigned int) sb.mtime);
            char pathFinal[2048] = {0};
            strcpy(pathFinal, folder);
            strcat(pathFinal, "/");
            strcat(pathFinal, sb.name);
            ZLog::DebugV("pathFinal : %s\n", pathFinal);
            if (sb.name[len - 1] == '/') {
                safe_create_dir(pathFinal);
            } else {
                zf = zip_fopen_index(za, i, 0);
                if (!zf) {
                    ZLog::ErrorV("boese, boese\n");
                    exit(100);
                }

                fd = open(pathFinal, O_RDWR | O_TRUNC | O_CREAT, 0644);
                if (fd < 0) {
                    ZLog::ErrorV("boese, boese\n");
                    exit(101);
                }
                sum = 0;
                while (sum != sb.size) {
                    int len = zip_fread(zf, buf, 100);
                    if (len < 0) {
                        ZLog::ErrorV("boese, boese\n");
                        exit(102);
                    }
                    write(fd, buf, len);
                    sum += len;
                }
                close(fd);
                zip_fclose(zf);
            }
        } else {
            ZLog::ErrorV("File[%s] Line[%d]\n", __FILE__, __LINE__);
        }

    }
    return 0;
}

static bool is_dir(const std::string& dir)
{
    struct stat st;
    ::stat(dir.c_str(), &st);
    return S_ISDIR(st.st_mode);
}

static void walk_directory(const std::string& startdir, const std::string& inputdir, zip_t *zipper)
{
    DIR *dp = ::opendir(inputdir.c_str());
    if (dp == nullptr) {
        throw std::runtime_error("Failed to open input directory: " + std::string(::strerror(errno)));
    }

    struct dirent *dirp;
    while ((dirp = readdir(dp)) != NULL) {
        if (dirp->d_name != std::string(".") && dirp->d_name != std::string("..")) {
            std::string fullname = inputdir + "/" + dirp->d_name;
            if (is_dir(fullname)) {
                if (zip_dir_add(zipper, fullname.substr(startdir.length() + 1).c_str(), ZIP_FL_ENC_UTF_8) < 0) {
                    throw std::runtime_error("Failed to add directory to zip: " + std::string(zip_strerror(zipper)));
                }
                walk_directory(startdir, fullname, zipper);
            } else {
                zip_source_t *source = zip_source_file(zipper, fullname.c_str(), 0, 0);
                if (source == nullptr) {
                    throw std::runtime_error("Failed to add file to zip: " + std::string(zip_strerror(zipper)));
                }
                if (zip_file_add(zipper, fullname.substr(startdir.length() + 1).c_str(), source, ZIP_FL_ENC_UTF_8) < 0) {
                    zip_source_free(source);
                    throw std::runtime_error("Failed to add file to zip: " + std::string(zip_strerror(zipper)));
                }
            }
        }
    }
    ::closedir(dp);
}

void zip_directory(const std::string& inputdir, const std::string& output_filename)
{
    int errorp;
    zip_t *zipper = zip_open(output_filename.c_str(), ZIP_CREATE | ZIP_EXCL, &errorp);
    if (zipper == nullptr) {
        zip_error_t ziperror;
        zip_error_init_with_code(&ziperror, errorp);
        throw std::runtime_error("Failed to open output file " + output_filename + ": " + zip_error_strerror(&ziperror));
    }

    try {
        walk_directory("", inputdir, zipper);
    } catch(...) {
        zip_close(zipper);
        throw;
    }

    zip_close(zipper);
}
