-- A solution contains projects, and defines the available configurations
solution 'video'
   configurations { 'Debug', 'Release' }
 
   -- A project defines one build target
   project 'video'
      kind 'ConsoleApp'
      language 'C++'
      files { 'src/*.cpp' }
      includedirs { 'include', 'C:/Programming/opencv/install/include' }
      libdirs { 'C:/Programming/opencv/install/bin' }
      links { 'opencv_highgui231', 'opencv_core231', 'opencv_contrib231', 'opencv_video231', 'opencv_imgproc231' }
      buildoptions { '-std=c++0x', '-D_GLIBCXX_USE_NANOSLEEP', '-m64' }
      
      dofile('redirect.lua')
 
      configuration 'Debug'
         defines { 'DEBUG' }
         flags { 'Symbols' }
 
      configuration 'Release'
         defines { 'NDEBUG' }
         flags { 'Optimize' }    