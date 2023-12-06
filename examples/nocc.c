#include "../nocc.h"

#include <string.h>
#include <stdio.h>

static const char* usage = 
"usage nocc\n"
"\n"
"commands:\n"
"\tbuild\t\tbuilds the project\n"
"\n"
"options:\n"
"\t-h,--help\t\tprints this message\n"
"\t-v,--version\t\tprints the version of the software";

static const char* build_usage = 
"usage nocc build\n"
"\n"
"options:\n"
"\t-d,--debug\t\tbuilds as a debug build\n"
"\t-d,--release\t\tbuilds as a release build\n"
"\t   --dist\t\tbuilds as a dist build\n"
"\t-h,--help\t\tprints this message\n";

const char* BINARY_PATH = "./bin"; 

bool build_helloworlds();

int main(int argc, char** argv) {
    if(argc < 2) {
        printf("%s", usage);
        return 1;
    }

    // TODO: Make a CLI parser. 
    // For now, i am assuming, that if you want help you will only
    // put --help or -h. 
    if(strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        printf("%s", usage);
        return 0;
    }

    // TODO: Make a CLI parser. 
    // For now, i am assuming, that if you want the version you will only
    // put --version or -v. 
    if(strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
        printf("%s", NOCC_VERSION);
        return 0;
    }

    if(strcmp(argv[1], "build") == 0) {
        if(argc == 3 && (strcmp(argv[2], "--help") == 0 || strcmp(argv[2], "-h") == 0)) {
            printf("%s", build_usage);
            return 0;
        }

        if(!build_helloworlds()) {
            nocc_error("unable to build helloworld");
            return 1;
        }
    }

    else {
        printf("%s", usage);
        return 1;
    }

    return 0;
}

bool build_helloworlds() {
    static const char* TARGET_DIR = "./helloworld.exe";
    
    const char* hellworld_c = "./helloworld.c";

    nocc_darray(const char*) cmd = nocc_da_create(const char*);
    nocc_cmd_add(cmd, "clang");
    nocc_cmd_add(cmd, hellworld_c, "-o", TARGET_DIR);

    nocc_cmd_execute(cmd);

    nocc_da_free(cmd);

    return 1;
}