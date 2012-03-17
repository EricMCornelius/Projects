-- A solution contains projects, and defines the available configurations
solution "redirect"
   configurations { "Debug", "Release" }
 
   -- A project defines one build target
   project "redirect"
      kind "StaticLib"
      language "C++"
      files { "src/*.cpp" }
      includedirs { "include" }
      buildoptions { "-std=c++0x" }
 
      configuration "Debug"
         defines { "DEBUG" }
         flags { "Symbols" }
 
      configuration "Release"
         defines { "NDEBUG" }
         flags { "Optimize" }    