Project({
    name: 'Project3',
    type: 'StaticLib',
    compiler_flags: ['-fPIC'],
    files: ['*.cpp'],
    deps: [],
    headers: ['*.h'],
    prebuild: function() {
      console.log('Project3 prebuild executed');
    },
    postbuild: function() {
      console.log('Project3 postbuild executed');
    }
});
