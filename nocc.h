#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cassert>

#include <vector>
#include <set>
#include <string>
#include <string_view>
#include <sstream>
#include <utility>
#include <type_traits>
#include <memory>
#include <iostream>
#include <filesystem>
#include <variant>
#include <optional>
#include <thread>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
#else
    #include <cstdio> // printf
#endif // _WIN32

#define nocc_rebuild_yourself(argc, argv) nocc::rebuild_yourself(argc, argv, __FILE__)

namespace nocc {

    /**
     *
     *  Formatting
     * 
     */

    namespace fmt {

        enum class format_arg_flags : uint8_t
        {
            zero_padding    = 0x01, // 1
            alternate       = 0x02, // 2
            prepend_plus    = 0x04, // 4
            prepend_minus   = 0x08, // 8
            prepend_space   = 0x10, // 16
            width           = 0x20, // 32
            precision       = 0x40, // 64
        };

        template<typename... T>
        using format_string_t = std::string_view;

        struct format_arg_detail
        {
        public:
            format_arg_detail(int64_t arg_id)
                : m_arg_id(arg_id)
            {}

            inline int64_t arg_id() const { return m_arg_id; }

            inline bool fzero_padding()  const { return m_flags & static_cast<uint8_t>(format_arg_flags::zero_padding); }
            inline bool falternate()     const { return m_flags & static_cast<uint8_t>(format_arg_flags::alternate); }
            inline bool fprepend_plus()  const { return m_flags & static_cast<uint8_t>(format_arg_flags::prepend_plus); }
            inline bool fprepend_minus() const { return m_flags & static_cast<uint8_t>(format_arg_flags::prepend_minus); }
            inline bool fprepend_space() const { return m_flags & static_cast<uint8_t>(format_arg_flags::prepend_space); }
            inline bool fwidth()         const { return m_flags & static_cast<uint8_t>(format_arg_flags::width); }
            inline bool fprecision()     const { return m_flags & static_cast<uint8_t>(format_arg_flags::precision); }

            inline uint64_t get_width()     const { return m_width; }
            inline uint64_t get_length()    const { return m_length; }
            inline uint64_t get_precision() const { return m_precision; }

            inline void toggle_flag(format_arg_flags flag) { m_flags ^= static_cast<uint8_t>(flag); }

            inline void set_width(uint64_t w) {
                m_width = w;
                toggle_flag(format_arg_flags::width);
            }

            inline void set_precision(uint64_t p) {
                m_precision = p;
                toggle_flag(format_arg_flags::precision);
            }


        private:
            int64_t m_arg_id = 0;
            uint8_t m_flags = 0;
            
            uint64_t m_width     = 0;
            uint64_t m_length    = 0;
            uint64_t m_precision = 0;
        };

        class format_arg_base
        {
        public:
            virtual ~format_arg_base() = default;
            virtual std::string format(format_arg_detail) = 0;
        };

        template<typename T>
        class format_arg : format_arg_base
        {};

        template<>
        class format_arg<int> : public format_arg_base
        {
        public:
            format_arg(int value)
                : m_value(value)
            {}

            std::string format(format_arg_detail detail)
            {
                std::stringstream ss;
                ss << m_value;
                return ss.str();
            }

        private:
            int m_value;
        };

        template<>
        class format_arg<const char*> : public format_arg_base
        {
        public:
            format_arg(const char* value)
                : m_value(std::move(value))
            {}

            std::string format(format_arg_detail detail)
            {
                return m_value;
            }

        private:
            const char* m_value;
        };

        template<>
        class format_arg<std::string> : public format_arg_base
        {
        public:
            format_arg(const std::string& value)
                : m_value(std::move(value))
            {}

            std::string format(format_arg_detail detail)
            {
                return m_value;
            }

        private:
            std::string m_value;
        };

        template<>
        class format_arg<std::filesystem::path> : public format_arg_base
        {
        public:
            format_arg(const std::filesystem::path& value)
                : m_value(std::move(value))
            {}

            std::string format(format_arg_detail detail)
            {
                return m_value.string();
            }

        private:
            std::filesystem::path m_value;
        };

        using format_args = std::vector<std::shared_ptr<format_arg_base>>;

        template<typename... Args>
        format_args make_format_args(Args&&... args)
        {
            format_args _args;
            (_args.emplace_back(std::make_shared<format_arg<std::decay_t<Args>>>(std::forward<Args>(args))), ...);
            return _args;
        }

        // vformat("my name is {0:^20} and I am {} years-old", "Joseph", 25);
        std::string vformat(std::string_view fmt, const format_args& args)
        {
            enum class state {
                manual, automated
            };

            std::string result;
            size_t args_index = 0;
            state s = state::automated;

            size_t prev_pos = 0;
            size_t pos = fmt.find('{', 0);
            while(pos != std::string_view::npos) {
                size_t end_of_scope = fmt.find('}', pos + 1);
                if(end_of_scope == std::string_view::npos) {
                    std::cout << "[error] Expected '}' but recieved end-of-string";
                    return result;
                }
                
                // result.insert(prev_pos, fmt.substr(prev_pos, pos - prev_pos));
                result += fmt.substr(prev_pos, pos - prev_pos);

                // if: {}
                if(end_of_scope == pos + 1) {
                    if(s == state::manual) {
                        std::cout << "[error] cannot switch from manual to automated args\n";
                        return result;
                    }

                    format_arg_detail detail(args_index);
                    // Setting default flags
                    detail.toggle_flag(format_arg_flags::zero_padding);
                    detail.toggle_flag(format_arg_flags::prepend_minus);

                    result += args[args_index]->format(detail);
                    args_index++;
                    prev_pos = end_of_scope + 1;
                    pos = fmt.find('{', end_of_scope + 1);
                    continue;
                }

                // TODO: Parse the format specification
            }

            result += fmt.substr(prev_pos);

            return result;
        }

        template<typename... Args>
        std::string format(fmt::format_string_t<Args...> fmt, Args&&... args)
        {
            return vformat(fmt, make_format_args<Args...>(std::forward<Args>(args)...));
        }

    }

    /**
     *  @brief Simple Implementation of std::format and fmt::format for C++17. Currently it only
     *  supports '{}' and does not support manual indexing, padding, and precision yet.
     *
     *  @param [fmt::format_string_t] fmt is the string to be formatted
     *  @param [Args] args are the variables to be formatted into the string
     *
     *  @return the formatted string
     *
     */
    template<typename... Args>
    std::string format(fmt::format_string_t<Args...> fmt, Args&&... args)
    {
        return fmt::format(fmt, std::forward<Args>(args)...);
    }

    /**
     *
     * Logging
     *
     */

    enum class log_level {
        trace, debug, info, warn, error, critical
    };

    namespace os {

        void print(const std::string& msg, uint8_t level)
        {
#ifdef _WIN32
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
            uint64_t length = msg.length();
            DWORD number_written = 0;
            WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), msg.c_str(), (DWORD)length, &number_written, 0);

            // Reset the console color to the original color
            SetConsoleTextAttribute(hConsole, originalColor);
#else
            const char* color_strings[] = {"1;30", "1;34", "1;32", "1;33", "1;31", "0;41"};
            printf("\033[%sm%s\033[0m", color_strings[level], msg.c_str());
#endif // _WIN32
        }

    }

    template<typename... Args>
    void print(log_level level, fmt::format_string_t<Args...> fmt, Args&&... args)
    {
        std::string formatteds = fmt::format(fmt, std::forward<Args>(args)...);
        os::print(formatteds, (uint8_t)level);
    }

    template<typename... Args>
    void trace(fmt::format_string_t<Args...> fmt, Args&&... args)
    {
        print(log_level::trace, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void debug(fmt::format_string_t<Args...> fmt, Args&&... args)
    {
        print(log_level::debug, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void info(fmt::format_string_t<Args...> fmt, Args&&... args)
    {
        print(log_level::info, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void warn(fmt::format_string_t<Args...> fmt, Args&&... args)
    {
        print(log_level::warn, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void error(fmt::format_string_t<Args...> fmt, Args&&... args)
    {
        print(log_level::error, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void critical(fmt::format_string_t<Args...> fmt, Args&&... args)
    {
        print(log_level::critical, fmt, std::forward<Args>(args)...);
    }

    /**
     *
     * filesystem
     *
     */

    bool mkdir(const std::filesystem::path& p)
    {
        return std::filesystem::create_directories(p);
    }

    template<typename... Args>
    std::string generate_filename(fmt::format_string_t<Args...> fmt, Args&&... args)
    {
        return fmt::format(fmt, std::forward<Args>(args)...);
    }

    namespace detail {

        bool match(const char* needle, const char* haystack)
        {
            for(; *needle != '\0'; needle++)
            {
                switch(*needle)
                {
                    case '?':
                        break;
                    case '*':
                    {
                        if(needle[1] == '\0')
                            return true;
                        size_t len = strlen(haystack);
                        for(size_t i = 0; i < len; i++)
                            if(match(needle + 1, haystack + i))
                                return true;
                        return false;
                    }
                    default:
                        if(*needle != *haystack) {
                            // This is a hack.
                            // TODO: FIX
                            if((*needle == '/' && *haystack == '\\') || (*needle == '\\' && *haystack == '/'))
                                continue;
                            return false;
                        }
                        ++haystack;
                }
            }

            return *haystack == '\0';
        }

    }

    std::vector<std::filesystem::path> read_dir(const std::string& dir, const std::string& filter)
    {
        std::vector<std::filesystem::path> entries;

        for (const std::filesystem::directory_entry& dir_entry : 
                std::filesystem::recursive_directory_iterator(dir))
        {
            std::filesystem::path p = dir_entry.path();
            if(detail::match(filter.c_str(), p.string().c_str()))
                entries.emplace_back(p);
        }

        return entries;
    }

    bool should_rebuild(const std::filesystem::path& input_filename, const std::filesystem::path& output_filename)
    {
        std::filesystem::file_time_type if_time = std::filesystem::last_write_time(input_filename);
        std::filesystem::file_time_type of_time = std::filesystem::last_write_time(output_filename);
        return if_time > of_time;
    }

    bool should_rebuild(const std::vector<std::filesystem::path>& input_filenames, const std::filesystem::path& output_filename)
    {
        for(auto& fname : input_filenames)
        {
            if(should_rebuild(fname, output_filename))
                return true;
        }
        return false;
    }

    template<typename InputIt, typename UnaryFunc>
    constexpr UnaryFunc for_each(InputIt first, InputIt last, UnaryFunc f)
    {
        for(; first != last; first++)
            f(*first);
        return f;
    }

    class compiler_flags
    {
    public:
        enum class language { c, cpp, unknown };
        enum class c_compiler { none, gcc, clang };
        enum class cpp_compiler { none, gpp, clangpp };
        enum class c_std { none, std99, std11, std17, std23 };
        enum class cpp_std { none, std11, std14, std17, std20, std23 };
        enum class optimizations { none, O0, O1, O2, O3 };
        enum class kind { none, static_lib, shared_lib, executable };

    public:

        compiler_flags& lang(language lang)
        {
            m_lang = lang;
            return *this;
        }

        compiler_flags& cc(const std::variant<c_compiler, cpp_compiler>& comp)
        {
            m_cc = comp;
            return *this;
        }

        compiler_flags& standard(const std::variant<c_std, cpp_std>& std)
        {
            m_standard = std;
            return *this;
        }

        compiler_flags& include(const std::filesystem::path& p)
        {
            m_includes.emplace_back(p);
            return *this;
        }

        compiler_flags& include(const std::initializer_list<std::filesystem::path>& paths)
        {
            m_includes = paths;
            return *this;
        }

        compiler_flags& define(const std::string& define)
        {
            m_defines.emplace_back(define);
            return *this;
        }

        compiler_flags& define(const std::initializer_list<std::string>& defines)
        {
            m_defines = defines;
            return *this;
        }

        compiler_flags& library(const std::string& library)
        {
            m_libraries.emplace_back(library);
            return *this;
        }

        compiler_flags& library(const std::initializer_list<std::string>& libraries)
        {
            m_libraries = libraries;
            return *this;
        }

        compiler_flags& library_paths(const std::filesystem::path& p)
        {
            m_library_paths.emplace_back(p);
            return *this;
        }

        compiler_flags& library_paths(const std::initializer_list<std::filesystem::path>& ps)
        {
            m_library_paths = ps;
            return *this;
        }

        compiler_flags& src(const std::filesystem::path& p)
        {
            m_input_paths.emplace_back(p);
            return *this;
        }

        compiler_flags& obj(const std::vector<std::filesystem::path>& ps)
        {
            m_input_paths = ps;
            return *this;
        }

        compiler_flags& o(const std::filesystem::path& p)
        {
            m_output_path = p;
            return *this;
        }

        compiler_flags& debug()
        {
            m_debug = true;
            return *this;
        }

        compiler_flags& optimize(optimizations o)
        {
            m_opt = o;
            return *this;
        }

        compiler_flags& shared()
        {
            m_kind = kind::shared_lib;
            return *this;
        }

        compiler_flags& lib()
        {
            m_kind = kind::static_lib;
            return *this;
        }

        compiler_flags& exe()
        {
            m_kind = kind::executable;
            return *this;
        }

        std::optional<std::string> to_string() const {
            std::stringstream ss;
            switch (m_lang) {
                case language::c:
                    switch (std::get<c_compiler>(m_cc)) {
                        case c_compiler::gcc:
                            ss << "gcc ";
                            break;
                        case c_compiler::clang:
                            ss << "clang ";
                            break;
                        case c_compiler::none:
                        default:
                            nocc::error("[error] please select a compiler");
                            break;
                    }
                    switch (std::get<c_std>(m_standard)) {
                        case c_std::std99:
                            ss << "-std=c99 ";
                            break;
                        case c_std::std11:
                            ss << "-std=c11 ";
                            break;
                        case c_std::std17:
                            ss << "-std=c17 ";
                            break;
                        case c_std::std23:
                            ss << "-std=c23 ";
                            break;
                        case c_std::none:
                        default:
                            nocc::error("[error] please select a compiler");
                            break;
                    }
                    break;
                case language::cpp:
                    switch (std::get<cpp_compiler>(m_cc)) {
                        case cpp_compiler::gpp:
                            ss << "g++ ";
                            break;
                        case cpp_compiler::clangpp:
                            ss << "clang++ ";
                            break;
                        case cpp_compiler::none:
                        default:
                            nocc::error("[error] please select a compiler");
                            break;
                    }
                    switch (std::get<cpp_std>(m_standard)) {
                        case cpp_std::std11:
                            ss << "-std=c++11 ";
                            break;
                        case cpp_std::std14:
                            ss << "-std=c++14 ";
                            break;
                        case cpp_std::std17:
                            ss << "-std=c++17 ";
                            break;
                        case cpp_std::std20:
                            ss << "-std=c++20 ";
                            break;
                        case cpp_std::std23:
                            ss << "-std=c++23 ";
                            break;
                        case cpp_std::none:
                        default:
                            break;
                    }
                    break;
                case language::unknown:
                default:
                    error("[error]: unknown language");
                    return {};
            }

            if(m_debug)
                ss << "-g ";

            switch (m_opt) {
                case optimizations::O0:
                    ss << "-O0 ";
                    break;
                case optimizations::O1:
                    ss << "-O1 ";
                    break;
                case optimizations::O2:
                    ss << "-O2 ";
                    break;
                case optimizations::O3:
                    ss << "-O3 ";
                    break;
                case optimizations::none:
                default:
                    break;
            }

            if(m_includes.size() > 0) {
                for(auto& include : m_includes)
                    ss << "-I" << include << " ";
            }
            if(m_defines.size() > 0)
                for(auto& define : m_defines)
                    ss << "-D" << define << " ";

            if(m_libraries.size() > 0)
                for(auto& library : m_libraries)
                    ss << "-l" << library << " ";

            if(m_library_paths.size() > 0)
                for(auto& lib_path : m_library_paths)
                    ss << "-L" << lib_path << " ";

            switch (m_kind) {
                case kind::shared_lib:
                    ss << "-shared ";
                case kind::static_lib:
                    error("[error] not implemented yet");
                    break;
                case kind::executable:
                case kind::none:
                default:
                    break;
            }

            if(m_input_paths.size() > 0) {
                if(m_input_paths[0].extension() == ".c" || m_input_paths[0].extension() == ".cpp") 
                {
                    // Check if it's compiling source to object 
                    if(m_output_path.extension() == ".o" || m_output_path.extension() == ".obj") {
                        ss << "-c " << m_input_paths[0] << " -o " << m_output_path;
                    } else { // Check if it's source to exe or lib
                        ss << m_input_paths[0] << " -o " << m_output_path;
                    }
                }
                else // Check if it's linking object to exe or library
                {
                    ss << "-o " << m_output_path << " ";
                    for(auto const& path : m_input_paths) {
                        ss << path << " ";
                    }
                }
            } else {
                error("[error]: expecetd at least one input paths could be either source files or intermediate object files");
                return {};
            }

            return ss.str();
        }

    private:
        // language specific
        language m_lang = language::unknown;
        std::variant<c_compiler, cpp_compiler> m_cc = c_compiler::none;
        std::variant<c_std, cpp_std> m_standard = c_std::none;
        
        std::vector<std::filesystem::path> m_includes;
        std::vector<std::string> m_defines;
        std::vector<std::string> m_libraries;
        std::vector<std::filesystem::path> m_library_paths;

        std::vector<std::filesystem::path> m_input_paths;
        std::filesystem::path m_output_path;

        std::filesystem::path m_pch_path; // todo

        bool enable_precompiled_header = false;
        bool m_debug = false;
        optimizations m_opt = optimizations::none;
        kind m_kind = kind::none;
    };
#ifdef _WIN32
    typedef HANDLE pid;
#else // _WIN32
    typedef pid_t pid;
#endif // _WIN32

    pid build(const compiler_flags& flags)
    {
#ifdef _WIN32
        // TODO: create the string here
        std::optional<std::string> opt = flags.to_string();
        if(!opt) {
            nocc::error("[error]: failed to stringify the flags");
            return {};
        }
        std::string command = opt.value();

        info("{}\n", command);

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
                (char*)(command.c_str()),
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
            error("Failed to fork child process");
            return NULL;
        }

        CloseHandle(piProcInfo.hThread);

        return piProcInfo.hProcess;
#else // ifndef _WIN32
        pid_t cpid = fork();
        if(cpid == -1) {
            error("Failed to fork child process %s", strerror(errno));
            return -1;
        }

        if(cpid == 0) {
            // TODO: Need to combine the different vectors into one.
            if(execvp(cmd[i], cmd + 1) == -1) {
                error("Failed to execute cmd %s", strerror(errno));
                return -1;
            }
        }

        return cpid;

#endif // _WIN32
    }

    enum build_opts {
        multi_thread_compiling = 0x01
    };

    void compile(const compiler_flags& flags, uint32_t opts = 0x00)
    {
        build(flags);
        // TODO multi_thread_compiling is enabled
    }

    void link(const compiler_flags& flags, uint32_t opts = 0x00)
    {
        build(flags);
    }

    namespace os {

        std::optional<std::string> get_env_var(const char* envvar)
        {
#ifdef _WIN32
            std::string s;
            s.resize(4096, ' ');
            size_t len;
            getenv_s(&len, s.data(), 4096, envvar);
            if(len == 0)
                return {};
            s.resize(len);
            return s;
#else
            char* buffer = getenv(envvar);
            if(buffer == nullptr)
                return {};

            return std::string(buffer);
#endif // _WIN32
        }

        std::set<std::filesystem::path> get_path_vars()
        {
            std::set<std::filesystem::path> result;
            std::optional<std::string> opt = nocc::os::get_env_var("PATH");
            if(!opt)
                return result;

            std::string pathenv = opt.value();

            size_t last_pos = 0;
            size_t pos = pathenv.find(';');
            while(pos != std::string::npos)
            {
                std::filesystem::path env = pathenv.substr(last_pos, pos - last_pos);
                if(std::filesystem::exists(env))
                    result.emplace(env);
                
                last_pos = pos + 1;
                pos = pathenv.find(';', pos + 1);
            }

            return result;
        }

    }

    namespace detail {

        std::optional<std::filesystem::path> find_compiler()
        {
#ifdef GCC_WORKS
            // While I prefer working with GNU's gcc and g++ compilers, for some reason
            // when I'm compiling nocc with g++ version 14.1.0 I get the following error
            //      `collect2.exe: error: ld returned 116 exit status`
            // I have no idea what's causing this issue. I think it's the std::filesystem or some other
            // C++17 feature, but either way my plan is for this to become standard independent as
            // much as possible.
            const std::string compilers[2] = { "g++", "clang++" };
#else
            const std::string compilers[1] = { "clang++" };
#endif // GCC_WORKS
            
            // get compiler from environment variable
            for(auto var : compilers) {
                std::optional<std::string> comp = nocc::os::get_env_var(var.c_str());
                if(comp)
                    return comp.value();
                
            }

            // get compiler from the PATH environment variable
            std::set<std::filesystem::path> paths = nocc::os::get_path_vars();

            //
            for(auto& var : compilers) {
                for(auto& path : paths)
                {
                    for(auto const& direntry : 
                            std::filesystem::directory_iterator(path))
                    {
                        std::filesystem::path compiler_path = direntry.path();
                        if(compiler_path.stem().string() == var)
                            return compiler_path;
                    }
                }


            }

            return {};
        }

        bool rebuild_yourself(const char* source_file, const char* bin_file)
        {
            std::optional<std::filesystem::path> opt = find_compiler();
            if(!opt)
                return false;

            std::filesystem::path compiler_path = opt.value();
            nocc::compiler_flags::cpp_compiler compiler = compiler_path.stem() == "g++" ?
                nocc::compiler_flags::cpp_compiler::gpp : 
                compiler_path.stem() == "clang++" ? nocc::compiler_flags::cpp_compiler::clangpp :
                nocc::compiler_flags::cpp_compiler::none;


            compiler_flags flags;
            flags.lang(nocc::compiler_flags::language::cpp)
                .cc(compiler)
                .standard(nocc::compiler_flags::cpp_std::std17)
                .optimize(nocc::compiler_flags::optimizations::O2)
                .src(source_file)
                .o(bin_file);
            nocc::build(flags);

            return true;
        }

    }

    void rebuild_yourself(int argc, char** argv, const char* source_file)
    {
        const char* nocch_file = __FILE__;
        assert((argc >= 1) && "Expected more than 0 arguments");
        const char* bin_path = argv[0];

        // header file
        if(should_rebuild(nocch_file, bin_path)) {
            info("{} has changed rebuilding nocc.exe\n", nocch_file);
            detail::rebuild_yourself(source_file, bin_path);
            return;
        }

        // cpp file
        if(!should_rebuild(source_file, bin_path))
            return;

        info("{} has changed rebuilding nocc.exe\n", source_file);
        detail::rebuild_yourself(source_file, bin_path);
    }

}
