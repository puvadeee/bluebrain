define(['BlueBrain'], function (cannybots) {
    
        
    function SerialPort(port,  baud, settings) {
    }
    
    SerialPort.prototype.write = function(buffer) {
        cannybots.debug("Serial write: " + buffer.arr);
        cannybots.useQueues = 0;
        cannybots.useBinary = 1;
        cannybots.sendBytes(buffer.arr)
    };
    
    SerialPort.prototype.on = function(eventType, callback)  {
        //console.log("Serial on event:" + eventType);
        
        if (eventType == "data") {
            cannybots.recvDelegate = function (msg) {
				var obj = JSON.parse(msg);
				if (obj.rawBytes && obj.rawBytes.length>0) {
					callback(obj.rawBytes);
				}
            };

        } else {
            // register error handler here...
        }
    };
    
    
    return SerialPort;
});
