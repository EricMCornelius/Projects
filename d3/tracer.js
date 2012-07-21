var util = require('util');
var events = require('events');

exports.trace = trace;

function trace(server) {
  return new tracer(server);
};

function tracer(server) {
  var self = this;

  events.EventEmitter.call(self);

  var on_connect = function(socket) {
    var buf = '';
    socket.on('data', function(chunk) {
      buf += chunk;
    });
    socket.message = function() {
      return buf;
    };
    socket.clear = function() {
      buf = '';
    };
  };

  var on_request = function(request) {
    self.emit_message(request.connection.message());
    request.connection.clear();
  };

  server.on('connection', on_connect);
  server.on('request', on_request);
};

util.inherits(tracer, events.EventEmitter);

tracer.prototype.emit_message = function(message) {
  this.emit('message', message);
};