//
// Created by niuhuan on 2022/12/6.
//

#ifndef ZSIGN_ZIPS_H
#define ZSIGN_ZIPS_H

#include <string>

int unzip(const char *zipFile, const char *folder);

void zip_directory(const std::string& inputdir, const std::string& output_filename);

#endif //ZSIGN_ZIPS_H
