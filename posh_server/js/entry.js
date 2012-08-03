var d3 = require('d3');
var io = require('socket.io-client');

var connection;

var input_btn;
var chart;
var nodes;
var edges;

var dependency_graph;

var node_map;
var node_info = {};

var interpolator = d3.interpolateRgb(d3.rgb(128,128,128), d3.rgb(0,0,256));

// callback when node is selected to set .selected class
selectNode = function() {
  var selected = d3.select(this);
  var curr = selected.classed('selected');

  nodes.classed('selected', false);
  selected.classed('selected', !curr);

  if (!curr) {
    var id = selected.attr('id');
    console.log(node_info[id]);
  }
}

// set node colors based on current state
refresh = function() {
  nodes.classed('nonbuild', function(d) { return !('target' in d.node); })
       .classed('building', function(d) { return ('building' === d.node.state); })
       .classed('built', function(d) { return ('built' === d.node.state); })
       .classed('failed', function(d) { return ('failed' === d.node.state); });
}

init = function() {
  connection = io.connect('http://EricCornelius.dyndns.org:8998');

  connection.on('initialize_graph', function(data) {
    dependency_graph = data;
  });

  connection.on('graph_svg', function(data) {
    // write the svg data to the chart
    chart.html(data.svg);

    defs = chart.select('svg')
                .insert('defs', '*');

    gradient = defs.append('linearGradient')
                   .attr('id', 'linear_gradient')
                   .attr('x1', '0%')
                   .attr('x2', '100%')
                   .attr('y1', '0%')
                   .attr('y2', '100%');

    gradient.append('stop')
            .attr('offset', '0%')
            .attr('stop-color', interpolator(0))
            .attr('stop-opacity', '0.5');

    gradient.append('stop')
            .attr('offset', '100%')
            .attr('stop-color', interpolator(1))
            .attr('stop-opacity', '1');

    gradient2 = defs.append('linearGradient')
                    .attr('id', 'linear_gradient2')
                    .attr('x1', '0%')
                    .attr('x2', '100%')
                    .attr('y1', '0%')
                    .attr('y2', '100%');

    gradient2.append('stop')
             .attr('offset', '0%')
             .attr('stop-color', '#ffffff')
             .attr('stop-opacity', '0.85');

    gradient2.append('stop')
             .attr('offset', '100%')
             .attr('stop-color', '#ffffff')
             .attr('stop-opacity', '0.15');

    mask = chart.select('svg')
                .select('defs')
                .append('mask')
                .attr('id', 'linear_mask')
                .attr('x', '0')
                .attr('y', '0')
                .attr('width', '1')
                .attr('height', '1')
                .attr('maskContentUnits', 'objectBoundingBox')
                .append('rect')
                .attr('x', '0')
                .attr('y', '0')
                .attr('width', '1')
                .attr('height', '1')
                .style('fill', 'url(#linear_gradient2)');

    chart.select('#graph1')
         .select('polygon')
         .style('fill', 'black');

    /*
    chart.select('#graph1')
         .select('polygon')
         .style('fill', 'url(#linear_gradient)');
    */

    // initialize the svg <g> node id to actual id map
    // and the node_info data object which is bound to the graph
    node_map = data.id_map;
    dependency_graph.nodes.forEach(function(node) {
      node_info[node_map[node.id]] = node;
    });

    // bind the corresponding data to each <g> node in the graph
    nodes = chart.selectAll('.node')
                 .datum(function(d, i) { return { id: this.id, node: node_info[this.id] }; })
                 .on('click', selectNode);

    edges = chart.selectAll('.edge');
    edges.selectAll('path, polygon')
         .style('stroke', 'white');
    edges.selectAll('polygon')
         .style('fill', 'grey');

    nodes.selectAll('ellipse')
         .style('mask', 'url(#linear_mask)');

    nodes.selectAll('text')
         .attr('font-weight', 'bold')
         .attr('fill', 'gold')
         .attr('font-family', 'consolas');

    // refresh colors
    refresh();
  });

  connection.on('begin_dependency', function(node) {
    // update node state to 'building' and refresh colors
    var n = node_info[node_map[node.id]];
    n.state = 'building';

    refresh();
  });

  connection.on('end_dependency', function(node) {
    // update node state and cmd/error and refresh colors
    var n = node_info[node_map[node.id]];
    n.state = 'built';
    n.cmd = node.__cmdinfo;
    n.error = node.error;

    refresh();
  });

  connection.on('fail_dependency', function(node) {
    // update node state and cmd/error and refresh colors
    var n = node_info[node_map[node.id]];
    n.state = 'failed';
    n.cmd = node.__cmdinfo;
    n.error = node.error;

    refresh();
  });

  connection.on('clean', function(data) {
    /*
    nodes.selectAll('ellipse')
         .transition()
         .duration(1000)
         .attr('cx', 0)
         .attr('cy', 0)
         .remove();

    console.log(nodes.selectAll('text'));
    nodes.selectAll('text')
         .transition()
         .duration(1000)
         .attr('x', 0)
         .attr('y', 0)
         .remove();

    edges.selectAll('*')
         .transition()
         .duration(1000)
         .remove();
    */

    // reset state on all nodes and refresh colors
    Object.keys(node_info).forEach(function(id) {
      delete node_info[id].state;
    });

    refresh();
  });

  input_btn = d3.select('#build_btn');
  chart = d3.select('#chart')
}

execute_generate = function() {
  connection.emit('generate', 0);
}

execute_build = function() {
  connection.emit('build', 0);
}

execute_clean = function() {
  connection.emit('clean', 0);
}
