#!/usr/bin/env node

var express = require('express');
var socket_io = require('socket.io');
var browserify = require('browserify');
var posh = require('posh');
var tracer = require('./tracer');

var path = require('path');

var static_root = path.join(__dirname, 'public_html');
var js_root = path.join(__dirname, 'js');

var app = express();

app.use(express.logger({immediate: true, format: 'dev'}));
app.use(express.bodyParser());

var entry_script = path.join(js_root, 'entry.js');
var bundle = browserify({
  watch: true,
  entry: entry_script
});
app.use(bundle);

var server = app.listen(8998);

var socket_io = require('socket.io');
var io = socket_io.listen(server);

io.set('log level', 1);

var subscribers = [];

var notify = function(type, data) {
  subscribers.forEach(function(subscriber) {
    subscriber(type, data);
  });
};

var build_map = {};

var register_builder = function(builder) {
  builder.on('dependency_graph', function(g) {
    notify('initialize_graph', g);
  });
  builder.on('begin_dependency', function(n) {
    notify('begin_dependency', n);
  });
  builder.on('end_dependency', function(n) {
    notify('end_dependency', n);
  });
  builder.on('fail_dependency', function(n) {
    notify('fail_dependency', n);
  });
  builder.on('graph_svg', function(svg) {
    notify('graph_svg', svg);
  });
  builder.on('clean', function() {
    notify('clean', {});
  });
}

io.sockets.on('connection', function(socket) {
  subscribers.push(function(type, data) {
    socket.emit(type, data);
  });

  socket.on('build', function(id) {
    if (id in build_map) {
      var builder = build_map[id];
      builder.build();
    }
  });

  socket.on('generate', function(id) {
    var builder = posh.initialize({root: __dirname, concurrency: 4});
    register_builder(builder);
    builder.generate();

    build_map[id] = builder;
  });

  socket.on('clean', function(id) {
    if (id in build_map) {
      var builder = build_map[id];
      builder.clean();
    }
  });
});

app.use(express.static(static_root));
app.use(express.directory(static_root));

var e = tracer.trace(server);
e.on('message', function(msg) {
  console.log(msg);
});
