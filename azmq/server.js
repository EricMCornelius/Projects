#!/usr/bin/env node

var zmq = require('zmq');
var sock = zmq.socket('router');

sock.bindSync('tcp://127.0.0.1:9119');
sock.on('message', function(part1, part2) {
  var args = Array.prototype.slice.call(arguments)
    .map(function(buf) { return buf.toString(); });
  console.log('work:', args);
});
