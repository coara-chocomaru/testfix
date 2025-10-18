#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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
    
            if (chmod(fullpath, mode) != 0) {
                perror("chmod failed on file");
            }
        }
    }

    closedir(dir);
}

int main() {

    sleep(30);

    recursive_chmod("/data", 0777);

    return 0;
}
