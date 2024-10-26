#ifndef MIME_TYPES_H
#define MIME_TYPES_H

#include <unordered_map>
#include <string>

extern std::unordered_map<std::string, std::string> mimeTypes;

std::string getMimeType(const std::string &filePath);

#endif