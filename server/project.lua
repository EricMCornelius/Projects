-- A solution contains projects, and defines the available configurations
solution "server"
   configurations { "Debug", "Release" }
 
   -- A project defines one build target
   project "server"
      kind "ConsoleApp"
      language "C++"
      files { "src/*.cpp" }
      includedirs { "include" }
      libdirs { }
      links { "boost_system" }
      --"ws2_32", "mswsock" }
      buildoptions { "-std=c++0x", "-pthread" }
 
      configuration "Debug"
         defines { "DEBUG" }
         flags { "Symbols" }
 
      configuration "Release"
         defines { "NDEBUG" }
         flags { "Optimize" }    