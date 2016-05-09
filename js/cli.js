var zmq = require('zmq'),
	requester = zmq.socket('req');

requester.on("message", function(reply) {
	// console.log("Received reply" + ": [", reply.toString(), ']');

	if (parseInt(reply) == 1) {		// prepare to upload file...
		console.log(1);
	} else {						// don't need to upload again...
		console.log(0);
	}
	requester.close();
	process.exit(0);
});

requester.connect("tcp://192.168.1.112:5555");

// 1-117446064992713570345829064343663480705714593696910686886841809781365454286255502988052436129413252039134462942256917708029364377352892615444306118292236429881449700720300233304251599630713385158433650960702496911981171408707012495596788716459922641115227726393394324226659761738405691961991012172304664599575
var args = process.argv.slice(2);
var cmd = args.toString().split("-");

if (parseInt(cmd[0]) == 1) {
	requester.send(args[0]);
} else {

}

process.on('SIGINT', function() {
	requester.close();
});