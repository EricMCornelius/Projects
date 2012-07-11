Project({
  name: 'Project1',
  type: 'c++',
  deps: ['Project2'],
  files: ['*.cpp'],
  outputdir: '/var/tmp/build',
  prebuild: function() {
    console.log("Project1 prebuild executed");
  },
  postbuild: function() {
    console.log("Project1 postbuild executed");
  }
});
