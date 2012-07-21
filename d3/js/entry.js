require('./foo');
require('./bar');
var g = require('./graph');
var d3 = require('d3');

init = function() {
  var svg = d3.select('body')
              .append('svg')
              .attr('width', screen.width)
              .attr('height', screen.height);

  svg.attr('width', screen.width / 2)
     .attr('height', screen.height / 2);

  var data = [];
  for (var i = 0; i < 40; ++i)
    data.push(Math.random());

  var f = g.Graph(svg, data);

  /*
  var data = [
    { position: 50, color: 'red' },
    { position: 100, color: 'orange' },
    { position: 150, color: 'yellow' },
    { position: 200, color: 'green' },
    { position: 250, color: 'blue' }
  ];
  d3.select('svg')
    .selectAll('circle')
    .data(data)
    .enter()
    .append('circle')
    .text(function(d) { return d * 2; } )
    .attr('fill', function(d) { return d.color; })
    .attr('cx', function(d) { return d.position; })
    .attr('cy', 50)
    .attr('r', 50)
    .attr('class', 'hover_highlight');
  */

}