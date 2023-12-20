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
    #include <unistd.h>
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
void* _nocc_da_remove(void* array, size_t index, void* ouput_ptr);
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
#define nocc_da_free(a)                         _nocc_da_free(a)

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
 * @brief Removes the element from the array
 * 
 * @param {void*} a -- The array
 * @param {size_t} index -- The index of the array to remove
 * @param {void*} output_ptr -- the pointer to the element, that was removed 
 * 
 * @return {void}
*/
#define nocc_da_remove(a, i, op)               { a = _nocc_da_remove((a), (i), (op)); }

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
#define nocc_str_push_cstr(str, s)          nocc_da_pushn(str, strlen(s), s)

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
    NOCC_APT_SWITCH,  
} _nocc_argparse_type;

#define _USE_NEW_ARGPARSE_ 
typedef enum {
    NOCC_APK_UNKNOWN = 0,
    NOCC_APK_OPTION, NOCC_APK_ARGUMENT, NOCC_APK_COMMAND
} _nocc_argparse_kind;

typedef struct nocc_argparse_opt {
    const char* name;             // long_name for your options;
    const char* description;
    void* output_ptr;

    // internal        
    _nocc_argparse_kind _kind;
    void* default_;

    union {
        // Options
        struct {
            char short_name;
            _nocc_argparse_type _type;
            struct nocc_argparse_opt* _children;
            size_t _length_children;
        };

        // Argument

        // Command
        struct {
            nocc_darray(struct nocc_argparse_opt) arguments;
            size_t arguments_size;
            nocc_darray(struct nocc_argparse_opt) options;
            size_t options_size;
            nocc_darray(struct nocc_argparse_opt) commands;
            size_t commands_size;
        };
    };

} nocc_argparse_opt;

#define nocc_ap_opt_boolean(sn, ln, desc, def, op) { ._kind=NOCC_APK_OPTION, ._type=NOCC_APT_BOOLEAN, .short_name=(sn), .name=(ln), .description=(desc), .default_=(def), .output_ptr=(op), ._children=NULL, ._length_children=0 }
#define nocc_ap_opt_switch(a, def, op) { ._kind=NOCC_APK_OPTION, ._type=NOCC_APT_SWITCH, .short_name=0, .name=NULL, .description=NULL, .default_=(def), .output_ptr=(op), ._children=(a), ._length_children=(sizeof(a) / sizeof(nocc_argparse_opt)) }

#define nocc_ap_arg_string(n, d, def, op) { ._kind=NOCC_APK_ARGUMENT, ._type=NOCC_APT_STRING, .name=(n), .description=(d), .default_=(def), .output_ptr=(op) }

#define nocc_ap_cmd(n, d, o, a, c, op) {                                                \
    ._kind = NOCC_APK_COMMAND,                                                          \
    .name = (n),                                                                        \
    .description = (d),                                                                 \
    .options = (o), .options_size = (sizeof(o) / sizeof(nocc_argparse_opt)),            \
    .arguments = (a), .arguments_size = (sizeof(a) / sizeof(nocc_argparse_opt)),        \
    .commands = (c), .commands_size = (sizeof(c) / sizeof(nocc_argparse_opt)),          \
    .default_ = NULL,                                                                   \
    .output_ptr = (op)                                                                  \
}

bool _nocc_ap_parse_rec(nocc_argparse_opt* command, int beg, nocc_darray(char*) args);
inline bool _nocc_ap_find_if_is_long(char c);
bool _nocc_ap_get_option_status(nocc_argparse_opt* opt, bool is_long, char* arg);
bool _nocc_ap_parse_option(nocc_argparse_opt* command, int beg, nocc_darray(char*) args, char* arg);
bool _nocc_ap_parse_argument(nocc_argparse_opt* command, int beg, nocc_darray(char*) args, char* arg);
void _nocc_ap_set_default_option(nocc_argparse_opt* command);
void _nocc_ap_set_default_argument(nocc_argparse_opt* command);
void _nocc_ap_set_default(nocc_argparse_opt* command);

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
bool nocc_ap_parse(nocc_argparse_opt* program, int argc, char** argv) {
    nocc_darray(char*) args = nocc_da_reserve(char*, argc - 1);
    nocc_da_pushn(args, argc - 1, argv + 1);

    bool status = _nocc_ap_parse_rec(program, 0, args);

    nocc_da_free(args);
    return status;
}

/**
 * @brief Creates the usage string and prints the usage to output.
 * 
 * @param {nocc_argparse_opt*} program -- The command to usageify.
 * 
 * @return {bool}
*/
bool nocc_ap_usage(nocc_argparse_opt* program) {
    if(program->_kind != NOCC_APK_COMMAND) return false;

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
        nocc_str_push_cstr(usage_string, "\n\n");
        nocc_str_push_cstr(usage_string, "Arguments:\n");
        for(size_t i = 0; i < program->arguments_size; i++) {
            nocc_str_push_char(usage_string, '\t');
            nocc_str_push_cstr(usage_string, program->arguments[i].name);
            nocc_str_push_cstr(usage_string, "\t\t\t");
            nocc_str_push_cstr(usage_string, program->arguments[i].description);
            if(program->arguments[i].default_) {
                nocc_str_push_cstr(usage_string, " (default=");
                nocc_str_push_cstr(usage_string, program->arguments[i].default_);
                nocc_str_push_char(usage_string, ')');
            }
            nocc_str_push_char(usage_string, '\n');

        }
    }

    if(program->options) {
        nocc_str_push_cstr(usage_string, "\n\n");
        nocc_str_push_cstr(usage_string, "Options:\n");
        for(size_t i = 0; i < program->options_size; i++) {
            if(program->options[i]._type == NOCC_APT_SWITCH) {
                for(size_t j = 0; j < program->options[i]._length_children; j++) {
                    nocc_str_push_cstr(usage_string, "\t-");
                    nocc_str_push_char(usage_string, program->options[i]._children[j].short_name);
                    nocc_str_push_cstr(usage_string, ", --");
                    nocc_str_push_cstr(usage_string, program->options[i]._children[j].name);
                    nocc_str_push_cstr(usage_string, "\t\t\t");
                    nocc_str_push_cstr(usage_string, program->options[i]._children[j].description);
                    nocc_str_push_char(usage_string, '\n');
                }
            } else {
                    nocc_str_push_cstr(usage_string, "\t-");
                    nocc_str_push_char(usage_string, program->options[i].short_name);
                    nocc_str_push_cstr(usage_string, ", --");
                    nocc_str_push_cstr(usage_string, program->options[i].name);
                    nocc_str_push_cstr(usage_string, "\t\t\t");
                    nocc_str_push_cstr(usage_string, program->options[i].description);
                    nocc_str_push_char(usage_string, '\n');
            }
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
 * @param {const char*} file_extension -- the file extension to keep
 * @param {const char**} files  -- the files in the directory
 *  
 * @return {boolean}
 */
bool nocc_read_dir(const char* src_dir, const char* file_extension, const char*** array_of_files_out) {
    nocc_darray(const char*) array_of_files_in_cwd = nocc_da_create(const char*);
    _nocc_read_dir_single_dir(src_dir, &array_of_files_in_cwd);
    for(size_t i = 0; i < nocc_da_size(array_of_files_in_cwd); i++) {
        if(strcmp(array_of_files_in_cwd[i], ".") == 0)  continue;
        if(strcmp(array_of_files_in_cwd[i], "..") == 0) continue;

        // TODO: Need a better way of dealing with this.
        nocc_string dir = nocc_str_create();
        nocc_str_push_cstr(dir, src_dir);
        nocc_str_push_char(dir, '/');
        nocc_str_push_cstr(dir, array_of_files_in_cwd[i]);
        nocc_str_push_null(dir);

        nocc_file_type type = _nocc_get_file_type(dir);
        switch (type)
        {
        case NOCC_FT_FILE:
            // Step 1 get the file extension
            // TODO: Refactor this code out of the funciton
            char* it = dir + 1;
            for(; it != '\0'; it++) {
                if(*it == '.') {
                    it++;
                    break;
                }
            }

            if(strcmp(it, file_extension) == 0) {
                nocc_da_push(*array_of_files_out, strdup(dir));
            }
            break;

        case NOCC_FT_DIRECTORY:
            nocc_read_dir(dir, file_extension, array_of_files_out);
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

nocc_string _nocc_generate_object_file(const char* filename, const char* fmt, ...) {
    nocc_string obj_file = nocc_str_create();
    va_list args;
    va_start(args, fmt);

    for(const char* it = fmt; *it != '\0'; it++) {
        if(*it != '%') {
            nocc_str_push_char(obj_file, *it);
            continue;
        }
    
        it++;

        switch(*it) {
        case 'n':
            char base_name[1024] = "";
            _nocc_get_basename(filename, base_name);
            nocc_str_push_cstr(obj_file, base_name);
            break;

        case 's':
            const char* string = va_arg(args, const char*);
            nocc_str_push_cstr(obj_file, string);
            break;
        case '%':
            nocc_str_push_char(obj_file, *it);
            break;
        default:
            break;
        }
    }
    nocc_str_push_null(obj_file);

    va_end(args);
    return obj_file;
}

/**
 * If you used Makefile then it's the patsubst function
 * 
 * fmt:
 *  %n -- The name of the file. The name is automatically deduced from the array of files.
 * 
 * @brief generates the object files based on format specified.
 * 
 * @param {nocc_darray(nocc_string)} array_of_object_files
 * @param {nocc_darray(const char*)} array_of_source_files
 * @param {const char*} fmt -- the filepathname
 * 
 * @return {bool} true if successfully generated the object file names
 * 
*/
#define nocc_generate_object_files(array_of_object_files, array_of_source_files, fmt, ...) {                                    \
    array_of_object_files = nocc_da_reserve(nocc_string, nocc_da_size(array_of_source_files));                                  \
    for(size_t i = 0; i < nocc_da_size(array_of_source_files); i++) {                                                           \
        nocc_string obj_file = _nocc_generate_object_file(array_of_source_files[i], fmt, ##__VA_ARGS__);                        \
        nocc_da_push(array_of_object_files, obj_file);                                                                          \
    }                                                                                                                           \
}

// Command Begin
#define nocc_cmd_add(cmd, ...)          nocc_da_pushn(cmd, sizeof((const char*[]){__VA_ARGS__}) / sizeof(const char*), ((const char*[]){__VA_ARGS__}))
#define nocc_cmd_addn(cmd, n, a)        nocc_da_pushn(cmd, n, a)

#ifdef _WIN32
    typedef HANDLE pid;
#else // _WIN32
    typedef pid_t pid;
#endif // _WIN32

void _nocc_cmd_pid_wait(pid pid) {
#ifdef _WIN32
    DWORD result = WaitForSingleObject(pid, INFINITE);

    if(result == WAIT_FAILED) {
        nocc_assert(false, "Could not wait for child process %s", GetLastError());
        return;
    }

    DWORD exit_code;
    if(GetExitCodeProcess(pid, &exit_code) == 0) {
        nocc_assert(false, "Could not get the exit code %lu", GetLastError());
        return;
    }

    if(exit_code != 0) {
        nocc_assert(false, "Exit code recieved %d", exit_code);
        return;
    }

    CloseHandle(pid);
#else
    for(;;) {
        int wstatus = 0;
        if(waitpid(pid, &wstatus, 0) < 0) {
            nocc_assert(false, "Could not wait for child process %s", strerror(errno));
            return;
        }

        if(WIFEXITED(wstatus)) {
            int exit_code = WEXITSTATUS(wstatus);
            if (exit_status != 0) {
                nocc_assert(false, "Exited with exit code %d", exit_status);
            }

            break;
        }
    }

    if (WIFSIGNALED(wstatus)) {
        nocc_assert(false, "command process was terminated by %s", strsignal(WTERMSIG(wstatus)));
    }
#endif // _WIN32
}

pid _nocc_cmd_run_command_async(nocc_darray(const char*) cmd) {
#ifdef _WIN32
    nocc_string built_command = nocc_str_create();
    for(size_t i = 0; i < nocc_da_size(cmd); i++) {
        nocc_str_push_cstr(built_command, cmd[i]);
        nocc_str_push_char(built_command, ' ');
    }
    nocc_str_push_null(built_command);

    STARTUPINFO siStartInfo;
    ZeroMemory(&siStartInfo, sizeof(siStartInfo));
    siStartInfo.cb = sizeof(STARTUPINFO);
    // NOTE: theoretically setting NULL to std handles should not be a problem
    // https://docs.microsoft.com/en-us/windows/console/getstdhandle?redirectedfrom=MSDN#attachdetach-behavior
    siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    // TODO: check for errors in GetStdHandle
    siStartInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    siStartInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION piProcInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    BOOL bSuccess =
        CreateProcess(
            NULL,
            built_command,
            NULL,
            NULL,
            TRUE,
            0,
            NULL,
            NULL,
            &siStartInfo,
            &piProcInfo
        );

    if (!bSuccess) {
        // TODO: Improve error handling
        nocc_assert(false, "Failed to fork child process");
        return NULL;
    }

    CloseHandle(piProcInfo.hThread);

    nocc_str_free(built_command);
    return piProcInfo.hProcess;
#else // ifndef _WIN32
    pid_t cpid = fork();
    if(cpid == -1) {
        nocc_assert(false, "Failed to fork child process %s", strerror(errno));
        return -1;
    }

    if(cpid == 0) {
        if(execvp(cmd[i], cmd + 1) == -1) {
            nocc_assert(false, "Failed to execute cmd %s", strerror(errno));
            return -1;
        }
    }

    return cpid;

#endif // _WIN32
}

bool nocc_cmd_execute(nocc_darray(const char*) cmd) {
    _nocc_cmd_pid_wait(_nocc_cmd_run_command_async(cmd));
    return true;
}

/**
 * @brief determines whether the file should be recompiled or not.
 * 
 * @param {const char**} inputfiles -- an array (or pointer to) a filename
 * @param {size_t} input_files_size -- the length of the input files array (or 1) if it is a pointer.
 * @param {const char*} outputfile  -- the name of the target file 
 * 
 * @return {bool} return's true, if needs to rebuild 
*/
bool nocc_should_recompile(const char** inputfiles, size_t input_files_size, const char* outputfile) {
#ifdef _WIN32
    BOOL status;
    
    HANDLE output_file_fd = CreateFile(outputfile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
    if(output_file_fd == INVALID_HANDLE_VALUE) {
        if(GetLastError() == ERROR_FILE_NOT_FOUND) return true;
        nocc_error("File not found (%s)", outputfile);
        return true;
    }

    FILETIME output_file_time;
    status = GetFileTime(output_file_fd, NULL, NULL, &output_file_time);
    CloseHandle(output_file_fd);
    if(!status) { 
        nocc_error("Could not obtain file time: %s (%s)", GetLastError(), outputfile);
        return true;
    }

    for(size_t i = 0; i < input_files_size; i++) {
        const char* inputfile = inputfiles[i];

        HANDLE input_file_fd = CreateFile(inputfile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
        if(input_file_fd == INVALID_HANDLE_VALUE) {
            if(GetLastError() == ERROR_FILE_NOT_FOUND) return true;
            nocc_error("File not found (%s)", inputfile);
            return true;
        }

        FILETIME input_file_time;
        status = GetFileTime(input_file_fd, NULL, NULL, &input_file_time);
        CloseHandle(input_file_fd);
        if(!status) { 
            nocc_error("Could not obtain file time: %s (%s)", GetLastError(), inputfile);
            return true;
        }

        if(CompareFileTime(&input_file_time, &output_file_time) == 1) return true;
    }

    return false;
#else
    // of course everything is easier on linux.
    struct stat statbuf = {0};

    if (stat(outputfile, &statbuf) < 0) {
        // NOTE: if output does not exist it 100% must be rebuilt
        if (errno == ENOENT) return true;
        nocc_error("could not stat %s: %s", outputfile, strerror(errno));
        return true;
    }
    int output_file_time = statbuf.st_mtime;

    for (size_t i = 0; i < input_files_size; ++i) {
        const char *inputfile = inputfiles[i];
        if (stat(inputfile, &statbuf) < 0) {
            // NOTE: non-existing input is an error cause it is needed for building in the first place
            nocc_error("could not stat %s: %s", inputfile, strerror(errno));
            return true;
        }
        int input_file_time = statbuf.st_mtime;
        // NOTE: if even a single inputfile is fresher than outputfile that's 100% rebuild
        if (input_file_time > output_file_time) return 1;
    }

    return 0;
#endif // _WIN32
}

bool nocc_should_recompile1(const char* inputfile, const char* outputfile) {
    return nocc_should_recompile(&inputfile, 1, outputfile);
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

void* _nocc_da_remove(void* array, size_t index, void* output_ptr) {
    nocc_assert(array, "Please enter a valid array");
    nocc_assert(index >= 0 && index < nocc_da_size(array));
    _nocc_da_header* header = _nocc_da_calc_header(array);
    
    uint64_t addr = (uint64_t)array;
    if(output_ptr != NULL) {
        memcpy(output_ptr, (array + index), header->stride);
    }

    if(index != header->size - 1) {
        memmove(
            (void*)(addr + (index * header->stride)),
            (void*)(addr + ((index + 1) * header->stride)),
            header->stride * (header->size - (index - 1))
        );
    }

    header->size--;
    return array;
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

// ARGPARSE IMPLEMENTATION BEGIN

bool _nocc_ap_parse_rec(nocc_argparse_opt* command, int beg, nocc_darray(char*) args) {
    nocc_assert(command, "command cannot be NULL");
    nocc_assert(args, "argv cannot be NULL");

    while(nocc_da_size(args) != 0) {
        char* arg = args[0];

        if (command->_kind != NOCC_APK_COMMAND) {
            nocc_assert(false, "Unknown argparse kind");
            return false;
        } 
        
        if(command->commands) {
            nocc_argparse_opt cmd;
            for(size_t i = 0; i < command->commands_size; i++) {
                cmd = command->commands[i];

                if(strcmp(cmd.name, arg) != 0)
                    continue;
                
                *(bool*)(cmd.output_ptr) = true;
                nocc_da_remove(args, beg, NULL);
                return _nocc_ap_parse_rec(&cmd, beg, args);
            }
        }

        if(_nocc_ap_parse_option(command, beg, args, arg)) {
            goto next_iteration;
        }

        _nocc_ap_parse_argument(command, beg, args, arg);

    next_iteration:
        continue;
    }

    _nocc_ap_set_default(command);

    return true;
}

bool _nocc_ap_find_if_is_long(char c) {
    return c == '-';
}

bool _nocc_ap_get_option_status(nocc_argparse_opt* opt, bool is_long, char* arg) {
    if(is_long == false) {
        if(opt->short_name != arg[1]) {
            return false;
        }
    } else {
        if(strcmp(opt->name, arg + 2) != 0) {
            return false;
        } 
    }
    
    return true;
}

bool _nocc_ap_parse_option(nocc_argparse_opt* command, int beg, nocc_darray(char*) args, char* arg) {
    if(command->options == NULL) return false;
    if(arg[0] != '-') return false;
    
    for(size_t i = 0; i < command->options_size; i++) {
        nocc_argparse_opt opt = command->options[i];
        bool is_long = _nocc_ap_find_if_is_long(arg[1]);

        switch (opt._type)
        {
        case NOCC_APT_BOOLEAN: {
            bool status = _nocc_ap_get_option_status(&opt, is_long, arg);
            if(!status)
                break;
            *(bool*)(opt.output_ptr) = true;
            nocc_da_remove(args, beg, NULL);
            return true;

        } break;
        case NOCC_APT_SWITCH: {
            // switch has suboptions
            for(size_t i = 0; i < opt._length_children; i++) {
                bool status = _nocc_ap_get_option_status(&(opt._children[i]), is_long, arg);
                if(!status)
                    continue;
                *(char**)(opt.output_ptr) = opt._children[i].name;
                nocc_da_remove(args, beg, NULL);
                return true;
            }
        } break;
        default:
            break;
        }
    }

    return false;
}

bool _nocc_ap_parse_argument(nocc_argparse_opt* command, int beg, nocc_darray(char*) args, char* arg) {
    if(command->arguments == NULL) return false;

    for(size_t i = 0; i < command->arguments_size; i++) {
        nocc_argparse_opt argument = command->arguments[i];

        *(char**)(argument.output_ptr) = arg;
        nocc_da_remove(args, beg, NULL);
        return true;
    }

    return false;
}

void _nocc_ap_set_default_option(nocc_argparse_opt* command) {
    if(command->options == NULL) return;

    for(size_t i = 0; i < command->options_size; i++) {
        nocc_argparse_opt opt = command->options[i];
        
        if(opt.default_ == NULL)
            continue;

        switch (opt._type)
        {
        case NOCC_APT_BOOLEAN:
            if(opt.output_ptr != NULL)
                break;
            *(bool*)opt.output_ptr = *(bool*)opt.default_;
            break;

        case NOCC_APT_STRING:
        case NOCC_APT_SWITCH:
            if(*(char**)opt.output_ptr != NULL)
                break;
            *(char**)opt.output_ptr = (char*)opt.default_;
            break;
        
        case NOCC_APT_FLOAT:
        case NOCC_APT_NUMBER:
        case NOCC_APT_UNKNOWN:
        case NOCC_APT_ARRAY:
        default:
            nocc_assert(false, "Unknown type");
            break;
        }
    }

}

void _nocc_ap_set_default_argument(nocc_argparse_opt* command) {
    if(command->arguments == NULL) return;

    for(size_t i = 0; i < command->arguments_size; i++) {
        nocc_argparse_opt arg = command->arguments[i];
        if(*(char**)arg.output_ptr == NULL && arg.default_) {
            *(char**)(arg.output_ptr) = arg.default_;
        }
    }
}

void _nocc_ap_set_default(nocc_argparse_opt* command) {
    _nocc_ap_set_default_option(command);
    _nocc_ap_set_default_argument(command);
}

// END ARGPARSE IMPLEMENTATION BEGIN

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
    size_t index_of_first_dot = size;
    size_t i = (path[0] == '.' && path[1] == '/') ? 2 : 0;
    size_t index_of_last_slash = i;
    
    for(; i < size; i++) {
        char c = path[i];
        if(c == '/' && index_of_last_slash < index_of_first_dot)
            index_of_last_slash = i;
        if(c == '.' && i < index_of_first_dot)
            index_of_first_dot = i;
    }

    memcpy(basename_out, path + index_of_last_slash, index_of_first_dot - index_of_last_slash);

    basename_out[index_of_first_dot - index_of_last_slash] = '\0';

    return basename_out;
}

#endif // _WIN32

#endif // _NO_C_COMPILER