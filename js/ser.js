var zmq = require('zmq'),
	responder = zmq.socket('rep'),
	history = new Array();

function key_search(_key, _cb) {
	var r = 0;
	history.forEach(function(element, index, arr) {
		if (element.toString() === _key) {
			r = 1;
		}
	});
	_cb(r);
}

function key_add(_key) {
	history.push(_key);
}

// key_add('117446064992713570345829064343663480705714593696910686886841809781365454286255502988052436129413252039134462942256917708029364377352892615444306118292236429881449700720300233304251599630713385158433650960702496911981171408707012495596788716459922641115227726393394324226659761738405691961991012172304664599575');
responder.on('message', function(request) {
	// console.log("Received request: [", request.toString(), "]");
	var t = request.toString().split("-");
	var c = t[0];
	var k = t[1];

	switch (parseInt(c)) {
		case 1: // 1. do key searching
			key_search(k, function(res) {
				if (res == 1) {
					responder.send(0);	// return 0 -> Don't need to upload again
				} else {
					key_add(k); 		// add to history
					responder.send(1); 	// return 1 -> Upload the new file
				}
			});
			break;
		case 2: // upload file
			// 1. do file receiving
			// 2. send reply back to client.
			responder.send("OK with 2");
			break;
		default:
			console.log("do nothing with unknown comma...");
	}
});

responder.bind('tcp://*:5555', function(err) {
	if (err) {
		console.log(err);
	} else {
		console.log("Listening on 5555…");
	}
});

process.on('SIGINT', function() {
	responder.close();
});