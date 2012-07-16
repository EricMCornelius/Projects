var fs = require('fs');
var util = require('util');
var async = require('async');
var vm = require('vm');
var glob = require('glob');
var proc = require('child_process');
var assert = require('assert');
var path = require('path');
var $ = require('jquery');

// add require to the global namespace
global.require = require;

// global used to track name of currently processed file
__file = __filename;

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

launch = function(args, cb) {
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


RegistryType = function() {
  this.nodes = [],
  this.actions = []
};

RegistryType.prototype.add_node = function(node) {
  assert(exists(node.id), 'Node must specify an id');

  this.nodes.push(node);
};

RegistryType.prototype.add_action = function(action) {
  assert(exists(action.type), 'Action must specify a type');

  this.actions.push(action);
};

RegistryType.prototype.process = function(args) {
  var executed = false;
  this.actions.forEach(function(action) {
    if (action.type === args.node.type) {
      executed = true;
      action.exec(args);
    }
  });

  if (!executed)
    throw new UnhandledError({message: 'No matching action type detected', node: node});
};

GlobalRegistry = new RegistryType();

register = function(node) {
  node.__path = __file;
  GlobalRegistry.add_node(node);
};

DepRegistry = new RegistryType();

add_dep = function(node) {
  DepRegistry.add_node(node);
};

// args
// path: file to load
// prerun: function to execute before evaluation
// postrun: function to execute following evaluation
include = function(args) {
  fs.readFile(args.path, null, function(err, code) {
    args.prerun();
    vm.runInThisContext(code);
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
  nodes.forEach(function(node){ self.node_map[node.id] = node; });

  self.parents = {};
  self.children = {};

  generate_dependency_graph(self);
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
            __file = node.file;
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
    DepRegistry.add_action(process);
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
    setTimeout(function() {
      console.log('Compilation finished: ' + args.node.id);
      args.cb();
    }, 1000);
  }
};

var link = {
  type: 'link',
  exec: function(args) {
    setTimeout(function() {
      console.log('Link finished: ' + args.node.id);
      args.cb();
    }, 2000);
  }
};

var publish = {
  type: 'publish',
  exec: function(args) {
    setTimeout(function() {
      console.log('Publish finished: ' + args.node.id);
      args.cb();
    }, 500);
  }
};

GlobalRegistry.add_action(compile);
GlobalRegistry.add_action(link);
GlobalRegistry.add_action(publish);

ScanDepFiles(function() {
  process_graph({
    registry: GlobalRegistry
  });
});

//stress_test();