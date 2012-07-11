var fs = require('fs');
var vm = require('vm');
var glob = require('glob');
var path = require('path');
var util = require('util');
var proc = require('child_process');
var Q = require('q');

function exists(obj) {
  return (obj !== undefined && obj !== null);
}

function non_empty(str) {
  return str !== "";
}

function initialize(obj, vars) {
  if (!exists(vars))
    vars = {};

  for (var prop in obj.defaults)
    if (obj.defaults.hasOwnProperty(prop))
      obj[prop] = obj.defaults[prop];

  for (var prop in vars) {
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
  topological_order: [],
  name_to_project_map: {}
}

BuildSetType.prototype.add_project = function(project) {
  this.projects.push(project);
  this.name_to_project_map[project.name] = project;
}

BuildSetType.prototype.lookup_project = function(name) {
  return this.name_to_project_map[name];
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

  this.topological_order = ordered.reverse();

  // remove root
  this.topological_order.shift();
};

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
};

BuildSet = function(vars) {
  return new BuildSetType(vars);
};

GlobalBuildSet = BuildSet({name: "GlobalBuildSet"});

ProjectType = function(vars) {
  initialize(this, vars);
  this.build_set.add_project(this);

  if (!('root' in vars))
    this.root = path.dirname(current_file);
  if (!('outputdir' in vars))
    this.outputdir = path.join(this.root, 'build');
  if (!('toolchain' in vars))
    this.toolchain = current_toolchain;
  if (!('target' in vars))
    this.target = this.name;
};

ProjectType.prototype.defaults = {
  name: "",
  type: "",
  target: "",
  deps: [],
  files: [],
  publish: [],
  options: [],
  prebuild: function() { },
  build: function() { this.toolchain.build(this); },
  postbuild: function() { },
  build_set: GlobalBuildSet,
  toolchain: null
};

Project = function(vars) {
  return new ProjectType(vars);
};

BuildSettingsType = function(vars) {
  initialize(this, vars);
};

BuildSettingsType.prototype.defaults = {
  include_paths: [],
  lib_paths: [],
  libs: [],
  compiler_flags: [],
  linker_flags: [],
  defines: []  
};

BuildSettings = function(vars) {
  return new BuildSettingsType(vars);
};

ToolchainType = function(vars) {
  initialize(this, vars);
};

ToolchainType.prototype.defaults = {
  name: "",
  compiler: "",
  linker: "", 
  settings: BuildSettings(),
  generators: {
    include_paths: function(args) { return []; },
    lib_paths: function(args) { return []; },
    links: function(args) { return []; },
    defines: function(args) { return []; },
    compiler_flags: function(args) { return []; },
    linker_flags: function(args) { return []; },
    append_settings: function(args) { }
  },
  build: function(project) { }
};

Toolchain = function(vars) {
  return new ToolchainType(vars);
}

gcc = Toolchain(
{
  name: "g++",
  compiler: "g++",
  linker: "g++",
  settings: BuildSettings({include_paths: ['include']}),

  generators: {
    include_paths: function(args) {
      return args.map(function(path) {
        return '-I' + path;
      });
    },

    lib_paths: function(args) {
      return args.map(function(path) {
        return '-L' + path;
      });
    },

    links: function(args) {
      return args.map(function(lib) {
        return '-l' + lib;
      });
    },

    defines: function(args) {
      return args.map(function(define) {
        return '-D' + define;
      });
    },

    compiler_flags: function(args) {
      return args;
    },

    linker_flags: function(args) {
      return args;
    },

    append_settings: function(settings, appended) {
      var fields = ['include_paths', 'lib_paths', 'libs', 'compiler_flags', 'linker_flags', 'defines'];
      fields.map(function(field) {
        settings[field] = settings[field].concat(appended[field]);
      });
    },
  },

  compile_env: function(args) {
    var settings = BuildSettings(args);
    this.generators.append_settings(settings, this.settings);
 
    var flags = this.generators.compiler_flags(settings.compiler_flags);
    var defines = this.generators.defines(settings.defines);
    var includes = this.generators.defines(settings.include_paths);

    var args = flags.concat(defines).concat(includes);

    return {
      compiler: this.compiler,
      args: args
    }
  },
  
  link_env: function(args) {
    var settings = BuildSettings(args);
    this.generators.append_settings(settings, this.settings);
  
    var flags = this.generators.linker_flags(settings.linker_flags);
    var libpaths = this.generators.lib_paths(settings.lib_paths);
    var links = this.generators.links(settings.libs);

    var args = flags.concat(libpaths).concat(links);

    return {
      linker: this.linker,
      args: args
    }
  },

  build: function(project) {
    var compile_env = this.compile_env(project);
    var compile = this.compile;
    var link_env = this.link_env(project);
    var link = this.link;
    compile(project, compile_env, function() { link(project, link_env, function() { }) });
  },

  compile: function(project, env, callback) {
    console.log("Compiling now...");

    try {
      fs.mkdirSync(project.outputdir);
    }
    catch(e) { } 

    var files = [];
    project.files.map(function(file) {
      var file_path = path.join(project.root, file);
      files = files.concat(glob.sync(file_path, null));
    });

    var pending = files.length;
    files.map(function(file) {
      var base = path.basename(file, path.extname(file));
      output = path.join(project.outputdir, base + '.o');
      args = env.args.concat(['-c', file, '-o', output]);

      console.log(env.compiler);
      dump(args);

      var invoke = proc.spawn(env.compiler, args);
      invoke.stdout.on('data', function(data) {
        console.log('stdout: ' + data);
      });

      invoke.stderr.on('data', function(data) {
        console.log('stderr: ' + data);
      });

      invoke.on('exit', function(code) {
        --pending;
        if (pending === 0)
          callback();
      });
    });
  },

  link: function(project, env, callback) {
    console.log("Linking now...");
    
    var obj_glob = path.join(project.outputdir, '*.o');
    var files = glob.sync(obj_glob, null);

    var pending = files.length;
    args = [].concat(env.args);
    if (project.type == "SharedLib")
      args = args.concat('-shared');
    args = args.concat('-o', project.target).concat(files);

    console.log(env.linker);
    dump(args);

    var invoke = proc.spawn(env.linker, args);
    invoke.stdout.on('data', function(data) {
      console.log('stdout: ' + data);
    });

    invoke.stderr.on('data', function(data) {
      console.log('stderr: ' + data);
    });

    invoke.on('exit', function(code) {
      --pending;
      if (pending === 0)
        callback();
    });
  }
});

current_toolchain = gcc;

var current_file = '';
include = function(path) {
  current_file = path;
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
      var node = { path: path.join(parent.path, file), children: [] };
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
    console.log(JSON.stringify(arg, null, 2));
  }
  catch(e) {
    console.log(arg);
  }
}

var tree = { path: __dirname, children: [] };

var generate = function(node) {
  if (exists(node.context))
    include(node.context);
  
  node.children.forEach(function(child){ generate(child); });
}

var build = function() {
  GlobalBuildSet.topological_order.map(function(name) {
    var project = GlobalBuildSet.lookup_project(name);
    project.prebuild();
    project.build();
    project.postbuild();
  });  
}

var post_search = function(err) {
  generate(tree);

  try {
    GlobalBuildSet.generate_dependency_graph();
    dump(GlobalBuildSet.dep_graph);
    dump(GlobalBuildSet.topological_order);
  }
  catch(e) {
    console.log(e.message);
  }

  dump(GlobalBuildSet.projects);

  build();
}

recursive_search(tree, post_search);
