-- A solution contains projects, and defines the available configurations
solution "test"
   configurations { "Debug", "Release" }
 
   -- A project defines one build target
   project "test"
      kind "ConsoleApp"
      language "C++"
      files { "src/*.cpp" }
      includedirs { "include", "../redirect/include" }
      libdirs { "../redirect" }
      links { "redirect", "opengl32", "gdi32" }
      buildoptions { "-std=c++0x" }
 
      configuration "Debug"
         defines { "DEBUG" }
         flags { "Symbols" }
 
      configuration "Release"
         defines { "NDEBUG" }
         flags { "Optimize" }    