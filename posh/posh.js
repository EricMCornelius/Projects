var fs = require('fs');
var vm = require('vm');
var glob = require('glob');
var path = require('path');
var util = require('util');
var proc = require('child_process');
var async = require('async');

var DEBUG = true;

dump = function(arg) {
  if (!DEBUG)
    return;

  try {
    console.log(JSON.stringify(arg, null, 2));
  }
  catch(e) {
    console.log(arg);
  }
}

fs.copy = function (src, dst, cb) {
  function copy(err) {
    var is, os;

    if (!err)
      console.log('Overwriting file: ' + dst);

    fs.stat(src, function (err) {
      if (err)
        return cb(err);

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

function launch(args, cb) {
  dump(args);

  var invoke = proc.spawn(args.cmd, args.args);
  invoke.stdout.on('data', function(data) {
    dump(data);
  });

  invoke.stderr.on('data', function(data) {
    console.log(data + '');
  });

  invoke.on('exit', function(code) {
    if (code !== 0)
      return cb(new Error(code));

    cb(null);
  });
};

function BuildSetType(vars) {
  var self = this;
  initialize(self, vars);

  self.topological_order = [];
  self.name_to_project_map =  {};
  self.add_project = self.add_project.bind(self);
  self.lookup_project = self.lookup_project.bind(self);
  self.topological_sort = self.topological_sort.bind(self);
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
};

BuildSetType.prototype.add_project = function(project) {
  this.projects.push(project);
  this.name_to_project_map[project.name] = project;
};

BuildSetType.prototype.lookup_project = function(name) {
  return this.name_to_project_map[name];
};

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

  this.topological_order = ordered.filter(function(name) { return name !== 'root'; });
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

BuildSetType.prototype.generate_transitive_closure = function(project) {
  var projects = project.deps.splice(0);
  var closure = {};

  while (projects.length > 0) {
    var p = this.lookup_project(projects.shift());
    closure[p.name] = p;
    projects = projects.concat(p.deps);
  }

  var ordered = [];
  this.topological_order.forEach(function(name) {
    if (name in closure)
      ordered.push(name);
  });

  return ordered.map(this.lookup_project);
};

BuildSet = function(vars) {
  return new BuildSetType(vars);
};

GlobalBuildSet = BuildSet({name: "GlobalBuildSet"});

ProjectType = function(vars) {
  var self = this;
  initialize(self, vars);

  self.__prebuild = self.__prebuild.bind(self);
  self.__compile = self.__compile.bind(self);
  self.__link = self.__link.bind(self);
  self.__publish = self.__publish.bind(self);
  self.__postbuild = self.__postbuild.bind(self);

  self.build_set = GlobalBuildSet;
  self.toolchain = null;
  self.build_set.add_project(self);

  if (!exists(vars.root))
    self.root = path.dirname(current_file);

  if (!exists(vars.output_root))
    self.output_root = self.root;

  if (!exists(vars.toolchain))
    self.toolchain = current_toolchain;

  if (!exists(vars.target))
    self.target = self.name;

  var default_dirs = [
    ['outputdir', 'build'],
    ['bindir', 'bin'],
    ['libdir', 'lib'],
    ['includedir', 'include'],
    ['resourcedir', 'resources']
  ];

  default_dirs.forEach(function(info) {
    var field = info[0];
    var def = info[1];
    if (exists(vars[field]))
      self[field] = path.join(self.output_root, self[field]);
    else
      self[field] = path.join(self.output_root, def);
  });
};

ProjectType.prototype.defaults = {
  name: "",
  type: "",
  target: "",
  deps: [],
  files: [],
  headers: [],
  resources: [],
  options: [],
  prebuild: function() { },
  compile: function() { },
  link: function() { },
  publish: function() { },
  postbuild: function() { },
  __prebuild: function(cb) { this.prebuild(); cb(null); },
  __compile: function(cb) { this.toolchain.compile(this, cb); },
  __link: function(cb) { this.toolchain.link(this, cb); },
  __publish: function(cb) { this.toolchain.publish(this, cb); },
  __postbuild: function(cb) { this.postbuild(); cb(null); }
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
  initialize(self, args);

  this.compile = this.compile.bind(self);
  this.compile_env = this.compile_env.bind(self);
};

GccToolchainType.prototype.defaults = {
  name: "g++",
  compiler: "g++",
  linker: "g++",
  settings: BuildSettings({include_paths: ['include']}),

  generators: {
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

    collate_settings: function(obj1, obj2) {
      var fields = ['include_paths', 'lib_paths', 'libs', 'compiler_flags', 'linker_flags', 'defines'];
      return fields.map(function(field) {
        return obj1[field].concat(obj2[field]);
      });
    },
  },
};

GccToolchainType.prototype.compile_env = function(project) {
  //var settings = BuildSettings(project);
  var settings = this.collate_settings(project, this.settings);

  var deps = GlobalBuildSet.generate_transitive_closure(project);
  deps.forEach(function(dep) {
    settings.include_paths.push(dep.includedir);
  });
 
  var flags = this.generators.compiler_flags(settings.compiler_flags);
  var defines = this.generators.defines(settings.defines);
  var includes = this.generators.includes(settings.include_paths);

  var args = flags.concat(defines).concat(includes);

  return {
    compiler: this.compiler,
    args: args
  }
};
  
GccToolchainType.prototype.link_env = function(project) {
  //var settings = BuildSettings(project);
  
  var deps = GlobalBuildSet.generate_transitive_closure(project);
  deps.forEach(function(dep) {
    settings.lib_paths.push(dep.libdir);
    settings.libs.unshift(dep.target);
  });
  
  var flags = this.generators.linker_flags(settings.linker_flags);
  var libpaths = this.generators.libs(settings.lib_paths);
  var links = this.generators.links(settings.libs);
  var rpaths = this.generators.rpaths(settings.lib_paths);

  var args = flags.concat(libpaths).concat(links).concat(rpaths);

  return {
    linker: this.linker,
    args: args
  }
};

GccToolchainType.prototype.publish_env = function(project) {
  return { };
};

GccToolchainType.prototype.compile = function(project, cb) {
  console.log('Compiling ' + project.name + '...');
  var env = this.compile_env(project);
  
  try {
    fs.mkdirSync(project.outputdir);
  }
  catch(e) { } 

  var files = [];
  project.files.map(function(file) {
    var file_path = path.join(project.root, file);
    files = files.concat(glob.sync(file_path, null));
  });

  var commands = files.map(function(file) {
    var base = path.basename(file, path.extname(file));
    output = path.join(project.outputdir, base + '.o');
    args = env.args.concat(['-c', file, '-o', output]);
    return {
      cmd: env.compiler,
      args: args,
    };
  });

  async.forEach(
    commands,
    function(cmd, cb) { launch(cmd, cb); },
    function(err) { cb(null); }
  );
};

GccToolchainType.prototype.publish = function(project, cb) {
  console.log('Publishing ' + project.name + '...');
  var env = this.publish_env(project);
    
  try {
    fs.mkdirSync(project.includedir);
  }
  catch(e) { }

  var files = [];
  project.headers.map(function(file) {
    files = files.concat(glob.sync(file, {cwd: project.root}));
  });

  async.forEach(
    files, 
    function(file, cb) { fs.copy(path.join(project.root, file), path.join(project.includedir, file), cb); },
    function(err) { cb(null); }
  );
};

GccToolchainType.prototype.link = function(project, cb) {
  console.log('Linking ' + project.name + '...');
  var env = this.link_env(project);

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
  
  var command = {
    cmd: env.linker,
    args: args
  };

  launch(command, cb);
};

current_toolchain = new GccToolchainType();

var current_file = '';
include = function(file_path) {
  current_file = file_path;
  var code = fs.readFileSync(file_path);
  var script = vm.createScript(code, file_path);
  return script.runInThisContext();
}

var generate = function(files) {
  files.forEach(function(file) { include(file); });
}

var build = function() {
  var build_glob = '**/build.js'
  var files = glob.sync(build_glob, null);
  files = files.map(function(file){ return path.join(path.dirname(__filename), file); });

  generate(files);

  try {
    GlobalBuildSet.generate_dependency_graph();
    dump(GlobalBuildSet.dep_graph);
    dump(GlobalBuildSet.topological_order);
  }
  catch(e) {
    console.log(e.message);
  }

  var projects = GlobalBuildSet.topological_order.map(function(name) {
    return GlobalBuildSet.lookup_project(name);
  });

  var compile = function(err, cb) {
    async.forEach(
      projects,
      function(project, cb) { 
        console.log('\n' + name + ' started');
        async.series([project.__prebuild, project.__publish, project.__compile], cb);
      },
      function(err) { cb(null); }
    );
  };

  var link = function(err) {
    async.mapSeries(
      projects,
      function(project, cb) {
        async.series([project.__link, project.__postbuild], cb);
      },
      function(err) { console.log('Finished'); }
    );
  };

  compile(null, link);
}

command_map = {
  'help': {
    triggers: ['-h', '--h', '-help', '--help', 'help'],
    action: function() { 
      console.log('PoshJS C++ Build System'); 
    }
  },
  'build': {
    triggers: ['-b', '--b', '-build', '--build', 'build'],
    action: function() {
      build();
    }    
  }
};

process.argv.forEach(function(arg) {
  for (name in command_map) {
    var command = command_map[name];
    command.triggers.forEach(function(trigger) {
      if (trigger === arg) {
        command.action();
      }
    });
  }
});
