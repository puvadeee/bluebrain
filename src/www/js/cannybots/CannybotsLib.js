//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Cannybots JavaScript Library
//
// Authors:  Wayne Keenan
//
// License: http://opensource.org/licenses/MIT
//
// Version:   1.0  -  15.01.2015  -  Inital Version  (wayne@cannybots.com)

//////////////////////////////////////////////////////////////////////////////////////////////////


//Queue class: code.stephenmorley.org
function Queue(){var a=[],b=0;this.getLength=function(){return a.length-b};this.isEmpty=function(){return 0==a.length};this.enqueue=function(b){a.push(b)};this.dequeue=function(){if(0!=a.length){var c=a[b];2*++b>=a.length&&(a=a.slice(b),b=0);return c}};this.peek=function(){return 0<a.length?a[b]:void 0}};

function cannybotsWebSocket()
{
    if ("WebSocket" in window)
    {
        var ws = new WebSocket("%%WEBSOCKET_URL%%");
        ws.onopen = function()
        {
            console.log("WS.onOpen");
        };
        ws.onmessage = function(evt) {
            //console.log("WS.onMessage");
            cannybots.receiveBytes(evt.data);
        };
        ws.onclose = function() {
            console.log("WS.onClose");
        };
        
        return ws;
    }
    else
    {
        console.log("Browser doesn't support WebSocket!");
    }
    return undefined;
}

var cannybots = new function() {
    var self = this;
    
    self.commandQueue = new Queue();
    self.byteQueue = new Queue();
    self.useQueues = true;
    self.okToSend = true;

    // Queuing
    // Alternative webworkers, see: http://code.tutsplus.com/tutorials/getting-started-with-web-workers--net-27667
    self.processQueue = function () {
        var qLength = self.commandQueue.getLength();
        if ( (self.okToSend) && (qLength>0)) {
            self.okToSend = false;
            var message = self.commandQueue.dequeue();
            self.sendNativeMessage(message);
        }
    }
    
    self.startQueueWorker=function () {
        //setInterval(self.processQueue, 500);
    }
    
    self.sendBytes = function(message) {
        var message = {
            "rawBytes":bytesArray
        };
        
        if (self.useQueues)
            self.byteQueue.enqueue(message);
        else
            self.sendNativeMessage(message);
    }
    self.createMessagePayloadForCommand = function(cmd, param) {
		
         return {  
			 "command":cmd,
            "p1":param,
            "rawBytes":self.createByteMessage(cmd,param),
        };
	}
	
    self.sendCommand = function(cmd, param) {
        var message = self.createMessagePayloadForCommand(cmd,param);
        
        if (self.useQueues) {
            console.log("INFO: sendCommand(Q): " + JSON.stringify(message));
            self.commandQueue.enqueue(message);
        } else {
            self.sendNativeMessage(message);
        }
    }

    self.receiveBytes = function (bytesArray) {
        self.okToSend = true;
         console.log("DEBUG: receiveBytes: " + bytesArray);
    }
    
    self.sendDebug = function(msg) {
        console.log("DEBUG: " + msg);
        self.sendNativeMessage({"debug":msg});
    }
    self.sendError = function(msg) {
        console.log("ERROR: " + msg);
        self.sendNativeMessage({"error":msg});
    }
    
    self.debug = function (msg){
        document.querySelector("#cannybotsDebugPane").innerHTML = msg;
    }
    self.sendNativeMessage = undefined; // setup in start


    self.setupSendNativeMessage = function() {
        // iOS client on iOS webapp
        if(typeof window.webkit != 'undefined') {
            if(typeof window.webkit.messageHandlers != 'undefined') {
                try {
                    self.sendNativeMessage =     function (message){
                        window.webkit.messageHandlers.interOp.postMessage(message);
                    }
                } catch (err) {
                    console.log('ERROR: The native context does not exist yet');
                }
            } else {
                console.log("WARN: no iOS messageHandlers");
            }
        } else {
            try {
                console.log("INFO: Using WebSocket API");
                self.websocket = cannybotsWebSocket();
                self.sendNativeMessage =     function (message){
                    self.websocket.send(JSON.stringify(message));
                }
            } catch (err) {
                console.log("ERROR: " + err.message);
            }
        }
    }
    var rChar = "\r".charCodeAt(0);
    self.createByteMessage = function(cmd, p1) {
        return [0,0,cmd.charCodeAt(0), p1>>8, p1 &0xFF,rChar];
        
    }
    
    self.startLib = function () {
        self.setupSendNativeMessage();
		self.ping = function() {
		// if okToSend taking longer then 15 seconds
			self.sendNativeMessage(self.createMessagePayloadForCommand('?',0),0);
		}
		window.setInterval(self.ping, 2000);

    }
    
}


window.setInterval(cannybots.processQueue, 500);

// run once DOM is loaded
if(window.attachEvent) {
    window.attachEvent('onload', cannybots.startLib);
} else {
    if(window.onload) {
        var curronload = window.onload;
        var newonload = function() {
            curronload();
            cannybots.startLib();
        };
        window.onload = newonload;
    } else {
        window.onload = cannybots.startLib;
    }
}
