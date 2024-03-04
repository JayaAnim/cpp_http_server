#include "globals.h"

const char* get_ft(const char* filepath) {
    const char* ext = strrchr(filepath, '.');
    if (ext != NULL) {
        if (strcmp(ext, ".html") == 0) {
            return "text/html";
        } else if (strcmp(ext, ".txt") == 0) {
            return "text/plain";
        } else if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) {
            return "image/jpeg";
        } else if (strcmp(ext, ".png") == 0) {
            return "image/png";
        } else if (strcmp(ext, ".gif") == 0) {
            return "image/gif";
        } else if (strcmp(ext, ".pdf") == 0) {
            return "application/pdf";
        }
    }
    return NULL;
}
