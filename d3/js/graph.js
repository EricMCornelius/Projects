var d3 = require('d3');
var _ = require('underscore');

exports.Graph = Graph;

function Graph(parent, data, args) {

  var self = this;

  var graph
    = parent.append('g');

  var width = parent.attr('width');
  var height = parent.attr('height');

  var left = 60;
  var bottom = 60;

  var xTicks = 5;
  var yTicks = 5;

  var graph_width = width - left;
  var graph_height = height - bottom;

  var padding = 4;
  var w = width / data.length - padding;

  var xScale
    = d3.scale.linear()
        .domain([0, data.length])
        .range([0, graph_width]);

  var yScale
    = d3.scale.linear()
        .domain([0, d3.max(data, function(d) { return d; })])
        .range([0, graph_height]);

  var yAxisScale
    = d3.scale.linear()
        .domain([0, d3.max(data, function(d) { return d; })])
        .range([graph_height, 0]);

  var xAxis
    = d3.svg.axis()
            .ticks(xTicks)
            .scale(xScale)
            .orient('bottom');

  var yAxis
    = d3.svg.axis()
            .ticks(yTicks)
            .scale(yAxisScale)
            .orient('left');

  var contents
    = graph.append('g')
           .attr('transform', 'scale(1,-1) translate(' + left + ',' + bottom + ')');

  var xAxisE = graph.append('g')
       .attr('transform', 'translate(' + left + ',' + graph_height +')')
       .classed('xAxis', true)
       .call(xAxis);

  var yAxisE = graph.append('g')
       .attr('transform', 'translate(' + left + ',0)')
       .classed('yAxis', true)
       .call(yAxis);

  var cScale
    = d3.scale.linear()
        .domain([0, d3.max(data, function(d) { return d; })])
        .range([160, 260]);

  var color = function(v) {
    var c = 'hsl(' + cScale(v) + ',100%,50%)';
    return c;
  };

  var click = function(e) {
    var elem = d3.select(this);

    elem.transition()
        .ease('cubic-in')
        .duration('500')
        .attr('height', function(d) { return yScale(d * .80); })
        .style('stroke-width', 3)
        .each('end', function(d) {


    elem.transition()
        .ease('cubic-out')
        .duration('750')
        .attr('height', function(d) { return yScale(d); })
        .style('stroke-width', 1);
    });
  };

  var construct = function(data) {
    contents
      .selectAll('rect')
      .data(data)
      .enter()
      .append('rect')
      .attr('y', -height)
      .attr('width', w)
      .attr('stroke', function(d) { return 'black'; })
      .classed('hover_highlight', true)
      .classed('bar', true)
      .on('click', click);
  };

  construct(data);

  var refresh_scales = function(data) {
    yScale.domain([0, d3.max(data, function(d) { return d; })]);
    yAxisScale.domain([0, d3.max(data, function(d) { return d; })]);
  };

  var render_axes = function(data) {
    refresh_scales(data);
    xAxis(xAxisE);
    yAxis(yAxisE);
  };

  var display = function(data) {
    contents.selectAll('rect')
            .data(data)
            .transition()
            .duration('750')
            .attr('height', function(d) { return yScale(d); })
            .attr('x', function(d, i) { return xScale(i); })
            .attr('fill', function(d) { return color(d); });

    render_axes(data);
  };

  var render = function(data) {
    display(data);
  };

  var swap = function(data, id1, id2) {
    var tmp = data[id1];
    data[id1] = data[id2];
    data[id2] = tmp;
  }

  function sorter(data) {
    var self = this;
    self.log = [];

    var id = -1;
    self.data = data.map(function(x) {
      return {id: ++id, value: x};
    });

    self.swap = function(id1, id2) {
      if (id1 === id2)
        return;

      self.log.push([self.data[id1].id, id2]);
      self.log.push([self.data[id2].id, id1]);

      var tmp = self.data[id1];
      self.data[id1] = self.data[id2];
      self.data[id2] = tmp;
    }

    self.move = function(id1, id2) {
      if (id1 === id2)
        return;

      self.log.push([self.data[id1].id, id2]);
      self.data[id2] = self.data[id1];
    }

    self.selection_sort = function() {
      var a = self.data;
      var n = a.length;

      for (var j = 0; j < n - 1; ++j) {
        var iMin = j;
        for (var i = j + 1; i < n; ++i) {
          if (a[i].value < a[iMin].value)
            iMin = i;
        }

        if (iMin != j)
          self.swap(iMin, j);
      }
    }

    self.insertion_sort = function() {
      var a = self.data;
      var n = a.length;

      for (var i = 1; i <= n - 1; ++i) {
        var item = a[i];
        self.move(i, n);
        var hole = i;
        while (hole > 0 && a[hole - 1].value > item.value) {
          self.move(hole - 1, hole);
          --hole;
        }
        self.move(n, hole);
      }
    }

    self.bubble_sort = function() {
      var a = self.data;
      var n = a.length;

      var sorted = false;
      while (!sorted) {
        sorted = true;
        for (var i = 1; i < self.data.length; ++i) {
          if (self.data[i].value < self.data[i - 1].value) {
            self.swap(i, i - 1);
            sorted = false;
          }
        }
      }
    }

    self.even_odd_sort = function() {
      var sorted = false;
      while (!sorted) {
        sorted = true;
        for (var i = 1; i < self.data.length - 1; i += 2) {
          if (self.data[i].value > self.data[i + 1].value) {
            self.swap(i, i + 1);
            sorted = false;
          }
        }

        for (var i = 0; i < self.data.length - 1; i += 2) {
          if (self.data[i].value > self.data[i + 1].value) {
            self.swap(i, i + 1);
            sorted = false;
          }
        }
      }
    }

    self.quick_sort = function() {
      var partition = function(left, right, pivot) {
        var pval = self.data[pivot].value;
        self.swap(pivot, right);
        var store = left;
        for (i = left; i < right; ++i) {
          if (self.data[i].value < pval) {
            self.swap(i, store);
            ++store;
          }
        }
        self.swap(store, right);
        return store;
      }

      var quicksort = function(left, right) {
        if (left < right) {
          var pivotIdx = left;
          var newPivotIdx = partition(left, right, pivotIdx);
          quicksort(left, newPivotIdx - 1);
          quicksort(newPivotIdx + 1, right);
        }
      }

      quicksort(0, self.data.length - 1);
    }
  };

  var sort_functors = ['selection_sort', 'insertion_sort', 'bubble_sort', 'even_odd_sort', 'quick_sort'];
  var sort_idx = 0;

  var height_adjust = false;
  var step_time = 200;
  var disp_time = 3000;

  function run(data, idxToPos) {
    if (sort.log.length === 0) {
      var newdata = getData(data.length);
      sort = new sorter(newdata);
      sort_idx = (sort_idx + 1) % sort_functors.length;
      sort[sort_functors[sort_idx]]();
      setTimeout(function(){
        render(newdata);
        setTimeout(function() {run(newdata);}, 1000);
      }, disp_time);
      return;
    }

    if (idxToPos === undefined) {
      var x = -1;
      idxToPos = [];
      data.map(function() {
        idxToPos.push(++x);
      });
    }

    var action = sort.log.shift();


    if (sort.log.length > 0) {
      var next = sort.log[0];
      if (next[1] === idxToPos[action[0]] && action[1] == idxToPos[next[0]]) {
        console.log('Swap!');
      }
    }


    idxToPos[action[0]] = action[1];

    if (height_adjust) {
      swap(data, pair[0], pair[1]);

      contents.selectAll('rect')
              .select(function(d, i) {
                if (d === data[i])
                  return null;
                return this;
              })
              .data(data)
              .transition()
              .duration(step_time)
              .attr('height', function(d) { return yScale(d); })
              .attr('fill', function(d) { return color(d); })
              .each('end', function(d, i) { if (!has_run) { has_run = true; run(data); }});
    }
    else {
      contents.selectAll('rect')
              .select(function(d, i) {
                if (i === action[0])
                  return this;
                return null;
              })
              .transition()
              .duration(step_time)
              .attr('x', function(d, i) {
                return xScale(idxToPos[i]);
              })
              .attr('fill', function(d) { return color(d); })
              .each('end', function(d, i) { run(data); });
    }
  }

  var sort = new sorter(data);
  sort[sort_functors[sort_idx]]();

  render(data);

  setTimeout(function(){run(data);}, 1000);
  //run(data);
};

function randInt(min, max) {
  return Math.floor(Math.random() * (max - min + 1)) + min;
}

function getData(size) {
  var data = [];
  for (var i = 0; i < size; ++i)
    data.push(Math.random());
  return data;
}
