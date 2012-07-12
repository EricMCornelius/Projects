Project({
    name: 'Project4',
    type: 'StaticLib',
    compiler_flags: ['-fPIC', '-std=c++11'],
    files: ['*.cpp'],
    deps: [],
    headers: ['*.h'],
    output_root: '/var/tmp/build',
    includedir: 'p4includes',
    prebuild: function() {
      console.log('Project4 prebuild executed');
    },
    postbuild: function() {
      console.log('Project4 postbuild executed');
    }
});
