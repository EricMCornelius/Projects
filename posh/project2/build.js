Project({
    name: "Project2",
    type: "SharedLib",
    compiler_flags: ['-fPIC'],
    files: ['*.cpp'],
    deps: [],
    prebuild: function() {
      console.log("Project2 prebuild executed");
    },
    postbuild: function() {
      console.log("Project2 postbuild executed");
    }
});
