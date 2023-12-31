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
    char* project_name;
} nocc_ap_parse_result;

bool build_helloworlds(nocc_ap_parse_result* result);
bool run_helloworlds(nocc_ap_parse_result* result);

int main(int argc, char** argv) {
    nocc_ap_parse_result result = {};

    nocc_argparse_opt switch_args[] = {
        nocc_ap_opt_boolean('d', "debug", "Builds the program as a debug build", NULL, NULL),
        nocc_ap_opt_boolean('r', "release", "Builds the program as a release build", NULL, NULL)
    };

    nocc_argparse_opt build_options[] = {
        nocc_ap_opt_switch(switch_args, "debug", &(result.config)),
        nocc_ap_opt_boolean('h', "help", "Prints this message", NULL, &(result.help))
    };

    nocc_argparse_opt build_arguments[] = {
        nocc_ap_arg_string("project_name", "Builds the project", "all", &(result.project_name))
    };

    nocc_argparse_opt run_options[] = {
        nocc_ap_opt_boolean('h', "help", "Prints this message", NULL, &(result.help))
    };

    nocc_argparse_opt program_options[] = {
        nocc_ap_opt_boolean('h', "help", "Prints this message", NULL, &(result.help)),
        nocc_ap_opt_boolean('v', "version", "Prints the software version", NULL, &(result.version))
    };

    nocc_argparse_opt subcommands[] = { 
        nocc_ap_cmd("build", "Builds the project", build_options, build_arguments, NULL, &(result.build)),
        nocc_ap_cmd("run", "runs the project", run_options, NULL, NULL, &(result.run))
    };

    nocc_argparse_opt program = nocc_ap_cmd("nocc", "Building, linking, and running all your favorite code", program_options, NULL, subcommands, NULL);


    nocc_ap_parse(&program, argc, argv);

    int status = 0;

    if(result.build) {
        if(result.help) {
            nocc_ap_usage(&program.commands[0]);
            goto failure;
        }

        if(strcmp(result.project_name, "helloworld") == 0 || strcmp(result.project_name, "all") == 0) {
            if(!build_helloworlds(&result)) {
                nocc_error("unable to build helloworld");
                status = 1;
                goto failure;
            }

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

#define _NOCC_USE_NEW_GEN_FUNCTION_

bool build_helloworlds(nocc_ap_parse_result* result) {
    static const char* TARGET_DIR = "./helloworld.exe";
    
    const char* helloworld_c = "./helloworld.c";
    const char* helloworld_o = "./helloworld.o";

    // TODO: The nocc_should_recompile function should not be exposed to the user.
    if(nocc_should_recompile(&helloworld_c, 1, helloworld_o)) {
        printf("Compiling ./helloworld.c");

        // Compiling the file
        nocc_darray(const char*) cmd = nocc_da_create(const char*);
        nocc_cmd_add(cmd, "clang");
        if(strcmp(result->config, "debug") == 0) {
            nocc_cmd_add(cmd, "-g", "-O0");
        } else if(strcmp(result->config, "release") == 0){ 
            nocc_cmd_add(cmd, "-O2");
        }
        
        nocc_cmd_add(cmd, "-c", helloworld_c, "-o", helloworld_o);
        nocc_cmd_execute(cmd);
        
        nocc_da_free(cmd);
    }

    if(nocc_should_recompile(&helloworld_o, 1, TARGET_DIR)) {
        // Linking the file
        nocc_darray(const char*) cmd = nocc_da_create(const char*);
        nocc_cmd_add(cmd, "clang", "-o", TARGET_DIR, helloworld_o);
                
        nocc_cmd_execute(cmd);
            
        nocc_da_free(cmd);
    }


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