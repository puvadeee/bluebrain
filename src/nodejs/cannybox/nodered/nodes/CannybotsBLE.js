module.exports = function(RED) {
 
    

    function CannybotsBLE(n) {  
        var node= this;          
        var BlueBrain = require('./BlueBrain');
        
        RED.nodes.createNode(node,n);  

        node.log('Cannybots created');  
        node.log("addr = " +JSON.stringify(n.address));
        node.log("name = " +JSON.stringify(n.name));

        node.botName="no-name";
        node.deviceUuid   = n.address;
        
        
        node.status({fill:"red",shape:"ring",text:"disconnected"});   
       
        // BlueBrain Device discovery and events
       
        BlueBrain.discoverByUuid(node.deviceUuid, function(device) {
            node.log("Device matched: " +device);

            node.device = device;
                
            device.on("error", function(err){
                node.log("BlueBrain device error:" + err);
            });
            
            device.on("info", function(info){
                node.log("BlueBrain device info:" + info);
            });            
            
            
            device.on("ready", function(){
                node.log("BlueBrain device ready!!");
            });

            
            device.on("serial", function(buffer){
                //node.log("RX : " + buffer);
                sendEvent({ 
                    type: 'handleRead',
                    string: buffer.toString(),
                    //rawBytes: data,
                    hexBytes: buffer.toString('hex')
                });
            });
        
            device.on("disconnect", function() {
                node.log('we got disconnected!');
                node.status({fill:"red",shape:"ring",text:"disconnected"});   

            });
            
            
            device.connectAndSetUp(function(error) {
                node.log('were connected! err?=' + error);
                node.status({fill:"green",shape:"dot",text:"connected to '"+device._peripheral.advertisement.localName+"'"});
                BlueBrain.startScanning();
            });
        });      
        
        
        // Node events
        
        node.errorHandler = function(error) {
            node.error('CannybotsBLEs error: ' + error);          
        }
        
        node.on("input", function(msg) {  

            node.log('Cannybots Node BLE called: ' + JSON.stringify(msg));  
             if (!node.device) {
                 node.error("No device");
             }
                  
            var command        = msg.payload;     
            var action         = command.action;

            ///
            /// Action Handler
            ///
            if (action === 'writeHex') {
                var data = new Buffer( String(command.data).toLowerCase(), "hex");
                node.log("writeValue (raw hex):" + data.toString("hex"));
                node.device.send(data,false);
            } 
            else if (action === 'writeByteArray') 
            {
                var data = new Buffer(command.data.length);
                for (i=0; i< command.data.length; i++)
                    data.write(command.data[i])
                node.log("writeValue: buffer = '" + data + "'");
                
                node.device.send(data,false); //node.errorHandler);
            }
        });
      
         
        node.on("close", function() {
            node.log('Disconnecting from Device...');
            setTimeout(node.device.disconnect.bind(node.device, function(){}), 2000);
        });  
        
        
        function sendEvent(event) {
            node.log('cb -> node.send: ' + JSON.stringify(event));
            try {
                node.send({payload:event});
                    
            } catch (err) {
                node.error(err);
            }
        }
    
    }

    RED.nodes.registerType("Cannybots",CannybotsBLE);  
}
