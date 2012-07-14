var fs = require('fs');
var util = require('util');
var async = require('async');
var vm = require('vm');
var glob = require('glob');

dump = function(obj) {
  console.log(JSON.stringify(obj, null, 2));
}

exists = function(obj) {
  return (obj !== undefined && obj !== null);
}

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



RegistryType = function() {
  this.contents = []
};

RegistryType.prototype.add = function(obj) {
  this.contents.push(obj);
};

GlobalRegistry = new RegistryType();

register = function(obj) {
  GlobalRegistry.add(obj);
};

include = function(path, cb) {
  fs.readFile(path, null, function(err, code) {
    if (err)
      cb(err);
    vm.runInThisContext(code);
    cb(null);
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
  generational_sort(g);
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

DependencyGraph = function(nodes) {
  return new DependencyGraphType(nodes);
}

Execute = function(cb) {
  glob.Glob('**/*.dep', null, function(err, results) {
    async.forEach(results, function(file, cb) {
      include(file, cb);
    }, function(err){ cb(); });
  });
}

Execute(function() {
  var graph = DependencyGraph(GlobalRegistry.contents);
  console.log(graph.generations);
  //console.log(graph);
  console.log('Done!');
});


for (var i = 0; i < 100000; ++i) {
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