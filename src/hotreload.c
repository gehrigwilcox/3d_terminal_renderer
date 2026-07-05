#define _POSIX_C_SOURCE 199309L
#define _DEFAULT_SOURCE

#include "hotreload.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <dirent.h>

// Stable library code (render_math.cpp, and anything else you add later)
// lives here. Each file is compiled to a cached .o the first time it's
// seen (or whenever it changes), and every .o present gets linked into
// the scene .so. Add a new .c/.cpp file to this folder and it's picked
// up automatically — no build command to edit.
#define LIB_SRC_DIR "src/lib"
#define LIB_OBJ_DIR "build/lib_objs"

static time_t get_mtime(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return st.st_mtime;
}

// Latest mtime among all files directly inside src/lib/ (render_math.cpp,
// sdf.c, anything else you add there). Used so editing a library file
// alone -- without touching scene.c -- still triggers a hot reload.
static time_t get_lib_dir_mtime(void) {
    DIR *d = opendir(LIB_SRC_DIR);
    if (!d) return 0;

    time_t latest = 0;
    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.') continue; // skip . and ..
        char path[300];
        snprintf(path, sizeof(path), "%s/%s", LIB_SRC_DIR, entry->d_name);
        time_t m = get_mtime(path);
        if (m > latest) latest = m;
    }
    closedir(d);
    return latest;
}

// The combined "has anything relevant changed?" clock: newest of
// scene.c's own mtime and everything in src/lib/.
static time_t get_watch_mtime(const char *scene_src) {
    time_t scene_m = get_mtime(scene_src);
    time_t lib_m = get_lib_dir_mtime();
    return scene_m > lib_m ? scene_m : lib_m;
}

// Compiles any new/changed file in src/lib/ to build/lib_objs/<name>.o.
// Existing up-to-date .o files are left alone. Returns 0 on success.
static int build_lib_objects(void) {
    const char *script =
        "mkdir -p " LIB_OBJ_DIR " && "
        "for src in " LIB_SRC_DIR "/*.c " LIB_SRC_DIR "/*.cpp; do "
        "  [ -e \"$src\" ] || continue; "
        "  base=$(basename \"$src\"); "
        "  obj=\"" LIB_OBJ_DIR "/${base%.*}.o\"; "
        "  if [ ! -e \"$obj\" ] || [ \"$src\" -nt \"$obj\" ]; then "
        "    cc -O2 -fPIC -c -Iinclude -Iexternal/lalib/include \"$src\" -o \"$obj\" || exit 1; "
        "  fi; "
        "done";
    int rc = system(script);
    return rc == 0 ? 0 : -1;
}

// Compiles scene_src (always, since it changes every save) and links it
// against every cached object file in build/lib_objs/.
static int compile_scene(const char *scene_src, const char *so_path) {
    if (build_lib_objects() != 0) return -1;

    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
        "objs=$(find " LIB_OBJ_DIR " -name '*.o'); "
        "cc -O2 -shared -fPIC -Iinclude -Iexternal/lalib/include "
        "%s $objs -o %s.tmp -lm -lstdc++ "
        "2> build/scene_build.log && mv %s.tmp %s",
        scene_src, so_path, so_path, so_path);
    int rc = system(cmd);
    return rc == 0 ? 0 : -1;
}

static int try_load(const char *so_path, void **out_handle, scene_sdf_fn *out_fn) {
    void *handle = dlopen(so_path, RTLD_NOW);
    if (!handle) {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
        return -1;
    }
    scene_sdf_fn fn = (scene_sdf_fn)dlsym(handle, "scene_sdf");
    if (!fn) {
        fprintf(stderr, "dlsym failed: %s\n", dlerror());
        dlclose(handle);
        return -1;
    }
    *out_handle = handle;
    *out_fn = fn;
    return 0;
}

int hotreload_init(hot_scene_t *hs, const char *scene_src) {
    memset(hs, 0, sizeof(*hs));

    // version resets to 0 every run, so without this, any .so left over
    // from a previous run (e.g. you quit instead of it being replaced
    // mid-session) never gets cleaned up and just accumulates over time.
    system("rm -f build/scene_*.so");

    snprintf(hs->current_so_path, sizeof(hs->current_so_path),
             "build/scene_%d.so", hs->version++);

    if (compile_scene(scene_src, hs->current_so_path) != 0) {
        fprintf(stderr, "Failed to compile scene\n");
        system("cat build/scene_build.log");
        return -1;
    }
    if (try_load(hs->current_so_path, &hs->handle, &hs->scene_sdf) != 0) {
        return -1;
    }

    hs->last_mtime = get_watch_mtime(scene_src);
    return 0;
}

void hotreload_poll(hot_scene_t *hs, const char *scene_src) {
    time_t mtime = get_watch_mtime(scene_src);
    if (mtime == 0 || mtime == hs->last_mtime) return;

    usleep(50000); // let the editor finish writing before we read the file
    hs->last_mtime = mtime;

    char new_path[64];
    snprintf(new_path, sizeof(new_path), "build/scene_%d.so", hs->version++);

    if (compile_scene(scene_src, new_path) != 0) {
        return; // keep running the previously loaded scene
    }

    void *new_handle;
    scene_sdf_fn new_fn;
    if (try_load(new_path, &new_handle, &new_fn) != 0) {
        remove(new_path);
        return;
    }

    void *old_handle = hs->handle;
    char old_path[64];
    strncpy(old_path, hs->current_so_path, sizeof(old_path));
    old_path[sizeof(old_path) - 1] = '\0';

    hs->handle = new_handle;
    hs->scene_sdf = new_fn;
    strncpy(hs->current_so_path, new_path, sizeof(hs->current_so_path));
    hs->current_so_path[sizeof(hs->current_so_path) - 1] = '\0';

    // Safe to unlink now: on Linux, dlclose fully unmaps the old library
    // before the file's last reference is dropped, so this can't yank
    // code out from under a still-running thread.
    dlclose(old_handle);
    remove(old_path);
}