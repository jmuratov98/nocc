#ifndef _NOCC_H_
#define _NOCC_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <errno.h>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN 
    #include <Windows.h>
    #include <direct.h>
    #include <shlwapi.h>
#else
    #include <unistd.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <dirent.h>
    #include <libgen.h>
    #include <fnmatch.h>
#endif

// cstd extensions end
// Note: While this is defined in stdio.h, it's only available for GNU and BSD environments. To make
// nocc more platform-agnostic, I have decided to implement these functions. The following code
// is obtained from https://rextester.com/HUNM43537
#ifndef _vscprintf
int _vscprintf_so(const char* format, va_list pargs);
#endif // _vscprintf

#ifndef vasprintf
int vasprintf(char **strp, const char *fmt, va_list ap);
#endif // vasprintf

#ifndef asprintf
int asprintf(char *strp[], const char *fmt, ...);
#endif // asprintf
// cstd extensions end

// Defines Begin
#define NOCC_VERSION_MAJOR      "0"
#define NOCC_VERSION_MINOR      "3"
#define NOCC_VERSION_PATCH      "0"
#define NOCC_VERSION_PRERC      "alpha"
#define NOCC_VERSION_CORE       NOCC_VERSION_MAJOR "." NOCC_VERSION_MINOR "." NOCC_VERSION_PATCH
#ifdef NOCC_VERSION_PRERC
    #define NOCC_VERSION        NOCC_VERSION_CORE "-" NOCC_VERSION_PRERC
#else
    #define NOCC_VERSION        NOCC_VERSION_CORE
#endif

#define NOCC_INIT_CAP           10

// Defines End

// Logging Begin
typedef enum {
    NOCC_LOG_LEVEL_TRACE, NOCC_LOG_LEVEL_DEBUG, NOCC_LOG_LEVEL_INFO, NOCC_LOG_LEVEL_WARN, NOCC_LOG_LEVEL_ERROR, NOCC_LOG_LEVEL_CRITICAL,
    NOCC_LOG_LEVEL_OFF
} nocc_log_level;

int32_t _nocc_log_output(nocc_log_level, const char *const, ...);

#define nocc_trace(msg, ...)    _nocc_log_output(NOCC_LOG_LEVEL_TRACE, msg, ##__VA_ARGS__)
#define nocc_debug(msg, ...)    _nocc_log_output(NOCC_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)
#define nocc_info(msg, ...)     _nocc_log_output(NOCC_LOG_LEVEL_INFO, msg, ##__VA_ARGS__) 
#define nocc_warn(msg, ...)     _nocc_log_output(NOCC_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)
#define nocc_error(msg, ...)    _nocc_log_output(NOCC_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__) 
#define nocc_critical(msg, ...) _nocc_log_output(NOCC_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)
// Logging End

// Dynamic Array Begin
void* _nocc_darray_reserve(size_t stride, size_t cap);
void  _nocc_darray_free(void* array);
void* _nocc_darray_push(void* array, void* value);
void* _nocc_darray_pushn(void* array, size_t n, void* value);
void* _nocc_darray_remove(void* array, size_t index, void* ouput_ptr);
size_t _nocc_darray_size(void* array);
size_t _nocc_darray_capacity(void* array);
size_t _nocc_darray_stride(void* array);

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
#define nocc_darray_reserve(T, cap)                 _nocc_darray_reserve(sizeof(T), cap)

/**
 * @brief Creates an array with a capacity of 10.
 * 
 * @param {T} type -- the type of the array to reserve
 * 
 * @return {void*} returns the newly constructed array or NULL if the creation failed.
 * 
*/
#define nocc_darray_create(T)                       nocc_darray_reserve(T, NOCC_INIT_CAP)

/**
 * @brief Frees the array. If the elements were allocated on the heap. The user must free them.
 * 
 * @param {void*} array -- the type of the array to reserve
 * 
 * @return {void}
 * 
*/
#define nocc_darray_free(a)                         _nocc_darray_free(a)

/**
 * @brief Pushs the value to the end of the array. Think std::vector::push_back
 * 
 * @param {void*} a -- The array
 * @param {void*} v -- The value to add to the array.
 * 
 * @return {void}
*/
#define nocc_darray_push(a, v) {                \
    typeof((v)) temp = (v);                     \
    a = _nocc_darray_push(a, &temp);            \
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
#define nocc_darray_pushn(a, n, v)                  { a = _nocc_darray_pushn(a, n, v); }

/**
 * @brief Pushs the value to the end of the array. Think std::vector::push_back
 * 
 * @param {void*} a -- The array
 * @param {...} ... -- The values to add to the array.
 * 
 * @return {void}
*/
#define nocc_darray_push_many(a, ...)               _nocc_darray_pushn(a, sizeof((typeof(__VA_ARGS__)[]){__VA_ARGS__}) / nocc_da_stride(a), (typeof(__VA_ARGS__)[]){__VA_ARGS__})

/**
 * @brief Removes the element from the array
 * 
 * @param {void*} a -- The array
 * @param {size_t} index -- The index of the array to remove
 * @param {void*} output_ptr -- the pointer to the element, that was removed 
 * 
 * @return {void}
*/
#define nocc_darray_remove(a, i, op)               { a = _nocc_darray_remove((a), (i), (op)); }

/**
 * @brief returns the size of the array
 * 
 * @param {void*} a -- The array
 * 
 * @return {size_t} The size of the array
 * 
*/
#define nocc_darray_size(a)                 _nocc_darray_size(a)

/**
 * @brief returns the capacity of the array
 * 
 * @param {void*} a -- The array
 * 
 * @return {size_t} The capacity of the array
 * 
*/
#define nocc_darray_capacity(a)             _nocc_darray_capacity(a)

/**
 * @brief returns the stride of the array
 * 
 * @param {void*} a -- The array
 * 
 * @return {size_t} The stride of the array
 * 
*/
#define nocc_darray_stride(a)               _nocc_darray_stride(a)
// Dynamic Array End

// String Begin

#define nocc_string                         char*

char* _nocc_string_reserve(size_t cap);
void _nocc_string_free(char* s);
char* _nocc_string_pushc(nocc_string str, char c);
char* _nocc_string_push(nocc_string str, const char* cstr);
size_t _nocc_string_size(nocc_string str);
size_t _nocc_string_capacity(nocc_string str);

#define nocc_string_create()                nocc_string_reserve(NOCC_INIT_CAP)
#define nocc_string_reserve(cap)            _nocc_string_reserve(cap)
#define nocc_string_free(s)                 _nocc_string_free(s)

#define nocc_string_pushc(s, c)             { s = _nocc_string_pushc(s, c); }
#define nocc_string_push(s, cstr)           { s = _nocc_string_push(s, cstr); }
#define nocc_string_push_null(s)            _nocc_string_pushc(s, '\0')

#define nocc_string_size(s)                 _nocc_string_size(s)
#define nocc_string_capacity(s)             _nocc_string_capacity(s)
// String End

// File Functions Begin
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

#endif
// dirent.h =====================================================================

#ifdef _WIN32
int fnmatch(const char *pattern, const char *string, int flags);
#endif // _WIN32

typedef enum {
    NOCC_FT_FILE,
    NOCC_FT_DIRECTORY,
    NOCC_FT_UNKNOWN
} nocc_file_type;

nocc_file_type _nocc_get_file_type(const char* filepath);
bool nocc_mkdir_if_not_exists(const char* dirname);
nocc_darray(const char*) nocc_read_dir(const char *const filter);
bool nocc_read_dir_i(const char *const filter, nocc_darray(const char*)* array_outp);
nocc_darray(const char*) nocc_generate_objects(nocc_darray(const char*) sources, const char *const fmt, ...);
// File Functions End

// Command Begin

typedef struct {
    const char* cc;                     // compiler, for example gcc, clang, msvc, tsc
    const char* cflags;                 // C flags
    const char* includes;               // -I./include
    const char* defines;                // defines: -D_DEBUG
    const char* src_filter;             //
    const char* inc_filter;             //
    const char* obj_format;             //
} nocc_compilation_options;

typedef struct {
    const char* ld;                     // linker: gcc, clang, etc.
    const char* ldflags;                // ldflags: -lsomelib
    const char* ldadd;                  // ldadd: -Lbin
    nocc_darray(const char*) objs;      // 
    const char* target;                 //
} nocc_link_options;

#ifdef _WIN32
    typedef HANDLE pid;
#else // _WIN32
    typedef pid_t pid;
#endif // _WIN32

#define nocc_command                            nocc_darray(const char*)
#define nocc_command_create()                   nocc_darray_create(const char*)
#define nocc_command_free(cmd)                  nocc_darray_free(cmd)
#define nocc_command_add(cmd, ...)              nocc_darray_pushn(cmd, (sizeof((const char*[]){__VA_ARGS__})/sizeof(const char*)), ((const char*[]){__VA_ARGS__}))
#define nocc_command_addn(cmd, n, a)            nocc_darray_pushn(cmd, n, a)

bool nocc_command_execute(nocc_command cmd);
bool nocc_should_recompile_many(nocc_darray(const char*) inputfiles, nocc_darray(const char*) headerfiles, const char* outputfile);
bool nocc_should_recompile(const char* inputfile, nocc_darray(const char*) headerfiles, const char* outputfile);

nocc_darray(const char*) nocc_compile(nocc_compilation_options opts);
bool nocc_compile(nocc_link_options opts);

// Command End

// cstd extensions begin
// Note: While this is defined in stdio.h, it's only available for GNU and BSD environments. To make
// nocc more platform-agnostic, I have decided to implement these functions. The following code
// is obtained from https://rextester.com/HUNM43537
#ifndef _vscprintf
int _vscprintf_so(const char* format, va_list pargs) {
    int retval;
    va_list argcopy;
    va_copy(argcopy, pargs);
    retval = vsnprintf(NULL, 0, format, argcopy);
    va_end(argcopy);
}
#endif // _vscprintf

#ifndef vasprintf
int vasprintf(char **strp, const char *fmt, va_list ap) {
    int len = _vscprintf_so(fmt, ap);
    if (len == -1) return -1;
    char *str = malloc((size_t) len + 1);
    if (!str) return -1;
    int r = vsnprintf(str, len + 1, fmt, ap); /* "secure" version of vsprintf */
    if (r == -1) return free(str), -1;
    *strp = str;
    return r;
}
#endif // vasprintf

#ifndef asprintf
int asprintf(char *strp[], const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = vasprintf(strp, fmt, ap);
    va_end(ap);
    return r;
}
#endif // asprintf
// cstd extensions end

// Logging Implementation Start
void _nocc_log_platform_output(nocc_log_level, const char *const);

int32_t _nocc_log_output(nocc_log_level level, const char *const fmt, ...) {
    static const char* levels[NOCC_LOG_LEVEL_OFF] = { "trace", "debug", "info", "warn", "error", "critical" };

    // format the string and dynamically allocate it, rather than inputting the size.
    char *formatted_message;
    va_list args;
    va_start(args, fmt);
    int result = vasprintf(&formatted_message, fmt, args);
    if(result == -1) {
        printf("Failed to format the string");
        return -1;
    }
    va_end(args);

    char *outputted_message;
    result = asprintf(&outputted_message, "[%s]: %s\n", levels[level], formatted_message);
    if(result == -1) {
        printf("Failed to format the string");
        free(formatted_message);
        return -1;
    }

    free(formatted_message);

    _nocc_log_platform_output(level, outputted_message);

    free(outputted_message);

    return result;
}

void _nocc_log_platform_output(nocc_log_level level, const char *const msg) {
#ifdef _WIN32
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    static uint8_t levels[6] = {
        FOREGROUND_INTENSITY,
        FOREGROUND_BLUE,
        FOREGROUND_GREEN,
        FOREGROUND_RED | FOREGROUND_GREEN,
        FOREGROUND_RED,
        BACKGROUND_RED | BACKGROUND_GREEN | FOREGROUND_RED
    };

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    
    // Get the current console color
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
    WORD originalColor = consoleInfo.wAttributes;

    // Set the new console color
    SetConsoleTextAttribute(hConsole, levels[level]);
    
    // Print the message
    uint64_t length = strlen(msg);
    DWORD number_written = 0;
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), msg, (DWORD)length, &number_written, 0);

    // Reset the console color to the original color
    SetConsoleTextAttribute(hConsole, originalColor);
#else 
    const char* colour_strings[] = {"1;30", "1;34", "1;32", "1;33", "1;31", "0;41"};
    printf("\033[%sm%s\033[0m", colour_strings[level], message);
#endif // _WIN32
}

// Logging Implementation End

// Dynamic Array Implementation Begin
typedef struct {
    size_t capacity, size, stride;
} _nocc_darray_header;

// A helper function to calculate the head of the pointer. This is private and should not be utilized
#define _nocc_darray_calculate_header(a) (_nocc_darray_header*)((uint8_t*)(a) - sizeof(_nocc_darray_header))

void* _nocc_darray_resize(void* array, size_t new_size);

void* _nocc_darray_reserve(size_t stride, size_t cap) {
    size_t header_size = sizeof(_nocc_darray_header);
    size_t body_size = cap * stride;
    void* array = malloc(header_size + body_size);
    _nocc_darray_header* header = array;
    header->size = 0;
    header->capacity = cap;
    header->stride = stride;
    return (void*)((uint8_t*)(array) + header_size);
}

void _nocc_darray_free(void* array) {
    _nocc_darray_header* header = _nocc_darray_calculate_header(array);
    free(header);
}

void* _nocc_darray_push(void* array, void* value) {
    return _nocc_darray_pushn(array, 1, value);
}

void* _nocc_darray_pushn(void* array, size_t n, void* value) {
    _nocc_darray_header* header = _nocc_darray_calculate_header(array);
    if(header->size + n >= header->capacity) {
        size_t new_cap = header->size + n > header->capacity * 2 ? header->capacity * 2 + n : header->capacity * 2;
        array = _nocc_darray_resize(array, new_cap);
    }
    header = _nocc_darray_calculate_header(array);

    uint64_t addr = (uint64_t)array;
    memcpy(
            (void*)(addr + (header->size * header->stride)),
            value,
            n * header->stride
    );

    header->size += n;
    return array;
}

void* _nocc_darray_remove(void* array, size_t index, void* output_ptr) {
    _nocc_darray_header* header = _nocc_darray_calculate_header(array);
    
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

size_t _nocc_darray_size(void* array) {
    _nocc_darray_header* header = _nocc_darray_calculate_header(array);
    return header->size;
}

size_t _nocc_darray_capacity(void* array) {
    _nocc_darray_header* header = _nocc_darray_calculate_header(array);
    return header->capacity;
}

size_t _nocc_darray_stride(void* array) {
    _nocc_darray_header* header = _nocc_darray_calculate_header(array);
    return header->stride;
}

void* _nocc_darray_resize(void* array, size_t new_size) {
    _nocc_darray_header* header = _nocc_darray_calculate_header(array);
    size_t total_size = sizeof(_nocc_darray_header) + (new_size * header->stride);
    void* temp = realloc((void*)header, total_size);
    if(temp == NULL)
        return NULL;
    header = (_nocc_darray_header*)temp;
    header->capacity = new_size;
    return ((uint8_t*)temp + sizeof(_nocc_darray_header));
}
// Dynamic Array Implementation End

// String Implementation Begin

char* _nocc_string_resize(char*, size_t);

typedef struct {
    size_t size;
    size_t capacity;
} _nocc_string_header;

#define _nocc_string_calculate_header(s)    (_nocc_string_header*)((uint8_t*)(s) - sizeof(_nocc_string_header))

char* _nocc_string_reserve(size_t cap) {
    size_t header_size = sizeof(_nocc_string_header);
    size_t body_size = cap * sizeof(char);
    char* str = malloc(header_size + body_size);

    _nocc_string_header* header = (_nocc_string_header*)(void*)str;
    header->size = 0;
    header->capacity = cap;
    return ((uint8_t*)(str) + sizeof(_nocc_string_header));
}

void _nocc_string_free(char* s) {
    _nocc_string_header* header = _nocc_string_calculate_header(s);
    free(header);
}

char* _nocc_string_pushc(nocc_string str, char c) {
    _nocc_string_header* header = _nocc_string_calculate_header(str);
    if(header->size >= header->capacity) {
        str = _nocc_string_resize(str, header->capacity * 2);
    }
    header = _nocc_string_calculate_header(str);

    str[header->size++] = c;

    return str;
}

char* _nocc_string_push(nocc_string str, const char* cstr) {
    _nocc_string_header* header = _nocc_string_calculate_header(str);
    size_t n = strlen(cstr);
    if(header->size + n >= header->capacity) {
        size_t new_cap = header->size + n > header->capacity * 2 ? header->capacity * 2 + n : header->capacity * 2;
        str = _nocc_string_resize(str, new_cap);
    }
    header = _nocc_string_calculate_header(str);

    uint64_t addr = (uint64_t)str;
    memcpy(
            (void*)(addr + (header->size * sizeof(char))),
            cstr,
            n * sizeof(char)
    );

    header->size += n;
    return str;
}

size_t _nocc_string_size(nocc_string str) {
    _nocc_string_header* header = _nocc_string_calculate_header(str);
    return str[header->size] == '\0' ? header->size - 1 : header->size;
}

size_t _nocc_string_capacity(nocc_string str) {
    _nocc_string_header* header = _nocc_string_calculate_header(str);
    return header->capacity;
}

char* _nocc_string_resize(char* str, size_t new_size) {
    _nocc_string_header* header = _nocc_string_calculate_header(str);
    size_t total_size = sizeof(_nocc_string_header) + (new_size * sizeof(char));
    void* temp = realloc((void*)header, total_size);
    if(temp == NULL)
        return NULL;
    header = (_nocc_string_header*)temp;
    header->capacity = new_size;
    return ((uint8_t*)temp + sizeof(_nocc_string_header));
}
// String Implementation End

// File Functions Begin

bool _nocc_platform_mkdir(const char* dirname) {
    if(strcmp(dirname, ".") == 0) return true;
    if(strcmp(dirname, "..") == 0) return true;

#ifdef _WIN32
    if(CreateDirectory(dirname, NULL) == FALSE) {
        if(GetLastError() == ERROR_ALREADY_EXISTS) {
            nocc_trace("Dir '%s' already exists", dirname); 
            return true;
        }
        return false;
    }
    return true;
#else
    if(mkdir(dirname, 0x777) == -1) {
        if(errno == EEXIST) {
            nocc_trace("Dir '%s' already exists", dirname);
            return true;
        }
        nocc_error("Failed to create dir %s: %s", dirname, strerror(errno));
        return false;
    }
    return true;
#endif // _WIN32
}

bool nocc_mkdir_if_not_exists(const char* dirname) {
    const char* p;
    char* temp;
    bool ret = true;

    temp = calloc(1, strlen(dirname) + 1);
#ifdef _WIN32
    if ((p = strchr(dirname, ':')) != NULL) {
        p++;
    } else {
#endif
        p = dirname;
#ifdef _WIN32
    }
#endif

    while((p = strchr(p, '/')) != NULL) {
        if (p != dirname && *(p-1) == '/') {
            p++;
            continue;
        }

        memcpy(temp, dirname, p - dirname);
        temp[p - dirname] = '\0';
        p++;

        if(!_nocc_platform_mkdir(temp)) {
            ret = false;
            goto failure;
        }
    }

failure:
    free(temp);
    return ret;
}

bool _nocc_read_dir(const char *src_dir, const char *const filter, nocc_darray(const char*)* array_outp);
char* _nocc_generate_object(const char* src_file, const char *const fmt, va_list args);
char* _nocc_get_basename(const char* file);

nocc_darray(const char*) nocc_read_dir(const char *const filter) {
    nocc_darray(const char*) files = nocc_darray_create(const char*);
    nocc_read_dir_i(filter, &files);
    return files;
}

bool nocc_read_dir_i(const char *const filter, nocc_darray(const char*)* array_outp) {
    char dir[MAX_PATH] = {0};
    char* first_slash = strchr(filter, '/');
    if(first_slash == NULL)
        first_slash = (char*)filter;
    memcpy(dir, filter, first_slash - filter);
    _nocc_read_dir(dir, filter, array_outp);
    return true;
}

bool _nocc_read_dir(const char *src_dir, const char *const filter, nocc_darray(const char*)* array_outp) {
    DIR* dir = NULL;
    struct dirent* ent = NULL;

    dir = opendir(src_dir);
    if(dir == NULL) {
        nocc_error("Failed to open file %s: %s", ".", strerror);
        return false;
    }

    errno = 0;
    while((ent = readdir(dir)) != NULL) {
        if(strcmp(ent->d_name, ".") == 0) continue;
        if(strcmp(ent->d_name, "..") == 0) continue;

        char* buffer;
        asprintf(&buffer, "%s/%s", src_dir, ent->d_name);

        nocc_file_type type = _nocc_get_file_type(buffer);
        switch (type) {
            case NOCC_FT_FILE:
                if(fnmatch(filter, buffer, 0) == 0) {
                    nocc_darray_push(*array_outp, strdup(buffer));
                }
                break;
            case NOCC_FT_DIRECTORY:
                _nocc_read_dir(buffer, filter, array_outp);
            default:
                break;
        }

        free(buffer);
    }

    if (errno != 0) {
        nocc_error("Could not read directory %s: %s", src_dir, strerror(errno));
        return false;
    }

    if(dir)
        closedir(dir);
    return true;
}

nocc_darray(const char*) nocc_generate_objects(nocc_darray(const char*) sources, const char *const fmt, ...) {
    nocc_darray(const char*) objects = nocc_darray_reserve(const char*, nocc_darray_size(sources));
    va_list args;
    for(size_t i = 0; i < nocc_darray_size(sources); i++) {
        va_list args2;
        va_copy(args2, args);
        va_start(args2, fmt);
        nocc_string str = _nocc_generate_object(sources[i], fmt, args2);
        nocc_darray_push(objects, str);
        va_end(args2);
    }
    return objects;
}

nocc_string _nocc_generate_object(const char* src_file, const char *const fmt, va_list args) {
    nocc_string result = nocc_string_reserve(strlen(fmt));
    if(result == NULL)
        return NULL;

    for(const char* it = fmt; *it != '\0'; it++) {
        if(*it != '%') {
            nocc_string_pushc(result, *it);
            continue;
        }

        it++;

        switch (*it) {
            case 'n':
                char* basename = _nocc_get_basename(src_file);
                nocc_string_push(result, basename);
#ifdef _WIN32
                free(basename);
#endif
                break;
        }
    }

    nocc_string_push_null(result);
    return result;
}

char* _nocc_get_basename(const char* path) {
#ifdef _WIN32
    // Find the last occurrence of '/'
    const char* lastSlash = strrchr(path, '/');

    // If '/' is found, start after it; otherwise, start from the beginning
    const char* start = (lastSlash != NULL) ? lastSlash + 1 : path;

    // Find the last occurrence of '.' in the remaining string
    const char* lastDot = strrchr(start, '.');

    // Calculate the length of the substring without the extension
    size_t length = (lastDot != NULL) ? (size_t)(lastDot - start) : strlen(start);

    // Allocate memory for the result and copy the substring
    char* result = (char*)malloc(length + 1);
    if (result == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    strncpy(result, start, length);
    result[length] = '\0'; // Null-terminate the string

    return result;
#else
    return basename(file);
#endif // _WIN32
}

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

// fnmatch.h
#ifdef _WIN32
int fnmatch(const char *pattern, const char *string, int flags) {
    return PathMatchSpecA(string, pattern) == TRUE ? 0 : 1;
}
#endif // _WIN32

// dirent.h
#ifdef _WIN32
DIR* opendir(const char* name) {
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

int closedir(DIR* dir) {
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

#endif // _WIN32
// dirent.h

// File Functions End

// Command Implementation Begin

void _nocc_cmd_pid_wait(pid pid);
pid _nocc_cmd_run_command_async(nocc_command cmd);
bool _nocc_should_recompile(const char** inputfiles, size_t inputfiles_count, nocc_darray(const char*) headerfiles, const char* outputfile);

bool nocc_command_execute(nocc_command cmd) {
    _nocc_cmd_pid_wait(_nocc_cmd_run_command_async(cmd));
    return true;
}

bool nocc_should_recompile_many(nocc_darray(const char*) inputfiles, nocc_darray(const char*) headerfiles, const char* outputfile) {
    return _nocc_should_recompile(inputfiles, nocc_darray_size(inputfiles), headerfiles, outputfile);
}

bool nocc_should_recompile(const char* inputfile, nocc_darray(const char*) headerfiles, const char* outputfile) {
    return _nocc_should_recompile(&inputfile, 1, headerfiles, outputfile);
}

bool _nocc_should_recompile(const char** inputfiles, size_t inputfiles_count, nocc_darray(const char*) headerfiles, const char* outputfile) {
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

    for(size_t i = 0; i < inputfiles_count; i++) {
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

    size_t header_files_size = headerfiles == NULL ? 0 : nocc_darray_size(headerfiles);
    for(size_t i = 0; i < header_files_size; i++) {
        const char* headerfile = headerfiles[i];

        HANDLE header_file_fd = CreateFile(headerfile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
        if(header_file_fd == INVALID_HANDLE_VALUE) {
            if(GetLastError() == ERROR_FILE_NOT_FOUND) return true;
            nocc_error("File not found (%s)", headerfile);
            return true;
        }

        FILETIME header_file_time;
        status = GetFileTime(header_file_fd, NULL, NULL, &header_file_time);
        CloseHandle(header_file_fd);
        if(!status) { 
            nocc_error("Could not obtain file time: %s (%s)", GetLastError(), headerfile);
            return true;
        }

        if(CompareFileTime(&header_file_time, &output_file_time) == 1) return true;
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

    for (size_t i = 0; i < inputfiles_count; i++) {
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

    size_t header_files_size = headerfiles == NULL ? 0 : nocc_darray_size(headerfiles);
    for (size_t i = 0; i < header_files_size; i++) {
        const char *headerfile = headerfiles[i];
        if (stat(headerfile, &statbuf) < 0) {
            // NOTE: non-existing input is an error cause it is needed for building in the first place
            nocc_error("could not stat %s: %s", headerfile, strerror(errno));
            return true;
        }
        int header_file_time = statbuf.st_mtime;
        // NOTE: if even a single inputfile is fresher than outputfile that's 100% rebuild
        if (header_file_time > output_file_time) return 1;
    }

    return 0;
#endif // _WIN32
}
void _nocc_cmd_pid_wait(pid pid) {
#ifdef _WIN32
    DWORD result = WaitForSingleObject(pid, INFINITE);

    if(result == WAIT_FAILED) {
        nocc_error("Could not wait for child process %s", GetLastError());
        return;
    }

    DWORD exit_code;
    if(GetExitCodeProcess(pid, &exit_code) == 0) {
        nocc_error("Could not get the exit code %lu", GetLastError());
        return;
    }

    if(exit_code != 0) {
        nocc_error("Exit code recieved %d", exit_code);
        return;
    }

    CloseHandle(pid);
#else
    for(;;) {
        int wstatus = 0;
        if(waitpid(pid, &wstatus, 0) < 0) {
            nocc_error("Could not wait for child process %s", strerror(errno));
            return;
        }

        if(WIFEXITED(wstatus)) {
            int exit_code = WEXITSTATUS(wstatus);
            if (exit_status != 0) {
                nocc_error("Exited with exit code %d", exit_status);
            }

            break;
        }
    }

    if (WIFSIGNALED(wstatus)) {
        nocc_error("command process was terminated by %s", strsignal(WTERMSIG(wstatus)));
    }
#endif // _WIN32
}

pid _nocc_cmd_run_command_async(nocc_command cmd) {
#ifdef _WIN32
    nocc_string built_command = nocc_string_create();
    for(size_t i = 0; i < nocc_darray_size(cmd); i++) {
        nocc_debug("%s", cmd[i]);
        nocc_string_push(built_command, cmd[i]);
        nocc_string_pushc(built_command, ' ');
    }
    nocc_string_push_null(built_command);

    nocc_trace(built_command);

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
        nocc_error("Failed to fork child process");
        return NULL;
    }

    CloseHandle(piProcInfo.hThread);

    nocc_string_free(built_command);
    return piProcInfo.hProcess;
#else // ifndef _WIN32
    pid_t cpid = fork();
    if(cpid == -1) {
        nocc_error("Failed to fork child process %s", strerror(errno));
        return -1;
    }

    if(cpid == 0) {
        if(execvp(cmd[i], cmd + 1) == -1) {
            nocc_error("Failed to execute cmd %s", strerror(errno));
            return -1;
        }
    }

    return cpid;

#endif // _WIN32
}

nocc_darray(const char*) nocc_compile(nocc_compilation_options opts) {
    nocc_darray(const char*) srcs = nocc_read_dir(opts.src_filter);
    nocc_darray(const char*) incs = nocc_read_dir(opts.inc_filter);
    nocc_darray(const char*) objs = nocc_generate_objects(srcs, opts.obj_format);

    for(size_t i = 0; i < nocc_darray_size(srcs); i++) {
        if(!nocc_should_recompile(srcs[i], incs, objs[i]))
            continue;

        nocc_command command = nocc_command_create();
        nocc_command_add(command, opts.cc, "-c", srcs[i]);
        if(opts.cflags != NULL)
            nocc_command_add(command, opts.cflags);
        if(opts.includes != NULL)
            nocc_command_add(command, opts.includes);
        if(opts.defines != NULL)
            nocc_command_add(command, opts.defines);
        nocc_command_add(command, "-o", objs[i]);

        nocc_command_execute(command);

        nocc_command_free(command);
    }

    for(size_t i = 0; i < nocc_darray_size(srcs); i++) {
        free((char*)srcs[i]);
    }
    nocc_darray_free(srcs);
    for(size_t i = 0; i < nocc_darray_size(incs); i++) {
        free((char*)incs[i]);
    }
    nocc_darray_free(incs);

    return objs;
}

bool nocc_link(nocc_link_options opts) {
    if(nocc_should_recompile_many(opts.objs, NULL, opts.target)) {
        nocc_command command = nocc_command_create();
        nocc_command_add(command, opts.ld);
        if(opts.ldflags != NULL) {
            nocc_command_add(command, opts.ldflags);
        }
        nocc_command_add(command, "-o", opts.target);
        nocc_command_addn(command, nocc_darray_size(opts.objs), opts.objs);
        if(opts.ldadd) {
            nocc_command_add(command, opts.ldadd);
        }

        nocc_command_execute(command);
        nocc_command_free(command);
    }
    for(size_t i = 0; i < nocc_darray_size(opts.objs); i++) {
        free((char*)opts.objs[i]);
    }
    nocc_darray_free(opts.objs);
    return true;
}

// Command Implementation End


#endif // _NOCC_H_
