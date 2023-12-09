#include "../nocc.h"

#include <string.h>
#include <stdio.h>

const char* BINARY_PATH = "./bin"; 

typedef struct {
    bool build;
    bool run;
    bool help;
    bool version;
    char* config;
} nocc_ap_parse_result;

bool build_helloworlds(nocc_ap_parse_result* result);
bool run_helloworlds(nocc_ap_parse_result* result);

int main(int argc, char** argv) {
    nocc_ap_parse_result result = {};

    nocc_argparse_option build_options[] = {
        nocc_ap_opt_switch('d', "debug", "Builds as debug", result.config),
        nocc_ap_opt_switch('r', "release", "Builds as release", result.config),
        nocc_ap_opt_boolean('h', "help", "Prints this message", result.help)
    };

    nocc_argparse_option run_options[] = {
        nocc_ap_opt_boolean('h', "help", "Prints this message", result.help)
    };

    nocc_argparse_option program_options[] = {
        nocc_ap_opt_boolean('h', "help", "Prints this message", result.help),
        nocc_ap_opt_boolean('v', "version", "Prints the software version", result.help)
    };

    nocc_argparse_command subcommands[] = { 
        nocc_ap_cmd("build", "Builds the project", build_options, NULL, NULL, result.build),
        nocc_ap_cmd("run", "runs the project", run_options, NULL, NULL, result.run)
    };

    nocc_argparse_command program = nocc_ap_cmd("nocc", "Building, linking, and running all your favorite code", program_options, NULL, subcommands, result.build);

    nocc_ap_parse(&program, argc, argv);

    int status = 0;

    if(result.build) {
        if(result.help) {
            nocc_ap_usage(&program.commands[0]);
            goto failure;
        }

        if(!build_helloworlds(&result)) {
            nocc_error("unable to build helloworld");
            status = 1;
            goto failure;
        }
    }

    else if (result.run) {
        if(result.help) {
            nocc_ap_usage(&program.commands[1]);
            goto failure;
        }

        if(!run_helloworlds(&result)) {
            nocc_error("unable to run helloworld");
            status = 1;
            goto failure;
        }
    }

    else if(result.help) {
        nocc_ap_usage(&program);
        goto failure;
    }

    else if(result.version) {
        printf("%s", NOCC_VERSION);
        goto failure;
    }

    else {
        nocc_ap_usage(&program);
        status = 1;
        goto failure;
    }

failure:
    return 0;
}

bool build_helloworlds(nocc_ap_parse_result* result) {
    static const char* TARGET_DIR = "./helloworld.exe";
    
    const char* hellworld_c = "./helloworld.c";

    nocc_darray(const char*) cmd = nocc_da_create(const char*);

    printf("Building helloworld.c\n");
    nocc_cmd_add(cmd, "clang");
    if(strcmp(result->config, "debug") == 0) {
        nocc_cmd_add(cmd, "-g", "-O0");
    } else if(strcmp(result->config, "release") == 0){ 
        nocc_cmd_add(cmd, "-O2");
    }
    nocc_cmd_add(cmd, hellworld_c, "-o", TARGET_DIR);

    nocc_cmd_execute(cmd);

    nocc_da_free(cmd);

    return true;
}

bool run_helloworlds(nocc_ap_parse_result* result) {
    printf("Running helloworld.c\n");
    nocc_darray(const char*) cmd = nocc_da_create(const char*);
    nocc_cmd_add(cmd, ".\\helloworld.exe");
    
    nocc_cmd_execute(cmd);

    nocc_da_free(cmd);

    return true;
}