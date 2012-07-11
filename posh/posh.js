var fs = require('fs');
var vm = require('vm');
var util = require('util');

function exists(obj) {
  return (obj !== undefined && obj !== null);
}

function initialize(obj, vars) {
  if (!exists(vars))
    vars = {};

  for (var prop in obj.defaults)
    if (obj.defaults.hasOwnProperty(prop))
      obj[prop] = obj.defaults[prop];

  for (var prop in vars) {
    if (!obj.defaults.hasOwnProperty(prop))
      console.log("No default for property: " + prop);
    if (vars.hasOwnProperty(prop))
      obj[prop] = vars[prop];
  }
}

var forEach = function(obj, func) {
  Object.keys(obj).forEach(function(key) {
    var val = obj[key];
    func(key, val);
  });
}

function BuildSetType(vars) {
  initialize(this, vars);
};

BuildSetType.prototype.defaults = {
  name: "",
  projects: [],
  dep_graph: {
    nodes: [],
    parents: {},
    children: {}
  },
  topological_order: []
}

BuildSetType.prototype.add_project = function(project) {
  this.projects.push(project);
}

BuildSetType.prototype.topological_sort = function() {
  var ordered = [];
  var visited = {};

  var g = this.dep_graph;

  var check_cycle = function(trace, val, output) {
    if (trace.node === val) {
      output.push(trace.node);
      return false;
    } 

    if (exists(trace.parent)) {
      if (!check_cycle(trace.parent, val, output)) {
        output.push(trace.node);
        return false;
      }
    }

    return true;    
  }

  var print_cycle = function(cycle) {
    var msg = '';
    cycle.map(function(node) {
      msg += node + ' -> ';
    });
    msg += cycle[0];
    console.log(msg);
  }

  var visit = function(node, trace) {
    var cycle = [];
    if (!check_cycle(trace, node, cycle)) {
      print_cycle(cycle);
      throw new Error("Dependency cycle detected!");
    }

    if (visited[node])
      return;

    visited[node] = true;

    g.parents[node].map(function(parent) {
      visit(parent, {parent: trace, node: node});
    });

    ordered.push(node);
  }

  visit('root', {});

  this.topological_order = ordered;
}

BuildSetType.prototype.generate_dependency_graph = function() {
  var g = this.dep_graph;

  var count = 0;
  var root_name = 'root';
  g.nodes.push(root_name);
  g.children[root_name] = [];
  g.parents[root_name] = [];

  this.projects.map(function(project) {
    project.id = ++count;
    g.nodes.push(project.name);
    g.children[project.name] = [];
    g.parents[project.name] = [];
  });

  this.projects.map(function(project) {
    g.parents[root_name].push(project.name);	
    project.deps.map(function(dep) {
      if (-1 === g.nodes.indexOf(dep))
	throw new Error("Unrecognized dependency: " + dep);

      g.children[project.name].push(dep);
      g.parents[dep].push(project.name);
    });
  });

  this.topological_sort();
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
  deps: [],
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
        parent.context = node.path;
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

dump = function(arg) {
  try {
    console.log(JSON.stringify(arg, null, 4));
  }
  catch(e) {
    console.log(arg);
  }
}

var tree = { path: '.', children: [] };

var walk = function(node) {
  if (exists(node.context))
    include(node.context);
  
  node.children.forEach(function(child){ walk(child); });
}

var post_search = function(err) {
  //dump(tree);

  walk(tree);

  try {
    GlobalBuildSet.generate_dependency_graph();
    dump(GlobalBuildSet.dep_graph);
    dump(GlobalBuildSet.topological_order);
  }
  catch(e) {
    console.log(e.message);
  }
}

recursive_search(tree, post_search);
