ar d3 = require('d3');
var _ = require('underscore');

exports.BarGraph = BarGraph;

function BarGraph(parent, data, args) {

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
};