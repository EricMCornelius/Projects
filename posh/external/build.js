Project({
  name: 'boost',
  type: 'Dependency',
  libdir: 'lib64',
  includedir: '/usr/include',
  libs: ['boost_chrono', 'boost_thread-mt', 'boost_system'] 
});

Project({
  name: 'opengl',
  type: 'Dependency',
  libdir: 'lib64',
  includedir: '/usr/include'
});
