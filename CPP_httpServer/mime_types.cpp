#include "mime_types.h"

std::unordered_map<std::string, std::string> mimeTypes = {
    {".html", "text/html"},
    {".htm", "text/html"},
    {".css", "text/css"},
    {".js", "application/javascript"},
    {".json", "application/json"},
    {".png", "image/png"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".gif", "image/gif"},
    {".svg", "image/svg+xml"},
    {".webp", "image/webp"},
    {".ico", "image/vnd.microsoft.icon"},
    {".bmp", "image/bmp"},
    {".tiff", "image/tiff"},
    {".pdf", "application/pdf"},
    {".zip", "application/zip"},
    {".tar", "application/x-tar"},
    {".gz", "application/gzip"},
    {".xml", "application/xml"},
    {".csv", "text/csv"},
    {".txt", "text/plain"},
    {".mp4", "video/mp4"},
    {".mp3", "audio/mpeg"},
    {".wav", "audio/wav"},
    {".flac", "audio/flac"},
    {".ogv", "video/ogg"},
    {".ogg", "audio/ogg"},
    {".webm", "video/webm"}
};

std::string getMimeType(const std::string& filePath) {
    std::string extension = filePath.substr(filePath.find_last_of('.'));
    if (mimeTypes.find(extension) != mimeTypes.end()) {
        return mimeTypes[extension];
    }
    return "application/octet-stream";
};