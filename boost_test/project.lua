-- A solution contains projects, and defines the available configurations
solution "boost_test"
   configurations { "Debug", "Release" }
 
   -- A project defines one build target
   project "boost_test"
      kind "ConsoleApp"
      language "C++"
      files { "include/*.h", "src/*.cpp" }
      buildoptions { "-std=c++0x" }
 
      configuration "Debug"
         defines { "DEBUG" }
         flags { "Symbols" }
 
      configuration "Release"
         defines { "NDEBUG" }
         flags { "Optimize" }    