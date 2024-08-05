# No C Compiler (NOCC) #

## Intro ##
I really hate writing Makefiles and Premake scripts. Since, I pretty much use C to code. I figured why not implement a C script that would compile, run, and link not only a C file but also an entire C workspace. Also, the point of the build was to use a single file rather than using multiple files, and having a seperate executable to execute the nocc code, to then compile the entire project.

While the project was specifically inspired in C. The platform incopatability of C really,
pissed me off. So I decided to switch to C++. Currently, I use C++17 because of the filesystem
API.

This project was inspired by tsoding specifically his [nobuild](https://github.com/tsoding/nobuild) project.

To be honest I came with the acronym first, so if the name makes no sense that why.

## Getting Started ##
To get started download the nocc.h file. Create a nocc.cpp file. to build run `g++ -std=c++17 -02 ./nocc.cpp -o ./nocc.exe` and that's it.

The first time you build the project, the program will rebuild itself if the nocc.cpp or nocc.h files ever change after.

## Other ##
The code is poorly written. It took me two days to write the script. There some bugs with the code, for instance when I run nocc.exe the file will add or remove certain bytes for some reason. I have no idea. So if you do happen to run the code and it doesn't compile a file then try running it again. (Cause if it doesn't work the first time use a hammer and jam until it works)

## TODO ##
* [x] Add filters because if you build your project and compile it into the same folder, there are going to be compilation issues. Because, i'm just getting everything from the source folder and not filtering it for certain files.
* [] Add an actual CLI parser.
    * [ ] There is a CLI parser and it does work.
    * [ ] Be able to parse arguments for options. For example, --cheese=mozzerella or --age 24; both should be valid. (Don't really need this for this project)
* [x] Add the ability for nocc to only compile certain files, and not the entire project over again
    * [x] To do this the easiest (and the only way I know how to) is to differentiate the time from the source and executable. That's how make works, I think.
* [x] Add the ability to build itself. Rather than recompiling the file (I know its possible, but I have to my research on it).
* [ ] What would be cool, is to implement a -w flag, that watches for a change of any file in the program and recompiles the project. I don't know how cool this would be, but it would certainly be nice when developing a console application, you wouldn't have to type a command to recompile it. This would be annoying especially in big projects when compiling takes minutes.
