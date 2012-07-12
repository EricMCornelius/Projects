Project({
    name: 'Project2',
    type: 'SharedLib',
    compiler_flags: ['-fPIC'],
    files: ['*.cpp'],
    deps: ['Project3', 'Project4'],
    headers: ['*.h'],
    prebuild: function() {
      console.log('Project2 prebuild executed');
    },
    postbuild: function() {
      console.log('Project2 postbuild executed');
    }
});
