//#ifndef FILESYSTEM_H
//#define FILESYSTEM_H
//
//#include <string>
//#include <cstdlib>
//
//// FIX 3: Define logl_root placeholder since root_directory.h is not included.
//// This is the project root path when not set by environment variables.
//const char* logl_root = "";
//
//class FileSystem
//{
//private:
//    typedef std::string(*Builder) (const std::string& path);
//
//public:
//    static std::string getPath(const std::string& path)
//    {
//        static std::string(*pathBuilder)(std::string const&) = getPathBuilder();
//        return (*pathBuilder)(path);
//    }
//
//private:
//    // FIX 1: Renamed from 'getroot' to 'getRoot' for case consistency.
//    static std::string const& getRoot()
//    {
//        static char const* envroot = getenv("logl_root_path");
//        static char const* givenroot = (envroot != nullptr ? envroot : logl_root);
//        static std::string root = (givenroot != nullptr ? givenroot : "");
//        return root;
//    }
//
//    // FIX 2: Removed garbage/duplicate line.
//    static Builder getPathBuilder()
//    {
//        if (getRoot() != "")
//            return &FileSystem::getPathRelativeRoot;
//        else
//            return &FileSystem::getPathRelativeBinary;
//    }
//
//    static std::string getPathRelativeRoot(const std::string& path)
//    {
//        return getRoot() + std::string("/") + path;
//    }
//
//    static std::string getPathRelativeBinary(const std::string& path)
//    {
//        // This is the default path used by LearnOpenGL when CMake is not used.
//        return "../../../" + path;
//    }
//
//
//};
//
//#endif
//// Removed '#pragma once' and trailing '}' as they are redundant or placed incorrectly.