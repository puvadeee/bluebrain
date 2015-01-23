var http = require('http');
var express = require("express");
var RED = require("node-red");
var app = express();
var fs  = require('fs');


app.get('/js/cannybots/CannybotsLib.js', function(req, res) {
	var file = 'public/js/cannybots/CannybotsLib.js';
	fs.readFile(file, function (err, data) {
		if (err) throw err;
		var str = data.toString('utf-8');
		var wsUrl = 'ws://' + req.headers.host + '/api/ws/cannybots';
		res.send(str.replace("%%WEBSOCKET_URL%%", wsUrl));	
	});
	
})

app.use("/",express.static("public"));
module.exports = app;


