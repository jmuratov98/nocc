#ifndef _NO_C_COMPILER_H_
#define _NO_C_COMPILER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <errno.h>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN 
    #include <Windows.h>
    #include <direct.h>
#else
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <dirent.h>
    #include <libgen.h>
#endif

// DEFS
#define NOCC_VERSION_MAJOR      0
#define NOCC_VERSION_MINOR      2
#define NOCC_VERSION_PATCH      "0-a.0"
#define NOCC_VERSION            "0.2.0-a.0"

#define NOCC_INIT_CAP           10

#ifdef _MSVC_LANG
    #define nocc_debugbreak     __debugbreak
#else
    #define nocc_debugbreak     __builtin_trap
#endif

// Logging Begin ==========================================================
typedef enum {
    NOCC_LOG_LEVEL_TRACE, NOCC_LOG_LEVEL_DEBUG, NOCC_LOG_LEVEL_INFO, NOCC_LOG_LEVEL_WARN, NOCC_LOG_LEVEL_ERROR, 
    NOCC_LOG_LEVEL_OFF
} nocc_log_level;

#define buffer_size  1024
int _nocc_log_output(nocc_log_level level, const char* fmt, ...) {
    static const char* levels[NOCC_LOG_LEVEL_OFF] = { "trace", "debug", "info", "warn", "error" };
    char buffer[buffer_size] = {0};

    va_list args;
    va_start(args, fmt);
    vsprintf_s(buffer, buffer_size, fmt, args);
    va_end(args);

    int status = printf("[%s]: %s\n", levels[level], buffer);
    return status;
}

/**
 * @brief Logs a formatted output to the console. Either as a 'trace', 'debug', 'info', 'warn', or 'error'.
 * 
 * @param {const char*} fmt -- the formatted output
 * @param {...} ... -- the arguments which get formatted.
 * 
 * @return {int} returns the amount of bytes printed to the console.
 * 
*/
#define nocc_trace(fmt, ...)    _nocc_log_output(NOCC_LOG_LEVEL_TRACE, fmt, ##__VA_ARGS__)
#define nocc_debug(fmt, ...)    _nocc_log_output(NOCC_LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define nocc_info(fmt, ...)     _nocc_log_output(NOCC_LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define nocc_warn(fmt, ...)     _nocc_log_output(NOCC_LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define nocc_error(fmt, ...)    _nocc_log_output(NOCC_LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)

#ifdef NOCC_DEBUG
    #define NOCC_ENABLE_ASSERTS
#endif

#ifdef NOCC_ENABLE_ASSERTS
    #define nocc_assert(x, ...) {               \
        if((x)) {                               \
        } else {                                \
            nocc_error("ASSERTION FAILED %s: (%s %zu): %s", #x, __FILE__, __LINE__, ##__VA_ARGS__);\
            nocc_debugbreak();               \
        }                                       \
    }
#else
    #define nocc_assert(x, ...)
#endif // NOCC_ENABLE_ASSERTS
// Logging End ============================================================

// Array Begin ==========================================================
// This is private and should not be unitilized
typedef struct {
    size_t capacity, size, stride;
} _nocc_da_header;

// A helper function to calculate the head of the pointer. This is private and should not be utilized
#define _nocc_da_calc_header(a) (_nocc_da_header*)((uint8_t*)(a) - sizeof(_nocc_da_header));

// Since these function 
void* _nocc_da_reserve(size_t stride, size_t cap);
void  _nocc_da_free(void* array);
void* _nocc_da_push(void* array, void* value);
void* _nocc_da_pushn(void* array, size_t n, void* value);
void* _nocc_da_grow(void* array, size_t new_capacity);
size_t _nocc_da_size(void* array);
size_t _nocc_da_capacity(void* array);
size_t _nocc_da_stride(void* array);

/**
 * @brief a wrapper. To use this as the type. Think of std::vector<T> in C++
*/
#define nocc_darray(T) T*

/**
 * @brief Creates an array with a stated capacity
 * 
 * @param {T} type -- the type of the array to reserve
 * @param {size_t} cap -- The capacity of the array
 * 
 * @return {void*} returns the newly constructed array or NULL if the creation failed.
 * 
*/
#define nocc_da_reserve(T, cap)                 _nocc_da_reserve(sizeof(T), cap)

/**
 * @brief Creates an array with a capacity of 10.
 * 
 * @param {T} type -- the type of the array to reserve
 * 
 * @return {void*} returns the newly constructed array or NULL if the creation failed.
 * 
*/
#define nocc_da_create(T)                       nocc_da_reserve(T, NOCC_INIT_CAP)

/**
 * @brief Frees the array. If the elements were allocated on the heap. The user must free them.
 * 
 * @param {void*} array -- the type of the array to reserve
 * 
 * @return {void}
 * 
*/
#define nocc_da_free(a)                         _nocc_da_free(a);

/**
 * @brief Pushs the value to the end of the array. Think std::vector::push_back
 * 
 * @param {void*} a -- The array
 * @param {void*} v -- The value to add to the array.
 * 
 * @return {void}
*/
#define nocc_da_push(a, v) {                    \
    typeof((v)) temp = (v);                     \
    a = _nocc_da_push(a, &temp);                \
}

/**
 * @brief Pushs the value to the end of the array. Think std::vector::push_back
 * 
 * @param {void*} a -- The array
 * @param {size_t} n -- The amount of elements to add.
 * @param {void*} v -- The values (as an array) to add to the array.
 * 
 * @return {void}
*/
#define nocc_da_pushn(a, n, v)                  { a = _nocc_da_pushn(a, n, v); }

/**
 * @brief Pushs the value to the end of the array. Think std::vector::push_back
 * 
 * @param {void*} a -- The array
 * @param {...} ... -- The values to add to the array.
 * 
 * @return {void}
*/
#define nocc_da_push_many(a, ...)               { a = _nocc_da_pushn(a, sizeof((typeof(__VA_ARGS__)[]){__VA_ARGS__}) / nocc_da_stride(a), (typeof(__VA_ARGS__)[]){__VA_ARGS__}); }

/**
 * @brief returns the size of the array
 * 
 * @param {void*} a -- The array
 * 
 * @return {size_t} The size of the array
 * 
*/
#define nocc_da_size(a)                 _nocc_da_size(a)

/**
 * @brief returns the capacity of the array
 * 
 * @param {void*} a -- The array
 * 
 * @return {size_t} The capacity of the array
 * 
*/
#define nocc_da_capacity(a)             _nocc_da_capacity(a)

/**
 * @brief returns the stride of the array
 * 
 * @param {void*} a -- The array
 * 
 * @return {size_t} The stride of the array
 * 
*/
#define nocc_da_stride(a)               _nocc_da_stride(a)
// Array End ============================================================

// String Begin ==========================================================
/**
 * @brief the type of the string
 * 
 * TODO: what would be cool is to allow wide strings as well, such as wchar_t
*/
#define nocc_string char*

/**
 * @brief Creates a string with a capacity. This is just a wrapper of the array class from above
 * 
 * @param {size_t} cap -- The amount to reserve the array with
 * 
 * @return {void*} The newly created string
 * 
 * TODO: what would be cool is to allow wide strings as well, such as wchar_t
*/
#define nocc_str_reserve(cap)               nocc_da_reserve(char, cap)

/**
 * @brief Creates a string with a predefined capacity. This is just a wrapper of the array class from above
 * 
 * @return {void*} The newly created string
 * 
 * TODO: what would be cool is to allow wide strings as well, such as wchar_t
*/
#define nocc_str_create()                   nocc_da_reserve(char, NOCC_INIT_CAP)

/**
 * @brief Frees the string. This is just a wrapper of the array class from above
 * 
 * @param {void*} str -- The string
 * 
 * @return {void}
 * 
*/
#define nocc_str_free(str)                  nocc_da_free(str)

/**
 * @brief Pushes a character to the end of the string. This is just a wrapper of the array class from above
 * 
 * @param {void*} str -- The string
 * @param {char} c -- The character to add
 * 
 * @return {void}
*/
#define nocc_str_push_char(str, c)          nocc_da_push(str, c)

/**
 * @brief Pushes a '\0' character to the end of the string. Without it, the string could not be used as a C string. This is just a wrapper of the array class from above
 * 
 * @param {void*} str -- The string
 * 
 * @return {void}
*/
#define nocc_str_push_null(str)             nocc_da_push(str, '\0')

/**
 * @brief Pushes a C string to the end of the string. This is just a wrapper of the array class from above
 * 
 * @param {void*} str -- The string
 * @param {char*} s -- The string to add
 * 
 * @return {void}
*/
#define nocc_str_push_cstr(str, s)          nocc_da_pushn(str, strlen(s), s);

/**
 * @brief Gets the size of the string. This is just a wrapper of the array class from above
 * 
 * @param {void*} str -- The string
 * 
 * @return {size_t} the size/length of the string.
*/
#define nocc_str_size(s)                    nocc_da_size

/**
 * @brief Gets the capacity of the string. This is just a wrapper of the array class from above
 * 
 * @param {void*} str -- The string
 * 
 * @return {size_t} the capacity of the string. The total it can carry until needing to resize the string.
*/
#define nocc_str_capacity(s)                nocc_da_capacity

/**
 * @brief Gets the stride of the string. This is just a wrapper of the array class from above
 * 
 * @param {void*} str -- The string
 * 
 * @return {size_t} the stride of each element of the string.
*/

#define nocc_str_stride(s)                  nocc_da_stride
// String End ============================================================

// Argument Parsing Begin =================================================

/**
 * TODO
 * TODO: While the design is cool? it is definitely not expandable. I think it would
 * TODO: definitely be better to move away from a heap based to a stack based CLI parser
 * TODO: And by that I mean the nocc_argparse_option, nocc_argparse_arguments, and nocc_argparse_command
 * TODO: are all allocated on the heap with the exception of the main `program` command that the user
 * TODO: creates.
 * 
 * TODO: After the shift to a total stack based CLI parser, I need a good way to be able to parse
 * TODO: ints, strings, or floats from the CLI rather than just a boolean.
 * 
 * TODO: After that, I need a way to have a switch. By that I mean say a user wants to build a release build,
 * TODO: if the user types in '--release' it itmplies it as `config: release` rather than as a boolean field
 * TODO: that it currently is.
 * 
*/

typedef enum {
    NOCC_APT_UNKNOWN = 0,

    // BASE TYPES
    NOCC_APT_BOOLEAN, NOCC_APT_NUMBER, NOCC_APT_FLOAT, NOCC_APT_STRING,
    
    // COMPLEX TYPES
    NOCC_APT_ARRAY,     // TODO: Implement this later...

    /**
     * @brief the idea of this is allow for a flag to imply some thing else. For example, --debug flag
     * may be interpreted as to config="debug" (depending on the pointer that is put in)
    */
    NOCC_APT_SWITCH,    // TODO: Implement this to allow for other data types other than a string.  
} _nocc_argparse_type;

typedef struct {
    char short_name;
    const char* long_name;
    const char* description;
    void* output_ptr;

    // Internal stuff. If I used a .c file, i would hide this in there, but i am not.
    _nocc_argparse_type _type;
} nocc_argparse_option;

typedef struct {
    bool is_optional;
} nocc_argparse_argument;

typedef struct nocc_argparse_command {
    const char* name;
    const char* description;
    nocc_darray(nocc_argparse_argument) arguments;
    size_t arguments_size;
    nocc_darray(nocc_argparse_option) options;
    size_t options_size;
    nocc_darray(struct nocc_argparse_command) commands;
    size_t commands_size;
    bool* output_ptr;
} nocc_argparse_command;

#define nocc_ap_opt_boolean(sn, ln, d, op) { ._type=NOCC_APT_BOOLEAN, .short_name=(sn), .long_name=(ln), .description=(d), .output_ptr=&(op) }
#define nocc_ap_opt_switch(sn, ln, d, op)  { ._type=NOCC_APT_SWITCH, .short_name=(sn), .long_name=(ln), .description=(d), .output_ptr=&(op) }

#define nocc_ap_cmd(n, d, o, a, c, op) {                                                \
    .name = (n),                                                                        \
    .description = (d),                                                                 \
    .options = (o), .options_size = (sizeof(o) / sizeof(nocc_argparse_option)),         \
    .arguments = (a), .arguments_size = (sizeof(a) / sizeof(nocc_argparse_argument)),   \
    .commands = (a), .commands_size = (sizeof(c) / sizeof(nocc_argparse_command)),      \
    .output_ptr = &(op)                                                                 \
}

// TODO: refactor this code. 
bool _nocc_ap_parse_rec(nocc_argparse_command* command, int beg, int argc, char** argv) {
    nocc_assert(command, "command cannot be NULL");
    nocc_assert(beg < argc, "");
    nocc_assert(argv, "argv cannot be NULL");

    for(size_t i = beg; i < argc; i++) {
        char* arg = argv[i];

        if(command->commands) {
            for(size_t i = 0; i < command->commands_size; i++) {
                nocc_argparse_command cmd = command->commands[i];

                if(strcmp(cmd.name, arg) == 0) {
                    *(cmd.output_ptr) = true;
                }
                _nocc_ap_parse_rec(&cmd, beg + 1, argc, argv);
            }
        }

        if(command->options) {
            for(size_t i = 0; i < command->options_size; i++) {
                nocc_argparse_option opt = command->options[i];
                if(arg[0] == '-') {     // If true it can either be a long name or short name
                    if(arg[1] == '-') { // If true then its the long name
                        if(strcmp(opt.long_name, arg + 2) == 0) {
                            switch(opt._type) {
                            case NOCC_APT_BOOLEAN:
                                *(bool*)(opt.output_ptr) = true;
                                break;
                            case NOCC_APT_SWITCH:
                                *(char**)(opt.output_ptr) = opt.long_name; // Bit maniplation, casting a void* to a pointer to a char* (string).
                            case NOCC_APT_UNKNOWN:
                            default:
                                break;
                            }
                        }
                        continue;
                    }
                    
                    if(arg[1] == opt.short_name) {
                        switch(opt._type) {
                        case NOCC_APT_BOOLEAN:
                            *(bool*)(opt.output_ptr) = true;
                            break;
                        case NOCC_APT_SWITCH:
                            *(char**)(opt.output_ptr) = opt.long_name; // Bit maniplation, casting a void* to a pointer to a char* (string).
                        case NOCC_APT_UNKNOWN:
                        default:
                            nocc_error("Unknown argument type!");
                            break;
                        }
                    }
                }
            }
        }
    }

    return true;
}

/**
 * @brief Parses the options, arguments, and subcommands. If you call this function call this function with the main command rather than a subcommand.
 * This function parses it recursively. So you have to call this function once. And it will do all the work for you.
 * 
 * @param {nocc_argparse_command*} program -- The command to parse.
 * @param {int} argc -- The arg counter passed into main, or __argc.
 * @param {char**} argv -- The variadic arguments passed into main or __argv.
 * 
 * @return {bool}
*/
bool nocc_ap_parse(nocc_argparse_command* program, int argc, char** argv) {
    return _nocc_ap_parse_rec(program, 1, argc, argv);
}

/**
 * @brief Creates the usage string and prints the usage to output.
 * 
 * @param {nocc_argparse_command*} program -- The command to usageify.
 * 
 * @return {bool}
*/
bool nocc_ap_usage(nocc_argparse_command* program) {
    nocc_string usage_string = nocc_str_create();
    
    nocc_str_push_cstr(usage_string, "Usage: ");
    nocc_str_push_cstr(usage_string, program->name);
    if(program->commands) {
        nocc_str_push_char(usage_string, ' ');
        nocc_str_push_cstr(usage_string, "<command>");
    }
    if(program->arguments) {
        nocc_str_push_char(usage_string, ' ');
        nocc_str_push_cstr(usage_string, "[<arguments>]");
    }
    if(program->options) {
        nocc_str_push_char(usage_string, ' ');
        nocc_str_push_cstr(usage_string, "[options]");
    }
    nocc_str_push_cstr(usage_string, "\n\n");

    nocc_str_push_cstr(usage_string, program->description);

    if(program->commands) {
        nocc_str_push_cstr(usage_string, "\n\n");
        nocc_str_push_cstr(usage_string, "Commands:\n");
        for(size_t i = 0; i < program->commands_size; i++) {
            nocc_str_push_char(usage_string, '\t');
            nocc_str_push_cstr(usage_string, program->commands[i].name);
            nocc_str_push_cstr(usage_string, "\t\t\t\t");
            nocc_str_push_cstr(usage_string, program->commands[i].description);
            nocc_str_push_char(usage_string, '\n');
        }
    }

    if(program->arguments) {
        nocc_str_push_char(usage_string, '\n');
        nocc_str_push_cstr(usage_string, "Arguments\n");
        for(size_t i = 0; i < program->arguments_size; i++) {
            // TODO: Print arguments here
        }
    }

    if(program->options) {
        nocc_str_push_cstr(usage_string, "\n\n");
        nocc_str_push_cstr(usage_string, "Options:\n");
        for(size_t i = 0; i < program->options_size; i++) {
            nocc_str_push_cstr(usage_string, "\t-");
            nocc_str_push_char(usage_string, program->options[i].short_name);
            nocc_str_push_cstr(usage_string, ", --");
            nocc_str_push_cstr(usage_string, program->options[i].long_name);
            nocc_str_push_cstr(usage_string, "\t\t\t");
            nocc_str_push_cstr(usage_string, program->options[i].description);
            nocc_str_push_char(usage_string, '\n');
        }
    }

    nocc_str_push_null(usage_string);

    printf("%s", usage_string);

    nocc_str_free(usage_string);
    return true;
}

// Argument Parsing End ===================================================

// File Begins ============================================================

typedef enum {
    NOCC_FT_UNKNOWN,
    NOCC_FT_DIRECTORY,
    NOCC_FT_FILE
} nocc_file_type;

nocc_file_type _nocc_get_file_type(const char* filepath);
bool _nocc_read_dir_single_dir(const char* src_dir, const char*** files);
char* _nocc_get_basename(const char* dir, char* output);

// dirent.h =====================================================================
/**
 * 
 * Mini implmementation of dirent.h for Windows OS
 * 
 */
#ifdef _WIN32

struct dirent {
    char d_name[MAX_PATH + 1];
};

typedef struct {
    HANDLE hFind;
    WIN32_FIND_DATA data;
    struct dirent *dirent;
} DIR;

int closedir(DIR* dir);
DIR* opendir(const char* name);
struct dirent* readdir(DIR* dir);

// Technically not part of dirent.h, but i don't care.
char *basename(const char* path, char* basename_out);

#endif
// dirent.h =====================================================================

/**
 * Creates a directory if the folder does note exist
 * 
 * @param {const char*} dirname -- the directory name to create.
 * @return {bool} return's false if the directory wasn't created, however, if it exists it returns true.
 * 
 * TODO: Make this recursive... No idea how.
 */
bool nocc_mkdir_if_not_exists(const char* dirname) {
    int status;
#ifdef _WIN32
    status = _mkdir(dirname);
#else
    status = mkdir(dirname, 0x777);
#endif
    if(status == -1) {
        if(errno == EEXIST) {
            nocc_trace("Dir %s", dirname);
            return true;
        }
        nocc_error("Failed to create dir %s: %s", dirname, strerror(errno));
        return false;
    }
    return true;

}

/**
 * Recursively obtain all source files
 * 
 * @param {const char*} src_dir -- the directory to obtain all source files from
 * @param {const char**} files -- the files in the directory
 * 
 * @return {boolean}
 * 
 */
bool nocc_read_dir(const char* src_dir, const char*** array_of_files_out) {
    nocc_darray(const char*) array_of_files_in_cwd = nocc_da_create(const char*);
    _nocc_read_dir_single_dir(src_dir, &array_of_files_in_cwd);
    for(size_t i = 0; i < nocc_da_size(array_of_files_in_cwd); i++) {
        if(strcmp(array_of_files_in_cwd[i], ".") == 0)  continue;
        if(strcmp(array_of_files_in_cwd[i], "..") == 0) continue;

        nocc_string dir = nocc_str_create();
        nocc_str_push_cstr(dir, src_dir);
        nocc_str_push_char(dir, '/');
        nocc_str_push_cstr(dir, array_of_files_in_cwd[i]);
        nocc_str_push_null(dir);

        nocc_file_type type = _nocc_get_file_type(dir);
        switch (type)
        {
        case NOCC_FT_FILE:
            nocc_da_push(*array_of_files_out, dir);
            break;

        case NOCC_FT_DIRECTORY:
            nocc_read_dir(dir, array_of_files_out);
            break;

        case NOCC_FT_UNKNOWN:
        default:
            break;
        }

        nocc_str_free(dir);
    }
    nocc_da_free(array_of_files_in_cwd);
    return true;
}

/**
 * 
 * 
 * TODO: Change this function signature to make it more modifiable ...
*/
bool nocc_generate_object_files(const char** array_of_source_files, const char* obj_dir, char*** array_of_obj_files) {
    *array_of_obj_files = nocc_da_reserve(*array_of_obj_files, nocc_da_size(array_of_source_files));
    for(size_t i = 0; i < nocc_da_size(array_of_source_files); i++) {
        char base_name[1024] = "";
        _nocc_get_basename(array_of_source_files[i], base_name);

        nocc_string string = nocc_str_create();
        nocc_str_push_cstr(string, obj_dir);
        nocc_str_push_char(string, '/');
        nocc_str_push_cstr(string, base_name);
        nocc_str_push_cstr(string, ".o");

        nocc_da_push(*array_of_obj_files, string);
    }
}

// Command Begin
// #define nocc_cmd_add(cmd, ...)          nocc_da_push_many(cmd, ##__VA_ARGS__)
#define nocc_cmd_add(cmd, ...)          nocc_da_pushn(cmd, sizeof((const char*[]){__VA_ARGS__}) / sizeof(const char*), ((const char*[]){__VA_ARGS__}))
#define nocc_cmd_addn(cmd, n, a)        nocc_da_pushn(cmd, n, a)

bool nocc_cmd_execute(nocc_darray(const char*) cmd) {
    nocc_string built_command = nocc_str_create();
    for(size_t i = 0; i < nocc_da_size(cmd); i++) {
        nocc_str_push_cstr(built_command, cmd[i]);
        nocc_str_push_char(built_command, ' ');
    }
    nocc_str_push_null(built_command);

    int status = system(built_command);
    if(status == -1) {
        nocc_str_free(built_command);
        return false;
    }

    nocc_str_free(built_command);
    return true;
}


// Command Ends

// IMPLEMENTATION OF EXTERNAL FUNCTIONS ARE HERE

#define _NOCC_USE_ARRAY_IMPLEMENTATION 1
#if _NOCC_USE_ARRAY_IMPLEMENTATION
void* _nocc_da_reserve(size_t stride, size_t cap) {
    size_t header_size = sizeof(_nocc_da_header);
    size_t body_size = cap * stride;
    
    void* array = calloc(1, header_size + body_size);
    nocc_assert(array, "Failed to create an array");

    _nocc_da_header* header = array;
    header->capacity = cap;
    header->size = 0;
    header->stride = stride;
    
    return (void*)((uint8_t*)array + header_size); 
}

void _nocc_da_free(void* array) {
    _nocc_da_header* header = _nocc_da_calc_header(array);
    free(header);

}

void* _nocc_da_push(void* array, void* value) {
    nocc_assert(array, "Please enter a valid array");
    nocc_assert(value, "Please enter a valid value (entered NULL)");

    _nocc_da_header* header = _nocc_da_calc_header(array);
    if(header->size >= header->capacity)
        array = _nocc_da_grow(array, header->capacity * 2);

    header = _nocc_da_calc_header(array);
    
    uint64_t addr = (uint64_t)array;
    addr += (header->size * header->stride);
    memcpy((void*)addr, value, header->stride);

    header->size++;

    return array;
}

void* _nocc_da_pushn(void* array, size_t n, void* value) {
    nocc_assert(array, "Please enter a valid array");
    nocc_assert(value, "Please enter a valid value (entered NULL)");
    nocc_assert(n > 0, "Please enter a valid value (entered NULL)");

    _nocc_da_header* header = _nocc_da_calc_header(array);
    if(header->size + n >= header->capacity)
        array = _nocc_da_grow(array, (header->capacity * 2) + n);
    
    header = _nocc_da_calc_header(array);
    
    uint64_t addr = (uint64_t)array;
    addr += (header->size * header->stride);
    memcpy((void*)addr, value, n * header->stride);

    header->size += n;
    return array;
}

void* _nocc_da_grow(void* array, size_t new_capacity) {
    nocc_assert(array, "Please enter a valid array");
    _nocc_da_header* header = _nocc_da_calc_header(array);
    size_t stride = header->stride;
    size_t header_size = sizeof(_nocc_da_header); 
    size_t total_size = header_size + (stride * new_capacity);
    header->capacity = new_capacity;

    void* temp = realloc(header, total_size);
    nocc_assert(temp, "Failed to reallocate array");


   return (void*)((uint8_t*)temp + header_size); 
}

size_t _nocc_da_size(void* array) {
    nocc_assert(array, "Please enter a valid array");
    _nocc_da_header* header = _nocc_da_calc_header(array);
    return header->size;
}

size_t _nocc_da_capacity(void* array) {
    nocc_assert(array, "Please enter a valid array");
    _nocc_da_header* header = _nocc_da_calc_header(array);
    return header->capacity;
}

size_t _nocc_da_stride(void* array) {
    nocc_assert(array, "Please enter a valid array");
    _nocc_da_header* header = _nocc_da_calc_header(array);
    return header->stride;
}
#endif // _NOCC_USE_ARRAY_IMPLEMENTATION

// FILE IMPLEMENTATION 

nocc_file_type _nocc_get_file_type(const char* filepath) {
#ifdef _WIN32
    DWORD attribute = GetFileAttributesA(filepath);
    if(attribute == INVALID_FILE_ATTRIBUTES) {
        nocc_error("Failed to get the file attribute %s: %S", filepath, GetLastError());
        return NOCC_FT_UNKNOWN;
    }

    if(attribute & FILE_ATTRIBUTE_DIRECTORY) return NOCC_FT_DIRECTORY;
    return NOCC_FT_FILE;
#else
    struct stat statbuf;
    if (stat(path, &statbuf) < 0) {
        nocc_log_error("Could not get stat of %s: %s", path, strerror(errno));
        return -1;
    }

    switch (statbuf.st_mode & S_IFMT) {
        case S_IFDIR:  return NOCC_FT_DIRECTORY;
        case S_IFREG:  return NOCC_FT_FILE;
        default:       return NOCC_FT_UNKNOWN;
    }
#endif
}

bool _nocc_read_dir_single_dir(const char* src_dir, const char*** files) {
    DIR* dir = NULL;

    dir = opendir(src_dir);
    if(dir == NULL) {
        nocc_error("Failed to open file %s: %s", src_dir, strerror(errno));
        return false;
    }

    errno = 0;
    struct dirent* ent = readdir(dir);
    while(ent != NULL) {
        nocc_da_push(*files, strdup(ent->d_name));
        ent = readdir(dir);
    }

    if (errno != 0) {
        nocc_error("Could not read directory %s: %s", src_dir, strerror(errno));
    }

    if(dir)
        closedir(dir);
    return true;
}

char* _nocc_get_basename(const char* dir, char* output) {
#ifdef _WIN32
    return basename(dir, output);
#else
    return basename(dir);
#endif
}

/**
 * DIRENT.H
*/
#ifdef _WIN32
int closedir(DIR* dir) {
    nocc_assert(dir, "Please pass a valid dir object");

    if(!FindClose(dir->hFind)) {
        errno = ENOSYS;
        return -1;
    }

    if(dir->dirent) {
        free(dir->dirent);
    }

    free(dir);

    return 0;
}

DIR* opendir(const char* name) {
    nocc_assert(name, "name cannot be null");

    char buffer[MAX_PATH];
    snprintf(buffer, MAX_PATH, "%s\\*", name);

    DIR* dir = (DIR*)calloc(1, sizeof(DIR));
    
    dir->hFind = FindFirstFile(buffer, &dir->data);
    if(dir->hFind == INVALID_HANDLE_VALUE) {
        errno = ENOSYS;
        goto fail;
    }

    return dir;

fail:
    if(dir) {
        free(dir);
    }

    return NULL;
}

struct dirent* readdir(DIR* dir) {
    nocc_assert(dir, "Please pass a valid dir object");

    if(dir->dirent == NULL) {
        dir->dirent = (struct dirent*)calloc(1, sizeof(struct dirent));
    } else {
        if(!FindNextFile(dir->hFind, &dir->data)) {
            if(GetLastError() != ERROR_NO_MORE_FILES) {
                errno = ENOSYS;
            }

            return NULL;
        }
    }

    memset(dir->dirent->d_name, 0, sizeof(dir->dirent->d_name));

    strncpy(dir->dirent->d_name, dir->data.cFileName, sizeof(dir->dirent->d_name) - 1);

    return dir->dirent;
}

// ./some/random/filepath.c -> file
char *basename(const char* path, char* basename_out) {
    if(path == NULL) return NULL;

    size_t size = strlen(path);
    size_t index_of_last_slash = 0;
    size_t index_of_first_dot = size;
    size_t i = (path[0] == '.' && path[1] == '/') ? 2 : 0;
    
    for(; i < size; i++) {
        char c = path[i];
        if(c == '/' && index_of_last_slash < index_of_first_dot)
            index_of_last_slash = i;
        if(c == '.' && i < index_of_first_dot)
            index_of_first_dot = i;
    }

    memcpy(basename_out, path + index_of_last_slash + 1, index_of_first_dot - index_of_last_slash - 1);

    basename_out[index_of_first_dot - index_of_last_slash - 1] = '\0';

    return basename_out;
}

#endif // _WIN32

#endif // _NO_C_COMPILER