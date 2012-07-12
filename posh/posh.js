var fs = require('fs');
var vm = require('vm');
var glob = require('glob');
var path = require('path');
var util = require('util');
var proc = require('child_process');
var async = require('async');

fs.copy = function (src, dst, cb) {
  function copy(err) {
    var is, os;

    if (!err) {
      return cb(new Error("File " + dst + " exists."));
    }

    fs.stat(src, function (err) {
      if (err) {
        return cb(err);
      }
      is = fs.createReadStream(src);
      os = fs.createWriteStream(dst);
      util.pump(is, os, cb);
    });
  }

  fs.stat(dst, copy);
};

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
    if (vars.hasOwnProperty(prop))
      obj[prop] = vars[prop];
  }
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
  var self = this;

  self.name = "";
  self.type = "";
  self.target = "";
  self.deps = [];
  self.files = [];
  self.publish = [];
  self.options = [];
  self.prebuild = function() { };
  self.compile = function() { };
  self.link = function() { }; 
  self.install = function() { };
  self.postbuild = function() { };
  self.__prebuild = function(cb) { self.prebuild(); cb(null); };
  self.__compile = function(cb) { self.toolchain.compile(self, cb); };
  self.__link = function(cb) { self.toolchain.link(self, cb); };
  self.__install = function(cb) { self.toolchain.install(self, cb); };
  self.__postbuild = function(cb) { self.postbuild(); cb(null); };

  self.build_set = GlobalBuildSet;
  self.toolchain = null;

  initialize(self, vars);
  self.build_set.add_project(self);

  if (!('root' in vars))
    self.root = path.dirname(current_file);
  if (!('outputdir' in vars))
    self.outputdir = path.join(self.root, 'build');
  if (!('bindir' in vars))
    self.bindir = path.join(self.root, 'bin');
  if (!('libdir' in vars))
    self.libdir = path.join(self.root, 'lib');
  if (!('includedir' in vars))
    self.includedir = path.join(self.root, 'include');
  if (!('toolchain' in vars))
    self.toolchain = current_toolchain;
  if (!('target' in vars))
    self.target = self.name;
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

GccToolchainType = function(args) {
  var self = this;  

  self.name = "g++",
  self.compiler = "g++",
  self.linker = "g++",
  self.settings = BuildSettings({include_paths: ['include']}),

  self.generators = {
    includes: function(args) {
      return args.map(function(path) {
        return '-I' + path;
      });
    },

    libs: function(args) {
      return args.map(function(path) {
        return '-L' + path;
      });
    },

    rpaths: function(args) {
      return args.map(function(path) {
        return '-Wl,-rpath,' + path;
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
        settings[field].concat(appended[field]);
      });
    },
  };

  self.compile_env = function(project) {
    var settings = BuildSettings(project);
   
    var deps = project.deps.map(function(name) { return GlobalBuildSet.lookup_project(name); });
    deps.forEach(function(dep) { 
      settings.include_paths.push(dep.includedir); 
    });

    self.generators.append_settings(settings, self.settings);
 
    var flags = self.generators.compiler_flags(settings.compiler_flags);
    var defines = self.generators.defines(settings.defines);
    var includes = self.generators.includes(settings.include_paths);

    var args = flags.concat(defines).concat(includes);

    return {
      compiler: self.compiler,
      args: args
    }
  };
  
  self.link_env = function(project) {
    var settings = BuildSettings(project);
  
    var deps = project.deps.map(function(name) { return GlobalBuildSet.lookup_project(name); });
    deps.forEach(function(dep) {
      settings.lib_paths.push(dep.libdir);
      settings.libs.push(dep.target);
    });

    self.generators.append_settings(settings, self.settings);
  
    var flags = self.generators.linker_flags(settings.linker_flags);
    var libpaths = self.generators.libs(settings.lib_paths);
    var links = self.generators.links(settings.libs.reverse);
    var rpaths = self.generators.rpaths(settings.lib_paths);

    var args = flags.concat(libpaths).concat(links).concat(rpaths);

    return {
      linker: self.linker,
      args: args
    }
  };

  self.install_env = function(project) {
    return { };
  }

  self.install = function(project, cb) {
    console.log('Installing ' + project.name + '...');
    var env = self.install_env(project);
    
    try {
      fs.mkdirSync(project.includedir);
    }
    catch(e) { }

    var files = [];
    project.publish.map(function(file) {
      files = files.concat(glob.sync(file, {cwd: project.root}));
    });

    async.forEach(
      files, 
      function(file, cb) { fs.copy(path.join(project.root, file), path.join(project.includedir, file), cb); },
      function(err) { cb(null); }
    );
  };

  self.compile = function(project, cb) {
    console.log('Compiling ' + project.name + '...');
    var env = self.compile_env(project);
  
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
          cb(null, null);
      });
    });
  };

  self.link = function(project, cb) {
    console.log('Linking ' + project.name + '...');
    var env = self.link_env(project);

    try {
      fs.mkdirSync(project.libdir);
    }
    catch(e) { } 

    try {
      fs.mkdirSync(project.bindir);
    }
    catch(e) { } 
    
    var obj_glob = path.join(project.outputdir, '*.o');
    var files = glob.sync(obj_glob, null);

    args = [].concat(env.args);
    
    var target = project.target;
    var output = '';
    if (project.type === 'SharedLib') {
      args = args.concat('-shared');
      target = 'lib' + target + '.so';
      output = path.join(project.libdir, target);
    }
    else if(project.type === 'StaticLib') {
      target = 'lib' + target + '.a';
      output = path.join(project.libdir, target);
      args = ['rcs', output];
      env.linker = 'ar';
    }
    else if(project.type === 'Application') {
      output = path.join(project.bindir, target);
    }

    if (project.type !== 'StaticLib')
      args = args.concat('-o', output);

    args = args.concat(files);

    console.log(env.linker);
    console.log(args);

    var invoke = proc.spawn(env.linker, args);
    invoke.stdout.on('data', function(data) {
      console.log('stdout: ' + data);
    });

    invoke.stderr.on('data', function(data) {
      console.log('stderr: ' + data);
    });

    invoke.on('exit', function(code) {
      cb(null);
    });
  };
};

current_toolchain = new GccToolchainType();

var current_file = '';
include = function(file_path) {
  current_file = file_path;
  var code = fs.readFileSync(file_path);
  var script = vm.createScript(code, file_path);
  return script.runInThisContext();
}

dump = function(arg) {
  try {
    console.log(JSON.stringify(arg, null, 2));
  }
  catch(e) {
    console.log(arg);
  }
}

var generate = function(files) {
  files.forEach(function(file) { include(file); });
}

var build = function() {
  // call each project in the build set in topological order
  async.mapSeries(GlobalBuildSet.topological_order, function(name, cb) {
    // retrieve project and chain steps
    var project = GlobalBuildSet.lookup_project(name);
    console.log('\n' + name + ' started');

    async.series([project.__prebuild, project.__compile, project.__link, project.__install, project.__postbuild], function(err, results) { if (!err) console.log(name + ' completed!'); cb(); });
  }, function() { });
}

var post_search = function(err) {
  generate(files);

  try {
    GlobalBuildSet.generate_dependency_graph();
    dump(GlobalBuildSet.dep_graph);
    dump(GlobalBuildSet.topological_order);
  }
  catch(e) {
    console.log(e.message);
  }

  build();
}

var build_glob = '**/build.js'
var files = glob.sync(build_glob, null);
files = files.map(function(file){ return path.join(path.dirname(__filename), file); });
post_search();
