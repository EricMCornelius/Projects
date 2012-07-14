var fs = require('fs');
var vm = require('vm');
var glob = require('glob');
var path = require('path');
var util = require('util');
var proc = require('child_process');
var async = require('async');

fs.mkdirp = require('mkdirp');

var DEBUG = false;

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
  console.log('Copying ' + src + ' -> ' + dst); 

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
  for (var prop in vars) {
    if (vars.hasOwnProperty(prop))
      obj[prop] = vars[prop];
  }
}

launch = function(args, cb) {
  dump(args);

  var invoke = proc.spawn(args.cmd, args.args, args.opts);
  invoke.stdout.on('data', function(data) {
    console.log(data + '');
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

function Barrier(count, cb) {
  this.count = count;
  this.callback = cb;
};

Barrier.prototype.signal = function() {
  --this.count;
  if (this.count === 0)
    this.callback();
};

function Defer(func) {
  return function(cb) { func(cb); };
};

function BuildSetType(vars) {
  var self = this;

  self.name = '';
  self.projects = [];
  self.dep_graph = {
    nodes: [],
    parents: {},
    children: {},
  };
  self.topological_order = [];
  self.name_to_project_map = {};

  initialize(self, vars);

  self.topological_order = [];
  self.name_to_project_map =  {};
  self.add_project = self.add_project.bind(self);
  self.lookup_project = self.lookup_project.bind(self);
  self.generate = self.generate.bind(self);
  self.topological_sort = self.topological_sort.bind(self);
  self.generate_transitive_closures = self.generate_transitive_closures.bind(self);
  self.generate_dependency_graph = self.generate_dependency_graph.bind(self);
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

  this.projects.forEach(function(project) {
    project.id = ++count;
    g.nodes.push(project.name);
    g.children[project.name] = [];
    g.parents[project.name] = [];
  });

  this.projects.forEach(function(project) {
    g.parents[root_name].push(project.name);	
    project.deps.map(function(dep) {
      if (-1 === g.nodes.indexOf(dep))
	throw new Error("Unrecognized dependency: " + dep);

      g.children[project.name].push(dep);
      g.parents[dep].push(project.name);
    });
  });
};

BuildSetType.prototype.generate_transitive_closures = function() {
  var self = this;
  self.projects.forEach(function(project) {
    var projects = project.deps.slice(0);
    var closure = {};

    while (projects.length > 0) {
      var p = self.lookup_project(projects.shift());
      closure[p.name] = p;
      projects = projects.concat(p.deps);
    }

    var ordered = [];
    self.topological_order.forEach(function(name) {
      if (name in closure)
        ordered.push(name);
    });

    project.dependency_closure = ordered;    
  });
};

BuildSetType.prototype.generate = function() {  
  this.generate_dependency_graph();
  this.topological_sort();
  this.generate_transitive_closures();

  dump(GlobalBuildSet.dep_graph);
  dump(GlobalBuildSet.topological_order);
}

BuildSet = function(vars) {
  return new BuildSetType(vars);
};

GlobalBuildSet = BuildSet({name: "GlobalBuildSet"});

ProjectType = function(vars) {
  var self = this;

  self.name = '';
  self.type = '';
  self.target = '';
  self.deps = [];
  self.dependency_closure = [];
  self.sources = [];
  self.source_root = '';
  self.headers = [];
  self.header_root = '';
  self.libs = []; 
  self.resources = [];
  self.options = [];
  self.artifacts = [];
  self.commands = [];

  initialize(self, vars);

  self.__prebuild = self.__prebuild.bind(self);
  self.__compile = self.__compile.bind(self);
  self.__link = self.__link.bind(self);
  self.__publish = self.__publish.bind(self);
  self.__postbuild = self.__postbuild.bind(self);

  self.toolchain = null;
  GlobalBuildSet.add_project(self);

  if (!exists(vars.root))
    self.root = path.dirname(current_file);

  if (exists(vars.header_root))
    self.header_root = path.resolve(self.root, self.header_root);
  else
    self.header_root = self.root;

  if (exists(vars.source_root))
    self.source_root = path.resolve(self.root, self.source_root);
  else
    self.source_root = self.root;

  if (exists(vars.installdir))
    self.installdir = path.resolve(self.root, vars.installdir);
  else
    self.installdir = path.join(self.root, 'install');

  if (!exists(vars.toolchain))
    self.toolchain = current_toolchain;

  if (!exists(vars.target))
    self.target = self.name;

  var default_dirs = [
    ['builddir', 'build'],
    ['bindir', 'bin'],
    ['libdir', 'lib'],
    ['includedir', 'include'],
    ['resourcedir', 'resources']
  ];

  default_dirs.forEach(function(info) {
    var field = info[0];
    var def = info[1];
    if (exists(vars[field]))
      self[field] = path.resolve(self.installdir, self[field]);
    else
      self[field] = path.join(self.installdir, def);
  });
};

ProjectType.prototype = {
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
  this.include_paths = [];
  this.lib_paths = [];
  this.libs = [];
  this.compiler_flags = [];
  this.linker_flags = [];
  this.defines = [];

  initialize(this, vars);
};

BuildSettings = function(vars) {
  return new BuildSettingsType(vars);
};

GccToolchainType = function(args) {
  var self = this;  

  self.name = "g++";
  self.compiler = "g++";
  self.linker = "g++";
  self.settings = BuildSettings({libs: ['rt']});  

  initialize(self, args);

  self.compile = self.compile.bind(self);
  self.publish = self.publish.bind(self);
  self.link = self.link.bind(self);
  self.compile_env = self.compile_env.bind(self);
  self.link_env = self.link_env.bind(self);
};

GccToolchainType.prototype.generators = {
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
  }
};

GccToolchainType.prototype.collate_settings = function(obj1, obj2) {
  var fields = ['include_paths', 'lib_paths', 'libs', 'compiler_flags', 'linker_flags', 'defines'];
  
  var result = {};
  fields.forEach(function(field) {
    result[field] = obj1[field].concat(obj2[field]);
  });
  return result;
};

GccToolchainType.prototype.compile_env = function(project) {
  var settings = this.collate_settings(BuildSettings(project), this.settings);

  project.dependency_closure.forEach(function(dep_name) {
    var dep = GlobalBuildSet.lookup_project(dep_name);
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
  var settings = this.collate_settings(BuildSettings(project), this.settings);

  project.dependency_closure.forEach(function(dep_name) {
    var dep = GlobalBuildSet.lookup_project(dep_name);
    settings.lib_paths.push(dep.libdir);

    if (dep.type === 'SharedLib' || dep.type === 'StaticLib')
      settings.libs.push(dep.target);
    else
      dep.libs.forEach(function(lib) { settings.libs.push(lib); });
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
  if (project.type === 'Dependency')
    return cb(null);

  console.log('Compiling ' + project.name + '...');
  var env = this.compile_env(project);
  
  try {
    fs.mkdirp.sync(project.installdir);
  }
  catch(e) { } 

  var files = [];
  project.sources.map(function(source) {
    var source_path = path.join(project.source_root, source);
    files = files.concat(glob.sync(source_path, null));
  });

  var artifacts = files.map(function(file) {
    var base = path.basename(file, path.extname(file));
    output = path.join(project.builddir, base + '.o');
    
    return {
      type: 'object_file',
      info: {
        source: { path: file },
        output: { path: output }
      }
    };
  });

  var source_stat = function(cb) {
    var sources = artifacts.map(function(artifact){ return artifact.info.source.path; });
    async.map(sources, fs.stat, function(err, results) {
      for (var i = 0; i < results.length; ++i)
        artifacts[i].info.source.timestamp = results[i].mtime; 
      cb();
    });
  };

  var output_stat = function(cb) {
    var outputs = artifacts.map(function(artifact){ return artifact.info.output.path; });
    async.map(outputs, fs.stat, function(err, results) {
      for (var i = 0; i < results.length; ++i)
        artifacts[i].info.output.timestamp = results[i].mtime;
      cb();
    });
  };

  var compilations = function(cb) {
    project.artifacts = project.artifacts.concat(artifacts);

    artifacts = artifacts.filter(function(artifact) {
      return (artifact.info.source.timestamp > artifact.info.output.timestamp);
    });

    var commands = artifacts.map(function(artifact) {
      args = env.args.concat(['-c', artifact.info.source.path, '-o', artifact.info.output.path]);
 
      return {
        cmd: env.compiler,
        args: args,
        opts: { cwd: project.root }
      };
    });

    project.commands = project.commands.concat(commands);

    async.forEach(
      commands,
      function(cmd, cb) { launch(cmd, cb); },
      function(err) { cb(err); }
    );
  };

  var stat = function(cb) {
    async.parallel([source_stat, output_stat], function(err, results) { cb(); });
  };

  async.series([stat, compilations], function(err, results) { cb(); });
};

GccToolchainType.prototype.publish = function(project, cb) {
  if (project.type === 'Dependency')
    return cb(null);

  console.log('Publishing ' + project.name + '...');
  var env = this.publish_env(project);
    
  try {
    fs.mkdirp.sync(project.builddir);
    fs.mkdirp.sync(project.includedir);
  }
  catch(e) { }

  var files = [];
  project.headers.forEach(function(file) {
    files = files.concat(glob.sync(file, {cwd: path.join(project.header_root)}));
  });

  async.forEach(
    files,
    function(file, cb) { fs.copy(path.join(project.header_root, file), path.join(project.includedir, file), cb); },
    function(err) { cb(null); }
  );
};

GccToolchainType.prototype.link = function(project, cb) {
  if (project.type === 'Dependency')
    return cb(null);

  console.log('Linking ' + project.name + '...');
  var env = this.link_env(project);

  try {
    fs.mkdirp.sync(project.libdir);
  }
  catch(e) { } 

  try {
    fs.mkdirp.sync(project.bindir);
  }
  catch(e) { } 
    
  var obj_glob = path.join(project.builddir, '*.o');
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

  var artifact = {
    type: project.type,
    info: {
      sources: [],
      output: { path: output }
    }
  };

  var source_stat = function(cb) {
    async.map(files, fs.stat, function(err, results) {
      for (var i = 0; i < results.length; ++i)
        artifact.info.sources.push({ path: files[i], timestamp: results[i].mtime });
      cb();
    });
  };

  var output_stat = function(cb) {
    fs.stat(output, function(err, result) {
      artifact.info.output.timestamp = result.mtime;
      cb();
    });
  };

  var link_action = function() {
    project.artifacts.push(artifact);

    var updated = false;
    artifact.info.sources.forEach(function(source) {
      if (source.timestamp > artifact.info.output.timestamp)
        updated = true;
    });
    
    if (!updated)
      return cb();

    if (project.type !== 'StaticLib')
      args = args.concat('-o', output);

    args = args.concat(files);

    var command = {
      cmd: env.linker,
      args: args
    };

    project.commands.push(command);

    launch(command, cb);    
  };

  async.parallel([source_stat, output_stat], function(err, results) { link_action(); });
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
    GlobalBuildSet.generate();
  }
  catch(e) {
    console.log(e.stack);
  }

  var projects = GlobalBuildSet.topological_order.map(function(name) {
    return GlobalBuildSet.lookup_project(name);
  });
  projects.reverse();

  var compile = function(err, cb) {
    async.forEach(
      projects,
      function(project, cb) { 
        console.log('\n' + name + ' started');
        async.series([project.__prebuild, project.__publish, project.__compile], cb);
      },
      function(err) { if (exists(err)) return err; cb(null); }
    );
  };

  var link = function(err) {
    async.mapSeries(
      projects,
      function(project, cb) {
        async.series([project.__link, project.__postbuild], cb);
      },
      function(err) { 
        projects.forEach(function(project) { dump(project); });
        console.log('Finished'); 
      }
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
  },
  'debug': {
    triggers: ['-d', '--d', '-debug', '--debug', 'debug'],
    action: function() {
      DEBUG = true;
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


