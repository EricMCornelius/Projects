var express = require('express');
var Cluster = require('cluster2');

var hash = require('./pass').hash;

var path = require('path');

d = function(obj) {
  console.log(JSON.stringify(obj));
}

var public_html = path.join(__dirname, 'public');

var cluster = new Cluster({
  port: 8080,
  cluster: false
});

var app = express();

// config

app.set('view engine', 'ejs');
app.set('views', path.join(__dirname, 'views'));

// middleware

function tracer(req, res, next) {
  var buf = '';
  req.setEncoding('utf8');
  req.on('data', function(chunk) {
    buf += chunk;
  });
  req.on('end', function() {
    console.log(buf);
  });
  next();
}

app.use(tracer);
app.use(express.logger({immediate: true, format: 'dev'}));
app.use(express.bodyParser());
app.use(express.cookieParser('shhhh, very secret'));
app.use(express.session());

// Session-persisted message middleware

app.use(function(req, res, next){
  var err = req.session.error
  , msg = req.session.success;
  delete req.session.error;
  delete req.session.success;
  res.locals.message = '';
  if (err) res.locals.message = '<p class="msg error">' + err + '</p>';
  if (msg) res.locals.message = '<p class="msg success">' + msg + '</p>';
  next();
});

// dummy database

var users = {
  tj: { name: 'tj' }
};

// when you create a user, generate a salt
// and hash the password ('foobar' is the pass here)

hash('foobar', function(err, salt, hash){
  if (err) throw err;
  // store the salt & hash in the "db"
  users.tj.salt = salt;
  users.tj.hash = hash;
});


// Authenticate using our plain-object database of doom!
function authenticate(name, pass, fn) {
  if (!module.parent) console.log('authenticating %s:%s', name, pass);
  var user = users[name];
  // query the db for the given username
  if (!user) return fn(new Error('cannot find user'));
  // apply the same algorithm to the POSTed password, applying
  // the hash against the pass / salt, if there is a match we
  // found the user
  hash(pass, user.salt, function(err, hash){
    if (err) return fn(err);
    if (hash == user.hash) return fn(null, user);
    fn(new Error('invalid password'));
  })
}

function restrict(req, res, next) {
  if (req.session.user) {
    next();
  } else {
    req.session.error = 'Access denied!';
    res.redirect('/login');
  }
}

app.get('/', function(req, res){
  res.redirect('login');
});

app.get('/restricted', restrict, function(req, res){
  res.send('Wahoo! restricted area');
});

app.get('/logout', function(req, res) {
  // destroy the user's session to log them out
  // will be re-created next request
  req.session.destroy(function(){
    res.redirect('/');
  });
});

app.get('/login', function(req, res) {
  if (req.session.user) {
    req.session.success = 'Authenticated as ' + req.session.user.name
    + ' click to <a href="/logout">logout</a>. '
    + ' You may now access <a href="/restricted">/restricted</a>.';
  }
  res.render('login');
});

app.post('/login', function(req, res) {
  authenticate(req.body.username, req.body.password, function(err, user){
    if (user) {
      // Regenerate session when signing in
      // to prevent fixation
      req.session.regenerate(function() {
        // Store the user's primary key
        // in the session store to be retrieved,
        // or in this case the entire user object
        req.session.user = user;
        res.redirect('back');
      });
    } else {
      req.session.error = 'Authentication failed, please check your '
      + ' username and password.'
      + ' (use "tj" and "foobar")';
      res.redirect('login');
    }
  });
});

app.get('/socket.io/*', function(req, res) {
  console.log('Requesting script!');
  res.sendfile(path.join(__dirname, 'node_modules', 'socket.io', 'lib', 'socket.io.js'));
});

app.use(express.static(public_html));
app.use(express.directory(public_html));

/*
cluster.listen(function(cb) {
  cb(app);
});
*/

var server = app.listen(8080);

var socket_io = require('socket.io');
var io = socket_io.listen(server);

io.set('log level', 1);

io.sockets.on('connection', function(socket) {
  var exec = new function() {
    var self = this;
    self.counter = 0;
    self.timeout = 100;

    self.action = function() {
      socket.emit('news', { count: ++self.counter });
      self.counter = self.counter % 100;
      setTimeout(self.action, self.timeout);
    }
  };

  exec.action();

  socket.on('my other event', function(data) {
    //console.log(data);
  });
});


server.on('connection', function(socket) {
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
});

server.on('request', function(request) {
  request.on('end', function() {
    console.log(request.connection.message());
    request.connection.clear();
  });
});