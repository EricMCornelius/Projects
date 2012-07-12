Project({
  name: 'Project1.tsk',
  type: 'Application',
  deps: ['Project2'],
  files: ['*.cpp'],
  prebuild: function() {
    console.log('Project1 prebuild executed');
  },
  postbuild: function() {
    console.log('Project1 postbuild executed');
  }
});
