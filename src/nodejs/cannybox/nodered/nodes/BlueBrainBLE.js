module.exports = function(RED) {
 
    var events = require('events');
    var BlueBrainDevice = require('./BlueBrainDevice');
    var beanScanner = require('./BlueBrainScanner.js');
    

    function BlueBrain(n) {  
        var verboseLog = function (msg){
            if (RED.settings.verbose) {
                this.log(msg);
            }
        }.bind(this)
        
        
        var node= this;          

        RED.nodes.createNode(node,n);  
        events.EventEmitter.call(this);
        verboseLog("A Cannybots config node is being instantiated");
               
        // Unlimited listeners
        this.setMaxListeners(0);
  
        node.log("uuid = " +JSON.stringify(n.uuid));
        node.log("name = " +JSON.stringify(n.name));

        node.botName="no-name";
        node.name = n.name;
        node.uuid = n.uuid;
        node.device;
        
        node.connectiontype = "constant";//n.connectiontype;

        node._isAttemptingConnection = false;
        node._isConnectedAndSetUp = false;
       
        // BlueBrain Device discovery and events
       
        // Called after a Bean has been disconnected
        var _hasDisconnected = function (){
            node._isConnectedAndSetUp = false;
            node._isAttemptingConnection = false;
            node.status({fill:"red",shape:"ring",text:"disconnected"});   

            verboseLog("We disconnected from the Cannybot with name \"" + node.name + "\" and uuid " + node.device.uuid);
            node.emit("disconnected");
            
            if(node.connectiontype == 'constant' &&
                node.isBeingDestroyed !== true){
                _attemptConnection();
            }
        }.bind(node);
       
        // Called after a Bean has successfully connected
        var _hasConnected = function (){
            node._isConnectedAndSetUp = true;
            node.status({fill:"green",shape:"dot",text:"connected to '"+ node.device._peripheral.advertisement.localName+"'"});
            verboseLog("We connected to the Cannybot with name \"" + node.name + "\" and uuid " + node.device.uuid);
            node.emit("connected");

        }.bind(node);
       
       // This function will attempt to connect to a Bean. 
        var _attemptConnection = function(){
            if(node._isAttemptingConnection === true ||
                node.isBeingDestroyed === true){ 
                verboseLog("Already in a connection attempt to the Bean with name \"" + node.name + "\"");
                return false; 
            }

            verboseLog("Scanning for the Cannybot with addr \"" + node.name + "\"");

            node._isAttemptingConnection = true;

            node.emit("searching");

            var onDiscovery = function(device) {
                node.device = device;
                verboseLog("We found a desired Cannybot Cannybot with name \"" + node.name + "\" and uuid " + node.device.uuid);
                node.emit("connecting");
                node.device.connectAndSetUp(function(){
                    node._isAttemptingConnection = false;
                    node.device.once('disconnect', _hasDisconnected);
                    _hasConnected();
                }.bind(node))
            }.bind(node)

            BlueBrainDevice.discoverWithFilter(function(device) {
                verboseLog("looking for node.uuid = " + node.uuid + ", got device.uuid = " + device.uuid);
                if ( (node.uuid !== undefined) && (device.uuid !== node.uuid)) 
                    return false;
                return (device.uuid === node.uuid) || (device._peripheral.advertisement.localName === node.name);
            }.bind(node), onDiscovery);

            return true;
        }.bind(node)
       
       
        // Used to check if this node is currently conencted to a Cannybot
        var _isConnected = function (){
            if(node.device
                && node._isConnectedAndSetUp === true
                && node.device._peripheral.state == 'connected'
                && ((node.device.connectAndSetUp) ? node.device.connectAndSetUp === true : true)){
                return true;
            }else{
                return false;
            }
        }.bind(node)
        
        var _performFunctionWhenConnected = function(aFunction){
            if(_isConnected() === true){
                aFunction.call(node);
            }else{
                _attemptConnection(node.connectiontimeout);
            }
        }.bind(node)
        
        
        
        var write = function(data, done){
            _performFunctionWhenConnected(function(){
                node.device.write(data, done);
            })
        }.bind(node)


        

        // This is a second precaution in case the "disconnect" event isn't reached 
        if(node.connectiontype === 'constant'){
            // Queue up a call to attempt initial connection. 
            // This lets the Bean nodes that depend on this configuration get setup before connction is attempted
            setImmediate(function(){
                _attemptConnection();
            })

            // Check connection status periodically and attempt to reconnect if disconnceted
            node.reconnectInterval = setInterval(function(){
                if(_isConnected() === false){
                    _attemptConnection();
                }else{
                    verboseLog("We are currently connected to the Cannybot with uuid \"" + node.uuid + "\"");
                }
            }.bind(node), 10*1000)
        }

        // This event handle this Bean config node being destroyed
        node.on("close", function(done) {
            verboseLog("A Cannybot config node is being destroyed");
            node.isBeingDestroyed = true;
            clearInterval(node.reconnectInterval);
            //beanScanner.stopScanning();
            // This is a hack. It clears all scan requests for noble-device. 
            // If every other bean config node isn't also being reset now, then we have issues
            BlueBrainDevice.emitter.removeAllListeners('discover');  
            if (node._isConnected()) {
                node.device.disconnect(function(){
                    verboseLog("A Cannybot config node is finished being destroyed");
                    done();
                }.bind(node));
            }else{
                verboseLog("A Cannybot config node is finished being destroyed");
                done();
            }
        });
    
    
 

    }

    RED.nodes.registerType("BlueBrain",BlueBrain);  
    
    RED.httpAdmin.get("/discoveredbluebrains",function(req,res) {
        res.writeHead(200, {'Content-Type': 'text/plain'});
        var beans = beanScanner.getDiscoveredBeans();
        var beanReport =[];
        for (i = 0; i < beans.length; i++) {
            beanReport.push({
                "name":beans[i]._peripheral.advertisement.localName,
                "uuid":beans[i].uuid,
                "rssi":beans[i]._peripheral.rssi
            });
        }

        res.write(JSON.stringify(beanReport));
        res.end();
    });
}
