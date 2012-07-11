Project({
  name: 'test2',
  type: 'c++',
  deps: ['test']
});

Project({
  name: 'b',
  deps: ['c']
});

Project({
  name: 'c',
  deps: []
});
