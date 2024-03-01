#include "../nocc.h"
#include <getopt.h>

#define CFLAGS_COMMON "-std=c17 -Wall -Wformat=2"
#define CFLAGS_DEBUG "-g -O0"
#define CFLAGS_RELEASE "-O3"

#define LDFLAGS_COMMON ""
#define LDFLAGS_DEBUG "-g"
#define LDFLAGS_RELEASE ""

typedef enum {
    DEBUG, RELEASE
} config;

typedef struct {
    config config;
    bool help;
    bool version;
} result;

static const char* usage =
"./nocc.exe [options]\n"
"\n"
"options:\n"
"   -d, --debug         builds as a debug build\n"
"   -r, --release       builds as a release build\n"
"   -h, --help          prints this message\n"
"   -v, --version       prints the nocc's version\n"
;

int main(int argc, char** argv) {
    result res;
    static struct option long_options[] = {
    /*    NAME          ARGUMENT            FLAG    SHORTNAME */
        { "debug",      no_argument,        NULL,   'd' },
        { "release",    no_argument,        NULL,   'r' },
        { "help",       no_argument,        NULL,   'h' },
        { "version",    no_argument,        NULL,   'v' },
        { NULL,         0,                  NULL,   0   }
    };

    int c, opt_index = 0;
    while((c = getopt_long(argc, argv, "drhv", long_options, &opt_index)) != -1) {
        switch (c) {
            case 'h':
                printf("%s\n", usage);
                return 0;
            case 'v':
                printf("%s\n", NOCC_VERSION);
                return 0;
            case 'd':
                res.config = DEBUG;
                break;
            case 'r':
                res.config = RELEASE;
                break;
            case '?':
                printf("%s\n", usage);
                return 1;
            default:
                break;
        }
    }

    nocc_mkdir_if_not_exists("./bin/");
    nocc_mkdir_if_not_exists("./bin-obj/");

    const char* target = "./bin/helloworld.exe";

    char cflags[1024] = CFLAGS_COMMON " ";
    char* ldflags = NULL;
    if(res.config == DEBUG) {
        strcat(cflags, CFLAGS_DEBUG " ");
        ldflags = LDFLAGS_DEBUG " ";
    } else if(res.config == RELEASE) {
        strcat(cflags, CFLAGS_RELEASE);
    }

    nocc_compilation_options opts = {
        .cc = "gcc",
        .cflags = cflags,
        .includes = NULL,
        .defines = NULL,
        .src_filter = "./src/*.c",
        .inc_filter = "./src/*.h",
        .obj_format = "./bin-obj/%n.o"
    };
    nocc_darray(const char*) objs = nocc_compile(opts);

    nocc_link_options link_opts = {
        .ld = opts.cc,
        .ldflags = ldflags,
        .ldadd = NULL,
        .objs = objs,
        .target = target
    };
    nocc_link(link_opts);

    return 0;
}

