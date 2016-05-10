var http = require('http'),
	path = require('path'),
	os = require('os'),
	fs = require('fs'),
	util = require('util'),
	Busboy = require('busboy');

http.createServer(function(req, res) {
	if (req.method === 'POST') {
		var busboy = new Busboy({
			headers: req.headers
		});
		busboy.on('file', function(fieldname, file, filename, encoding, mimetype) {
			console.log("Save the file to the path -> " + path.join(os.tmpDir(), filename));
			file.pipe(fs.createWriteStream(path.join(os.tmpDir(), filename)));
		});
		busboy.on('finish', function() {
			res.writeHead(200, {
				'Connection': 'close'
			});
			res.end("That's all folks!");
		});
		return req.pipe(busboy);
	}
	res.writeHead(404);
	res.end();
}).listen(3000, function() {
	console.log('Listening for requests on 3000 port');
});