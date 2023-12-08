#include "../nocc.h"

#include <string.h>
#include <stdio.h>

const char* BINARY_PATH = "./bin"; 

typedef struct {
    bool build;
    bool run;
    bool help;
    bool version;
    bool debug, release;
} nocc_ap_parse_result;

bool build_helloworlds(nocc_ap_parse_result* result);
bool run_helloworlds(nocc_ap_parse_result* result);

int main(int argc, char** argv) {
    nocc_ap_parse_result result;

    bool *help = &result.help;

    nocc_argparse_option build_options[] = {
        { .short_name='h', .long_name="help", .description="Prints this message", .output_ptr=help },
        { .short_name='d', .long_name="debug", .description="Builds as debug", .output_ptr=&(result.debug) },
        { .short_name='r', .long_name="release", .description="Builds as release", .output_ptr=&(result.release) }
    };

    nocc_argparse_option run_options[] = {
        { .short_name='h', .long_name="help", .description="Prints this message", .output_ptr=help },
    };

    nocc_argparse_option program_options[] = {
        { .short_name='h', .long_name="help", .description="Prints this message", .output_ptr=help },
        { .short_name='v', .long_name="version", .description="Prints the version", .output_ptr=&(result.version) },
    };

    nocc_argparse_command subcommands[] = { 
        { .name = "build", .description = "Builds the project", .options = build_options, .options_size = (sizeof(build_options) / sizeof(nocc_argparse_option)), .output_ptr = &(result.build) },
        { .name = "run", .description = "runs the project", .options = run_options, .options_size = (sizeof(run_options) / sizeof(nocc_argparse_option)), .output_ptr = &(result.run) }
    };

    nocc_argparse_command program = {
        .name = "nocc",
        .description = "Building, linking, and running all your favorite code",
        .options = program_options, .options_size = (sizeof(program_options) / sizeof(nocc_argparse_option)),
        .commands = subcommands, .commands_size = (sizeof(subcommands) / sizeof(nocc_argparse_command))
    };

    // nocc_argparse_command program = {0};
    // nocc_ap_name(&program, "nocc");
    // nocc_ap_description(&program, "Building, linking, and running all your favorite code");
    // nocc_ap_option(&program, 'h', "help", "Prints this message", &result.help);
    // nocc_ap_option(&program, 'v', "version", "Prints the version of the software", &result.version);

    // nocc_argparse_command build = {0};
    // nocc_ap_name(&build, "build");
    // nocc_ap_description(&build, "Builds the project");
    // nocc_ap_option(&build, 'h', "help", "Prints this message", &result.help);
    // nocc_ap_option(&build, 'd', "debug", "Builds as a debug", &result.debug);
    // nocc_ap_option(&build, 'r', "release", "Builds as a release build", &result.release);
    // nocc_ap_command(&program, &build, &result.build);

    // nocc_argparse_command run = {0};
    // nocc_ap_name(&run, "run");
    // nocc_ap_description(&run, "Runs the project");
    // nocc_ap_option(&run, 'h', "help", "Prints this message", &result.help);
    // nocc_ap_command(&program, &run, &result.run);

    nocc_ap_parse(&program, argc, argv);

    int status = 0;

    if(result.build) {
        nocc_trace("Hello, world");
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
    nocc_cmd_add(cmd, hellworld_c, "-o", TARGET_DIR);
    if(result->debug) {
        nocc_cmd_add(cmd, "-g", "-O0");
    } else if(result->release) {
        nocc_cmd_add(cmd, "-O2");
    }

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