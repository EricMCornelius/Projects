#!/usr/bin/env node

var express = require('express');
var socket_io = require('socket.io');
var browserify = require('browserify');

var path = require('path');

var tracer = require('./tracer');

var static_root = path.join(__dirname, 'public_html');
var js_root = path.join(__dirname, 'js');

var app = express();

app.use(express.logger({immediate: true, format: 'dev'}));
app.use(express.bodyParser());
app.use(express.static(static_root));
app.use(express.directory(static_root));

var entry_script = path.join(js_root, 'entry.js');
var bundle = browserify({
  watch: true,
  entry: entry_script,
  require: {jquery: 'jquery-browserify'}
});

app.use(bundle);

var server = app.listen(8998);

var e = tracer.trace(server);

e.on('message', function(msg) {
  console.log(msg);
});
