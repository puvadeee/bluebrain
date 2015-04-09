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
        var ws = new WebSocket("%%WEBSOCKET_URL%%");  // this tempalte string will be replaced by the Web Server hosting this file
        ws.onopen = function()
        {
            console.log("WS.onOpen");
        };
        ws.onmessage = function(evt) {
            //console.log("WS.onMessage");
            //var decoded = atob(evt.data);
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
    self.recvDelegate = function() {};

    // Queuing
    // Alternative webworkers, see: http://code.tutsplus.com/tutorials/getting-started-with-web-workers--net-27667
    self.processQueue = function () {
        var qLength = self.commandQueue.getLength();
	//console.log("Proc Q");
        if ( (self.okToSend) && (qLength>0)) {
            self.okToSend = false;
            var message = self.commandQueue.dequeue();
            self.sendNativeMessage(message);
        }
    }
    self.clearQueue = function() {
        self.commandQueue = new Queue();
        self.okToSend = true;
    }
    
    self.startQueueWorker=function () {
        setInterval(self.processQueue, 500);
    }
    
    self.stringToByteArray = function (str) {
        var b = [], i, unicode;
        for(i = 0; i < str.length; i++) {
            unicode = str.charCodeAt(i);
            // 0x00000000 - 0x0000007f -> 0xxxxxxx
            if (unicode <= 0x7f) {
                b.push(String.fromCharCode(unicode));
                // 0x00000080 - 0x000007ff -> 110xxxxx 10xxxxxx
            } else if (unicode <= 0x7ff) {
                b.push(String.fromCharCode((unicode >> 6) | 0xc0));
                b.push(String.fromCharCode((unicode & 0x3F) | 0x80));
                // 0x00000800 - 0x0000ffff -> 1110xxxx 10xxxxxx 10xxxxxx
            } else if (unicode <= 0xffff) {
                b.push(String.fromCharCode((unicode >> 12) | 0xe0));
                b.push(String.fromCharCode(((unicode >> 6) & 0x3f) | 0x80));
                b.push(String.fromCharCode((unicode & 0x3f) | 0x80));
                // 0x00010000 - 0x001fffff -> 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            } else {
                b.push(String.fromCharCode((unicode >> 18) | 0xf0));
                b.push(String.fromCharCode(((unicode >> 12) & 0x3f) | 0x80));
                b.push(String.fromCharCode(((unicode >> 6) & 0x3f) | 0x80));
                b.push(String.fromCharCode((unicode & 0x3f) | 0x80));
            }
        }
        
        return b;
    }
    
    self.sendStringAsBytes = function(str) {
        self.debug("sendStringAsBytes= " + str);
        
        var bytes= self.stringToByteArray(str);
        self.debug("bytes= " + bytes);
        self.sendBytes(bytes);
        
    }
    
    self.sendBytes = function(bytesArray) {
        var message = {
            "rawBytes":bytesArray
        };
        
        if (self.useQueues)
            self.commandQueue.enqueue(message);
        else
            self.sendNativeMessage(message);
    }
    
    self.createMessagePayloadForCommand = function(cmd, param) {
		
         return {  
			 "command":cmd,
            "p1":param,
            "rawBytes":self.createByteMessage(cmd,param)
        };
	}
	
    self.sendCommand = function(cmd, param) {
        var message = self.createMessagePayloadForCommand(cmd,param);
        
        if (self.useQueues) {
            self.debug("INFO: sendCommand(Q): " + JSON.stringify(message));
            self.commandQueue.enqueue(message);
        } else {
            self.debug("INFO: sendCommand(NoQ): " + JSON.stringify(message));
            self.sendNativeMessage(message);
        }
    }

    self.receiveBytes = function (bytesArrayBase64) {
        self.okToSend = true;
        var decoded = atob(bytesArrayBase64);
        msg = "DEBUG: receiveBytes: " + decoded;
        self.debug(msg);
        self.recvDelegate(decoded);
    }
    
    self.sendDebug = function(msg) {
        self.debug("DEBUG: " + msg);
        self.sendNativeMessage({"debug":msg});
    }
    self.sendError = function(msg) {
        self.debug("ERROR: " + msg);
        self.sendNativeMessage({"error":msg});
    }
    
    self.debug = function (msg){
        console.log(msg);
        dbgTextArea = document.querySelector("#cannybotsDebugPane")
        if (dbgTextArea) {
            dbgTextArea.innerHTML = dbgTextArea.innerHTML + "\n" + msg;
            dbgTextArea.scrollTop = dbgTextArea.scrollHeight;
        }
    }
    self.sendNativeMessage = undefined; // setup in start


    self.setupSendNativeMessage = function() {

        // iOS
        if(typeof window.webkit != 'undefined') {
            self.debug("INFO: on webkit");
            if(typeof window.webkit.messageHandlers != 'undefined') {   // iOS check
                try {
                        self.debug('INFO: setting up JavaScript native bridge for iOS');
                        self.sendNativeMessage =     function (message){
                        window.webkit.messageHandlers.interOp.postMessage(message);
                    }
                    return;
                } catch (err) {3
                    self.debug('ERROR: The native context does not exist yet');
                }
            } else {
                self.debug("WARN: iOS native bridge not found.");
            }
        }

         // Android check
        if (typeof JsHandler != 'undefined') {
           try {
               console.log('INFO: setting up JavaScript native bridge for Android');

               self.sendNativeMessage =     function (message){
                var msgStr = JSON.stringify(message);
                   self.debug('INFO: sending:' + msgStr);

                   JsHandler.jsFnCall(msgStr);
               }
               return;
           } catch (err) {
               self.debug('ERROR: The Android JS Handler does not exist yet');
           }
        }
        //console.log("WARN: Android JavaScript native bridge not found.");

        // Fallback to WebSocket
        try {
            self.debug("INFO: Using WebSocket API");
            self.websocket = cannybotsWebSocket();
            self.sendNativeMessage =     function (message){
                self.debug("Cannybots.sendWS msg:" + JSON.stringify(message) );
                self.websocket.send(JSON.stringify(message));
            }
        } catch (err) {
            self.debug("ERROR: Couldnt fallback to WebSockets" + err.message);
        }

    }
    var rChar = "\r".charCodeAt(0);
    self.createByteMessage = function(cmd, p1) {
        return [0,0, cmd.charCodeAt(0), p1>>8, p1 & 0xFF];
        
    }
    
    self.startLib = function () {
        self.setupSendNativeMessage();
		self.ping = function() {
		// if okToSend taking longer then 15 seconds
			self.sendNativeMessage(self.createMessagePayloadForCommand('?',0),0);
		}

		window.setInterval(cannybots.processQueue, 500);
		//window.setInterval(self.ping, 2000);

    }
    
}



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
