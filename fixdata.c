#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <cutils/properties.h>

#define PATH_MAX 4096

void recursive_chmod(const char *path, mode_t mode) {
    DIR *dir;
    struct dirent *ent;
    char fullpath[PATH_MAX];
    if (chmod(path, mode) != 0) {
        perror("chmod failed");
    }

    dir = opendir(path);
    if (dir == NULL) {
        perror("opendir failed");
        return;
    }

    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }

        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, ent->d_name);

        
        struct stat st;
        if (lstat(fullpath, &st) == 0 && S_ISDIR(st.st_mode)) {
            recursive_chmod(fullpath, mode);
        } else {
            // Change permissions on files
            if (chmod(fullpath, mode) != 0) {
                perror("chmod failed on file");
            }
        }
    }

    closedir(dir);
}

int main() {
    char prop[PROP_VALUE_MAX];
    while (1) {
        property_get("sys.boot_completed", prop, "0");
        if (strcmp(prop, "1") == 0) {
            break;
        }
        sleep(1);
    }
    recursive_chmod("/data", 0777);

    return 0;
}
