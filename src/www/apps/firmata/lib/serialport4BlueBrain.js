define(['BlueBrain'], function (bluebrain) {
    
    function ab2str(buf) {
        return String.fromCharCode.apply(null, new Uint8Array(buf));
    }
    
    function str2ab(str) {
        var buf = new ArrayBuffer(str.length); // 1 bytes for each char
        var bufView = new Uint8Array(buf);
        for (var i=0, strLen=str.length; i<strLen; i++) {
            bufView[i] = str.charCodeAt(i);
            console.log("bufView[" +i+"] = " + bufView[i]);
        }
        console.log("buf = " + buf);
        return buf;
    }
    
    function SerialPort(port,  baud, settings) {
    }
    
    SerialPort.prototype.write = function(buffer) {
        console.log("Serial write: " + buffer.arr);
        bluebrain.useQueues = 0;
        bluebrain.useBinary = 1;
        bluebrain.sendBytes(buffer.arr)
    };
    
    SerialPort.prototype.on = function(eventType, callback)  {
        console.log("Serial on event:" + eventType);
        
        if (eventType == "data") {
            
            bluebrain.recvDelegate = function (bytes) {
                console.log("sp4bb, recv data len " + bytes.length);
                bytes = bytes.replace(/(\r\n|\n|\r)/gm,"");
                if (bytes.length>0) {
                    
                    callback(str2ab(bytes));
                }
                else
                    console.log("WARN: 0 length serial msg");
            };
        } else {
            // register error handler here...
        }
    };
    
    
    return SerialPort;
});