-- A solution contains projects, and defines the available configurations
solution "server"
   configurations { "Debug", "Release" }
 
   -- A project defines one build target
   project "server"
      kind "ConsoleApp"
      language "C++"
      files { "src/*.cpp" }
      includedirs { "include", "C:/Programming/boost_1_49_0" }
      libdirs { "C:/Programming/boost_1_49_0/stage/lib" }
      links { "boost_system-mgw47-mt-1_49", "ws2_32", "mswsock" }
      buildoptions { "-std=c++0x" }
 
      configuration "Debug"
         defines { "DEBUG" }
         flags { "Symbols" }
 
      configuration "Release"
         defines { "NDEBUG" }
         flags { "Optimize" }    