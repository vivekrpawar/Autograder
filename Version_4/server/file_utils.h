#ifndef FILEUTILS_H
#define FILEUTILS_H
#include <string>

int recv_file(std::string filename, int newsockfd);
std::string generateUniqueFileName();

#endif