#include "../nocc.h"

#include <iostream>

int main(int argc, char** argv)
{
    nocc_rebuild_yourself(argc, argv);

    nocc::mkdir("bin");
    nocc::mkdir("bin-obj");

    std::filesystem::path target = "./bin/print.exe";
    std::vector<std::filesystem::path> sources = nocc::read_dir("./src/", "./src/*.c");
    std::vector<std::filesystem::path> objects;

    nocc::for_each(sources.begin(), sources.end(), [&](const std::filesystem::path& p)
    {
        // generate object filename
        std::filesystem::path object = nocc::generate_filename("./bin-obj/{}.o", p.stem());
        objects.emplace_back(object);

        // Check if it should rebuild
        if(!nocc::should_rebuild(p, object))
            return;

        // Builds the object
        nocc::compiler_flags flags;
        flags.lang(nocc::compiler_flags::language::c)
            .cc(nocc::compiler_flags::c_compiler::gcc)
            .src(p)
            .o(object);
        nocc::compile(flags);
    });

    if(nocc::should_rebuild(objects, target)) {
        nocc::compiler_flags flags;
        flags.lang(nocc::compiler_flags::language::c)
            .exe()
            .cc(nocc::compiler_flags::c_compiler::gcc)
            .obj(objects)
            .o(target);
        nocc::link(flags);
    }

}
