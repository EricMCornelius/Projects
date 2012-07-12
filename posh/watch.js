if (!Object.prototype.watch) {
  Object.defineProperty(Object.prototype, "watch", 
  {
    enumerable: false,
    configurable: true,
    writable: false,
    value: function (prop, handler) {
      var oldval = this[prop],
      newval = oldval,
      getter = function () {
        return newval;
      },
      setter = function (val) {
        oldval = newval;
        return newval = handler.call(this, prop, oldval, val);
      };

      Object.defineProperty(this, prop, 
      {
        get: getter,
        set: setter,
        enumerable: true,
        configurable: true
      });
    }
  });
}

if (!Object.prototype.unwatch) {
  Object.defineProperty(Object.prototype, "unwatch", 
  {
    enumerable: false,
    configurable: true,
    writable: false,
    value: function (prop) {
      var val = this[prop];
      delete this[prop];
      this[prop] = val;
    }
  });
}

var group_model = {
  'name': 'Sample Group',
  'members': []
};

group_model.watch('name', function() {
  console.log('Name changed');
});
group_model.watch('members', function() {
  console.log('Members changed');
});

group_model.members.push('test');
group_model.name = 'test';
