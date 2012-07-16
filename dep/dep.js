var fs = require('fs');
var util = require('util');
var async = require('async');
var vm = require('vm');
var glob = require('glob');
var proc = require('child_process');
var assert = require('assert');
var path = require('path');
var wrench = require('wrench');
var $ = require('jquery');

// add require to the global namespace
global.require = require;

// global used to track name of currently processed file
__file = __filename;

// global build root
__build_root = '/var/tmp/build';

// global used to track current file env
__env = {};

// extends path with a split function
path.split = function(p) {
  return p.split(path.sep);
};

path.join_arr = function(l) {
  return l.reduce(function(l, r) {
    return path.join(l, r);
  }, '');
};

// extends fs with an asynchronous copy function
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

// pretty-printed json dump
dump = function(obj) {
  return JSON.stringify(obj, null, 2);
}

// deep clone of object
clone = function(obj) {
  return $.extend(true, {}, obj);
}

exists = function(obj) {
  return (obj !== undefined && obj !== null);
}

empty = function(str) {
  return (str !== '');
}

launch = function(args, cb) {
  args.opts = args.opts || {};
  var root = args.opts.cwd || __dirname;

  var invocation = {
    cmds: ['cd ' + root, [args.cmd].concat(args.args).join(' ')],
    stdout: [],
    stderr: [],
    exit_code: 0
  };

  console.log(invocation.cmds[1]);

  var invoke = proc.spawn(args.cmd, args.args, args.opts);
  invoke.stdout.on('data', function(data) {
    invocation.stdout.push(data);
  });

  invoke.stderr.on('data', function(data) {
    invocation.stderr.push(data);
  });

  invoke.on('exit', function(code) {
    invocation.exit_code = code;
  });

  invoke.on('close', function() {
    if (invocation.exit_code !== 0) {
      console.log(invocation.stderr + '');
      return cb(new Error(invocation.exit_code), invocation);
    }
    cb(null, invocation);
  });
};

// Does what it says... constructs a custom error constructor
ErrorConstructorConstructor = function(name) {
  function ErrorType(args) {
    Error.captureStackTrace(this, this.constructor);

    this.name = name;
    this.message = args.message;
    this.cycle = args.cycle;
  };

  util.inherits(ErrorType, Error);
  return ErrorType;
};

CycleError = ErrorConstructorConstructor('CycleError');

UnhandledError = ErrorConstructorConstructor('UnhandledNode');

DuplicateIdError = ErrorConstructorConstructor('DuplicateId');

DuplicateTargetError = ErrorConstructorConstructor('DuplicateTarget');


RegistryType = function() {
  this.nodes = [],
  this.process_actions = [],
  this.register_actions = []
};

RegistryType.prototype.register_node = function(node) {
  assert(exists(node.id), 'Node must specify an id');

  this.register(node);
};

RegistryType.prototype.add_process_action = function(action) {
  assert(exists(action.type), 'Action must specify a type');

  this.process_actions.push(action);
};

RegistryType.prototype.add_register_action = function(action) {
  assert(exists(action.type), 'Action must specify a type');

  this.register_actions.push(action);
};

RegistryType.prototype.register = function(node) {
  this.register_actions.forEach(function(action) {
    if (action.type === node.type || action.type === '*') {
      action.exec(node);
    }
  });
  this.nodes.push(node);
};

RegistryType.prototype.process = function(args) {
  var executed = false;
  this.process_actions.forEach(function(action) {
    if (action.type === args.node.type || action.type === '*') {
      executed = true;
      action.exec(args);
    }
  });

  if (!executed) {
    //throw new UnhandledError({message: 'No matching action type detected', node: node});
    args.cb();
  }
};

GlobalRegistry = new RegistryType();

register = function(node) {
  node.__file = path.basename(__file);
  node.__dir = path.dirname(__file);
  node.root = path.resolve(node.__dir, node.root);
  node.src_root = node.src_root || '';
  node.target_root = node.target_root || '';
  node.src_root = path.resolve(node.root, node.src_root);
  node.target_root = path.resolve(node.root, node.target_root);
  node.deps = node.deps || [];
  GlobalRegistry.register_node(node);
};

DepRegistry = new RegistryType();

add_dep = function(node) {
  DepRegistry.register_node(node);
};

// args
// path: file to load
// prerun: function to execute before evaluation
// postrun: function to execute following evaluation
include = function(args) {
  fs.readFile(args.path, null, function(err, code) {
    args.prerun();
    try {
      vm.runInThisContext(code);
    }
    catch(e) {
      console.log('Failed to process file: ' + args.path);
      throw (e);
    }
    args.postrun();
  });
};

// passed a set of node objects
// each node may have an id, list of deps,
// and any other arbitrary fields
DependencyGraphType = function(nodes) {
  var self = this;

  self.nodes = nodes;
  self.node_map = {};

  // maps node id to dependents
  self.parents = {};

  // maps node id to dependencies
  self.children = {};

  // maps targets (outputs) to nodes
  self.target_map = {};

  generate_node_map(self);
  generate_target_map(self);
  generate_dependency_graph(self);
};

// generate the node map for the graph, and check for duplicate node ids
generate_node_map = function(g) {
  g.nodes.forEach(function(node) {
    if (node.id in g.node_map)
      throw new DuplicateIdError({message: 'Duplicate node id', id: node.id});

    g.node_map[node.id] = node;
  });
};

// generate the output map for the graph, and check for duplicate outputs
generate_target_map = function(g) {
  g.nodes.forEach(function(node) {
    if (!exists(node.target) && node.target !== '')
      return;

    if (node.target in g.target_map)
      throw new DuplicateTargetError({message: 'Duplicate target', target:node.target});

    g.target_map[node.target] = node.id;
  });
};

generate_dependency_graph = function(g) {
  g.nodes.forEach(function(node) {
    if (!exists(node.deps))
      node.deps = [];

    g.parents[node.id] = [];
    g.children[node.id] = [];
  });

  g.nodes.forEach(function(node) {
    if (!exists(node.deps))
      node.deps = [];

    node.deps.map(function(dep) {
      if (!exists(g.node_map[dep]))
        throw new Error("Unrecognized dependency: " + dep);

      g.children[node.id].push(dep);
      g.parents[dep].push(node.id);
    });
  });

  dag_assert(g);
  //generational_sort(g);
};

// assert that the graph is infact a DAG
dag_assert = function(g) {
  var check_cycle = function(id, colors, trace) {
    if (!exists(trace))
      trace = [];

    trace.push(id);
    if (colors[id] === 'forward') {
      trace = trace.reverse();
      var front = trace.shift();
      var message = front;
      var cycle = [front];
      while(next = trace.shift()) {
        cycle.push(next);
        message += ' -> ' + next;
        if (next === front)
          throw new CycleError({message: message, cycle: cycle});
      }
    }

    colors[id] = 'forward';

    g.parents[id].forEach(function(id) {
      if (colors[id] !== 'back')
        check_cycle(id, colors, trace);
    });

    trace.pop();
    colors[id] = 'back';
  }

  var roots = g.nodes.filter(function(node){ return (node.deps.length === 0); });
  var colors = {};
  roots.forEach(function(root){ check_cycle(root.id, colors); });

  // if we have an independent cycle which
  // is not reachable from any root, then
  // it will exist in the node_map but not colors
  for (var x in g.node_map)
    if (!(x in colors))
      check_cycle(x, {});
}

// uses DAG property for efficient longest path calculation
// for every node in the dependency graph
generational_sort = function(g) {
  // associate node id to remaining dependency count
  var counts = {};

  // generate array of nodes with out-degree 0
  g.nodes.forEach(function(node){ counts[node.id] = node.deps.length; });
  var active = g.nodes.filter(function(node){ return node.deps.length === 0; });

  // for a given generation with 0 out-degree
  // iterate over all nodes, decrementing parent node dependency counts
  // and appending to the subsequent generation if all deps are satisfied
  var process_generation = function(arr) {
    var output = [];
    while (arr.length > 0) {
      var next = arr.shift();
      g.parents[next.id].forEach(function(id) {
        --counts[id];
        if (counts[id] === 0)
          output.push(g.node_map[id]);
      });
    }
    return output;
  };

  // iteratively process all generations, and assign the results
  // for each to the graph object for subsequent build ordering
  g.generations = [];
  do {
    g.generations.push(active.map(function(node){ return node.id; }));
    active = process_generation(active);
  } while(active.length > 0)
};

// visits all dependent nodes starting from root, executing cb at each location
recursive_visit = function(g, node, cb) {
  node.deps.forEach(function(dep) {
    var dep = g.node_map[dep];
    if (cb(dep) !== false)
      recursive_visit(g, dep, cb);
  });
};

// execute the steps associated with this node
// and execute callback when finished
process_node = function(args) {
  args.registry.process(args);
};

// run concurrent build with up to concurrency number of simultaneous executors
process_graph = function(args) {
  // default to 4 concurrent actions
  concurrency = args.concurrency || 4;
  registry = args.registry;
  callback = args.cb || function() {};

  // if g isn't defined, load from the registry
  g = args.g || DependencyGraph(registry.nodes);

  // associate node id to remaining dependency count
  var counts = {};

  // generate array of nodes with out-degree 0
  g.nodes.forEach(function(node){ counts[node.id] = node.deps.length; });
  var active = g.nodes.filter(function(node){ return node.deps.length === 0; });
  var concurrent = 0;

  // tracks the number of nodes which have finished execution
  var executed = 0;
  var termination = g.nodes.length;

  var process_next = function() {
    // if concurrency has been exceeded, ignore this invocation
    if (concurrent >= concurrency)
      return;

    var next = active.shift();
    if (exists(next)) {
      ++concurrent;

      process_node({
        registry: registry,
        graph: g,
        node: next,
        cb: function(err) {
          --concurrent;

          // if all nodes are finished executing, call termination callback
          ++executed;
          if (executed === termination)
            return callback();

          g.parents[next.id].forEach(function(id) {
            --counts[id];
            if (counts[id] === 0)
              active.push(g.node_map[id]);
          });

          process_next();
        }
      });

      process_next();
    }
  };

  process_next();
};

DependencyGraph = function(nodes) {
  return new DependencyGraphType(nodes);
}

ScanDepFiles = function(cb) {
  glob.Glob('**/.dep', null, function(err, results) {
    // sort the .dep files lexicographically by directory
    // this ensures that all parent directory nodes will preceed child directory nodes
    results.sort(function(p1, p2){ return path.dirname(p1) > path.dirname(p2); });

    // iterate through list of paths, and for each directory
    // set deps to the first dep file encountered iterating towards root
    // if nothing is found, set a dependency on the root dep file ('global')
    // TODO: improve efficiency
    var deps = {};
    results.forEach(function(p) {
      var dir = path.dirname(p);
      deps[dir] = [];
      var split = path.split(dir);
      while (split.length > 0) {
        split.pop();
        var joined = path.join_arr(split);
        if (joined in deps) {
          deps[dir].push(joined);
          break;
        }
      }
      if (deps[dir].length === 0 && dir !== '.')
        deps[dir].push('.');
    });

    // create nodes for each .dep file and add them
    // to the dependency registry
    for (var id in deps) {
      add_dep({
        id: id,
        type: 'dep_file',
        deps: deps[id],
        file: id + '/.dep'
      });
    }

    // each .dep file inherits its parent environment and
    // may pass a modified __env object to all its children

    // map of .dep directory name to corresponding env
    envs = {};

    // action to trigger for each dep_file node in the DepRegistry graph (tree)
    var process = {
      type: 'dep_file',
      exec: function(args) {
        var node = args.node;
        var graph = args.graph;
        var post = args.cb;

        // look up the first .dep file detected
        // when iterating from current to root directory
        var dep = graph.children[node.id][0];

        // load the source code for this file, and before
        // executing it set the __file and __env markers
        // afterwards, save the environment back into the envmap
        // for this .dep file
        include({
          path: node.file,
          prerun: function() {
            __file = path.join(__dirname, node.file);
            if (dep)
              __env = clone(envs[dep]);
          },
          postrun: function() {
            envs[node.id] = __env;
            post();
          }
        });
      }
    };

    // register the dep_file node processing action, and process the graph
    DepRegistry.add_process_action(process);
    process_graph({
      registry: DepRegistry,
      cb: cb
    });
  });
}

var stress_test = function(count) {
  count = count || 10000;

  for (var i = 0; i < count; ++i) {
    var node = {
      id: i.toString(),
      deps: []
    };

    if (i === 0) {
      register(node);
      continue;
    }

    while (Math.floor(Math.random() * 2)) {
      var dep = Math.floor(Math.random() * i).toString();
      if (node.deps.indexOf(dep) === -1)
        node.deps.push(dep);
    }

    register(node);
  };
}

var compile = {
  type: 'compile',
  exec: function(args) {
    var node = args.node;
    var g = args.graph;
    var cb = args.cb;

    var include_paths = {};
    recursive_visit(g, node, function(node) {
      if (node.type === 'publish') {
        include_paths[path.dirname(node.target)] = true;
      }
      else if(node.type === 'external') {
        node.include_path.forEach(function(p) {
          include_paths[p] = true;
        });
      }
    });

    var paths = [];
    for (p in include_paths)
      paths.push('-I' + p);

    var flags = node.compile_flags || [];

    // compile the compilation unit via g++
    var compile_func = function() {
      var compile_args = ['-c', node.source, '-o', node.target].concat(flags).concat(paths);
      wrench.mkdirSyncRecursive(path.dirname(node.target));
      launch({
        cmd: 'g++',
        args: compile_args,
        opts: {cwd: node.root}
      }, cb);
    };

    // gather implicit dependency info construction for the compilation unit via g++ -MM
    var dep_func = function(cb) {
      var dep_args = ['-c', node.source, '-MM'].concat(flags).concat(paths);
      launch({
        cmd: 'g++',
        args: dep_args,
        opts: {cwd: node.root}
      }, function(err, data) {
        var dep_info = data.stdout + '';
        var split = dep_info.replace(/[\\\n]/g, '')
                            .split(' ')
                            .filter(empty);

        // first element is the .o file target
        split.shift();
        // second element is the .cpp source
        split.shift();

        var implicit_deps = split.map(function(dep) {
          return g.target_map[dep];
        });

        if (err) {
          console.log(data.stderr + '');
          throw (err);
        }

        cb();
      });
    }

    dep_func(compile_func);
  }
};

var link = {
  type: 'link',
  exec: function(args) {
    var node = args.node;
    var g = args.graph;
    var cb = args.cb;

    var lib_paths = {};
    var lib_names = {};
    var obj_files = {};
    recursive_visit(g, node, function(node) {
      if (node.type === 'link') {
        lib_paths[path.dirname(node.target)] = true;
        lib_names[path.basename(node.target)] = true;
      }
      else if(node.type === 'compile') {
        obj_files[node.target] = true;
        return false;
      }
      else if(node.type === 'external') {
        node.lib_path.forEach(function(p) {
          lib_paths[p] = true;
        });
        node.libs.forEach(function(l) {
          lib_names[l] = true;
        });
      }
    });

    var paths = [];
    for (p in lib_paths)
      paths.push('-L' + p);

    var rpaths = [];
    for (p in lib_paths)
      rpaths.push('-Wl,-rpath,' + p);

    var libs = [];
    for (l in lib_names)
      libs.push('-l' + l);

    var objs = [];
    for (o in obj_files)
      objs.push(o);

    var target_dir = path.dirname(node.target);
    var target_name = path.basename(node.target);

    var flags = node.link_flags || [];
    if (node.subtype === 'shared') {
      flags.push('-shared');
      target_name = 'lib' + target_name + '.so';
    }

    var target = path.join(target_dir, target_name);
    wrench.mkdirSyncRecursive(path.dirname(target));

    var args = ['-o', target].concat(flags).concat(objs).concat(paths).concat(rpaths).concat(libs);

    launch({
      cmd: 'g++',
      args: args,
      opts: {cwd: node.root}
    }, cb);
  }
};

var publish = {
  type: 'publish',
  exec: function(args) {
    var node = args.node;
    var cb = args.cb;

    fs.copy(node.source, node.target, cb);
  }
};

var template = {
  type: 'template',
  exec: function(node) {
    if (typeof node.source === 'string')
      var sources = glob.sync(node.source, {cwd: node.src_root});
    else
      var sources = node.source;

    var deps = [];
    var count = 0;
    sources.forEach(function(src) {
      var ext = path.extname(src);
      var file = path.basename(src, ext);
      var dir = path.dirname(src);
      var inst = clone(node);
      inst.source = path.resolve(inst.src_root, src);

      inst.target = inst.target.replace('${file}', file)
                               .replace('${ext}', ext)
                               .replace('${dir}', dir)
                               .replace('${base}', node.__dir)
                               .replace('${root}', inst.root);

      inst.target = path.resolve(inst.target_root, inst.target);
      inst.id = inst.id + '/' + ++count;
      inst.type = node.subtype;
      deps.push(inst.id);
      register(inst);
    });

    // template nodes don't actually have a source or target
    delete node.source;
    delete node.target;
    node.deps = node.deps.concat(deps);
  }
};

var external = {
  type: 'external',
  exec: function(node) {
    node.lib_path = node.lib_path || [];
    node.include_path = node.include_path || [];
    node.libs = node.libs || [];
  }
};

var general = {
  type: '*',
  exec: function(node) {
    if (node.type === 'template' || node.type === 'external')
      return;

    // resolve the source and target paths for the node
    node.source = path.resolve(node.src_root, node.source);
    node.target = path.resolve(node.target_root, node.target);
  }
};

var compile_register = {
  type: 'compile',
  exec: function(node) {
    node.compile_flags = node.compile_flags || __env.compile_flags;
  }
};

var link_register = {
  type: 'link',
  exec: function(node) {
    node.link_flags = node.link_flags || __env.link_flags;
  }
};

GlobalRegistry.add_process_action(compile);
GlobalRegistry.add_process_action(link);
GlobalRegistry.add_process_action(publish);

GlobalRegistry.add_register_action(template);
GlobalRegistry.add_register_action(general);
GlobalRegistry.add_register_action(compile_register);
GlobalRegistry.add_register_action(link_register);

ScanDepFiles(function() {
  process_graph({
    registry: GlobalRegistry,
    cb: function() {
      console.log(this.g);
    }
  });
});

//stress_test();
