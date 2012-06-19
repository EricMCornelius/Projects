var fs = require('fs');
var vm = require('vm');

function check(obj) {
  return (obj !== undefined && obj !== null);
}

function initialize(obj, vars) {
  if (!check(vars))
    vars = {};

  for (var prop in obj.defaults) {
    if (obj.defaults.hasOwnProperty(prop) && !vars.hasOwnProperty(prop))
      obj[prop] = obj.defaults[prop];
    else
      obj[prop] = vars[prop];
  }
}

function BuildSetType(vars) {
  initialize(this, vars);
};

BuildSetType.prototype.defaults = {
  name: "",
  projects: []
}

BuildSetType.prototype.add_project = function(project) {
  this.projects.push(project);
}

BuildSet = function(vars) {
  return new BuildSetType(vars);
}

GlobalBuildSet = BuildSet({name: "GlobalBuildSet"});

ProjectType = function(vars) {
  initialize(this, vars);
  this.build_set.add_project(this);
};

ProjectType.prototype.defaults = {
  name: "",
  type: "",
  build_set: GlobalBuildSet
}

Project = function(vars) {
  return new ProjectType(vars);
};

include = function(path) {
  var code = fs.readFileSync(path);
  var script = vm.createScript(code, path);
  return script.runInThisContext();
}

var recursive_search = function(parent, cont) {
  fs.readdir(parent.path, function(error, files) {
    if (error)
      return cont(error);
    var remaining = files.length;
    if (!remaining)
      return cont(null);
    files.forEach(function(file) {
      var node = { path: parent.path + '/' + file, children: [] };
      if (file === "build.js") {
        parent.context = node;
	return cont(null);
      }
      else {
        fs.stat(node.path, function(error, info) {
          if (info && info.isDirectory()) {
            parent.children.push(node);
            recursive_search(node, function(error) {
              if (--remaining == 0)
                return cont(null);
            });
          }
          else {
            if (--remaining == 0)
              return cont(null);
          }
        });
      }
    });
  });
};

var tree = { path: '.', children: [] };
recursive_search(tree, function(err){console.log("Output:"); console.log(tree); console.log("Done!");});

//include('project.js');
//include('project2.js');

GlobalBuildSet.projects.map(function(project) {
  console.log(project);
});
