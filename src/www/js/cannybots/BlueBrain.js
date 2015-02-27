//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Cannybots BlueBRain JavaScript Library
//
// Authors:  Wayne Keenan
//
// License: http://opensource.org/licenses/MIT
//
// Version:   1.0  -  15.01.2015  -  Inital Version  (wayne@cannybots.com)
// Version:   1.1  -  05.02.2015  -  Renamed BlueRain and made a require.js cmodule  (wayne@cannybots.com)

//////////////////////////////////////////////////////////////////////////////////////////////////

define(function () {
    
    
    //Queue class: code.stephenmorley.org
    function Queue(){var a=[],b=0;this.getLength=function(){return a.length-b};this.isEmpty=function(){return 0==a.length};this.enqueue=function(b){a.push(b)};this.dequeue=function(){if(0!=a.length){var c=a[b];2*++b>=a.length&&(a=a.slice(b),b=0);return c}};this.peek=function(){return 0<a.length?a[b]:void 0}};
    
    function cannybotsWebSocket()
    {
        if ("WebSocket" in window)
        {
            var binarySupported = 'binaryType' in WebSocket.prototype;

            var ws = new WebSocket("%%WEBSOCKET_URL%%");
            
            if (binarySupported) {
                console.log("WS Binary support");
                ws.binaryType='arraybuffer';
            } else {
                console.log("WS Text mode only, no binary mode");
            }
            
            ws.onopen = function()
            {
                console.log("WS.onOpen");
            };
            ws.onmessage = function(evt) {
                console.log("WS.onMessage");
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
        self.useBinary = false;
        
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
        
        self.sendBytes = function(bytesArray) {
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
            //console.log("BB.DEBUG: receiveBytes: " + self.bytesToHex(bytesArray));
            console.log("BB.DEBUG: receiveBytes (base64): " + bytesArray);
            self.recvDelegate(atob(bytesArray));
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
            
            // iOS
            if(typeof window.webkit != 'undefined') {
                console.log("INFO: on webkit");
                if(typeof window.webkit.messageHandlers != 'undefined') {   // iOS check
                    try {
                        console.log('INFO: setting up JavaScript native bridge for iOS');
                        self.sendNativeMessage =     function (message){
                            window.webkit.messageHandlers.interOp.postMessage(message);
                        }
                        return;
                    } catch (err) {3
                        console.log('ERROR: The native context does not exist yet');
                    }
                } else {
                    console.log("WARN: iOS native bridge not found.");
                }
            }
            
            // Android check
            if (typeof JsHandler != 'undefined') {
                try {
                    console.log('INFO: setting up JavaScript native bridge for Android');
                    
                    self.sendNativeMessage =     function (message){
                        var msgStr = JSON.stringify(message);
                        console.log('INFO: sending:' + msgStr);
                        
                        JsHandler.jsFnCall(msgStr);
                    }
                    return;
                } catch (err) {
                    console.log('ERROR: The Android JS Handler does not exist yet');
                }
            }
            console.log("WARN: Android JavaScript native bridge not found.");
            
            // Fallback to WebSocket
            try {
                console.log("INFO: Using WebSocket API");
                self.websocket = cannybotsWebSocket();
                self.sendNativeMessage =     function (message){
                    //console.log("Cannybots.sendWS msg:" + JSON.stringify(message) );
                    try {
                        if (self.useBinary) {/*
                            var len =message.rawBytes.length;
                            var bytearray = new Uint8Array(len);
                            for (var i=0;i<len;++i) {
                                bytearray[i] = message.rawBytes[i];
                            }
                            
                            self.websocket.send(bytearray.buffer);
                            */
                            self.websocket.send(JSON.stringify(message));
                        }
                        else
                            self.websocket.send(JSON.stringify(message));
                    } catch (err) {
                        console.log("ERROR: sendNativeMessage, " + err);
                    }
                }
            } catch (err) {
                console.log("ERROR: Couldnt fallback to WebSockets" + err.message);
            }
            
        }
        var rChar = "\r".charCodeAt(0);
        self.createByteMessage = function(cmd, p1) {
            return [0,0,cmd.charCodeAt(0), p1>>8, p1 &0xFF,rChar];
            
        }
        
        /*
         self.bytesToHex= function (bytes) {
         for (var hex = [], i = 0; i < bytes.length; i++) {
         hex.push((bytes[i] >>> 4).toString(16));
         hex.push((bytes[i] & 0xF).toString(16));
         }
         return hex.join("");
         };*/
        
        self.startLib = function () {
            self.setupSendNativeMessage();
            self.ping = function() {
                // if okToSend taking longer then 15 seconds
                self.sendNativeMessage(self.createMessagePayloadForCommand('?',0),0);
            }
            //window.setInterval(self.ping, 2000);
            window.setInterval(cannybots.processQueue, 500);
        }
    }
    
    return cannybots;
});