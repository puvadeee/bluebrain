module.exports = function(RED) {
    var util = require("util");   
    var async = require('async');
    var noble = require('noble');
    var events = require('events');
    var debug = require('debug')('slave');
 
 
    var cannybotUARTServiceUUID = "7e400001b5a3f393e0a9e50e24dcca9e";
    var peripheralUuid = undefined;
    //var writeCharacteristic = undefined;
 
 
    function CannybotsBLE(n) {  
        RED.nodes.createNode(this,n);  
        var node= this;  
        node.log('Cannybots created');  

        node.writeCharacteristic = undefined;
        node.peripheral = undefined;
        node.peripheralUuid = n.address;  
        node.localName      = n.name;
        node.log("addr = " +JSON.stringify(n.address));
        node.log("name = " +JSON.stringify(n.name));
         
        node.status({fill:"red",shape:"ring",text:"disconnected"}); 
        noble.startScanning([cannybotUARTServiceUUID], true);  
        
        node.errorHandler = function(error) {
            node.error('NOBLE error: ' + error);  
        
        }
        
        this.on("input", function(msg) {  
            var node= this;  

            node.log('Cannybots called: ' + JSON.stringify(msg));  
                  
            var command        = msg.payload;     
            var action         = command.action;
            var peripheralUuid = command.peripheralUuid;

            ///
            /// Action Handler
            ///
            if (action === 'startScanning') {
                noble.startScanning(serviceUuids, command.allowDuplicates);

            } else if (action === 'writeHex') {
                if (node.writeCharacteristic) {
                    command.data = String(command.data); 
                    //node.log("writeValue (string):'" + command.data +"'");
                    var data = new Buffer(command.data, "hex");
                    //node.log("writeValue (raw hex):" + data.toString("hex"));
                    node.writeCharacteristic.write(data,false);
                } else {
                    node.warn("No TX characteristic found");
                }
            } else if (action === 'writeByteArray') {
                if (node.writeCharacteristic) {
                    node.log("writeValue: '" + command.data + "'");
                    //var data = command.data ? new Buffer(command.data) : null;
                    var data = new Buffer(command.data.length);
                    for (i=0; i< command.data.length; i++)
                        data.write(command.data[i])
                    node.log("writeValue: buffer = '" + data + "'");
                    
                    node.writeCharacteristic.write(data,false); //node.errorHandler);
                } else {
                    node.warn("No TX characteristic found");
                }
            }
                
      
         
         this.on("close", function() {
             noble.stopScanning();
            if (node.peripheral) {
                node.peripheral.disconnect();   
                node.peripheral = undefined;
            }
        });
        });  
        
         // TODO: open/close ws on state change

        function sendEvent(event) {
            node.log('cb -> node.send: ' + JSON.stringify(event));
            try {
                node.send({payload:event});
                    
            } catch (err) {
                node.error(err);
            }
        }
        

        function cb_uart_rxNotify(state) {
            node.log("Notify State : " +state);         
        }

        function  cb_uart_rxData(data, isNotification) {
            node.log("RX : " +data);
            sendEvent({ 
                type: 'handleRead',
                string: data.toString(),
                rawBytes: data,
                data: data.toString('hex')
            });
            
        }
        
        noble.on('stateChange', function(state) {
            node.log("stateChange : " +state);          

            sendEvent({
                type: 'stateChange',
                state: noble.state
            });
            if (state === 'poweredOn') {
                noble.startScanning([cannybotUARTServiceUUID], true);    
            } else {
                noble.stopScanning();
            }
        });
        
        
        noble.on('discover', function(peripheral) {
            //node.log("discover : " +peripheral);            

            if (peripheral.uuid === node.peripheralUuid) {
                noble.stopScanning();
            } else {
                return;
            }
            
            node.log('peripheral with UUID ' + peripheral.uuid + ' found');
            
            node.peripheral = peripheral;
            var advertisement = peripheral.advertisement;
            var localName = advertisement.localName;
            var txPowerLevel = advertisement.txPowerLevel;
            var manufacturerData = advertisement.manufacturerData;
            var serviceData = advertisement.serviceData;
            var serviceUuids = advertisement.serviceUuids;

            if (localName) {
              node.log('  Local Name        = ' + localName);
            }

            if (txPowerLevel) {
              node.log('  TX Power Level    = ' + txPowerLevel);
            }

            if (manufacturerData) {
              node.log('  Manufacturer Data = ' + manufacturerData.toString('hex'));
            }

            if (serviceData) {
              node.log('  Service Data      = ' + serviceData);
            }

            if (serviceUuids) {
              node.log('  Service UUIDs     = ' + serviceUuids);
            }

            explore(peripheral);
          }); 
          
    
    
    function explore(peripheral) {
        
        peripheral.on('connect', function() {
            sendEvent({
              type: 'connect',
              peripheralUuid: this.uuid
            });
            node.status({fill:"green",shape:"dot",text:"connected"});

          });


          peripheral.on('disconnect', function() {
            node.status({fill:"red",shape:"ring",text:"disconnected"}); 
        
            sendEvent({
              type: 'disconnect',
              peripheralUuid: peripheral.uuid
            });
            noble.startScanning([cannybotUARTServiceUUID], true);  

          });

          peripheral.connect(function(error) {
            peripheral.discoverServices([], function(error, services) {
              var serviceIndex = 0;

              async.whilst(
                function () {
                  return (serviceIndex < services.length);
                },
                function(callback) {
                  var service = services[serviceIndex];
                  var serviceInfo = service.uuid;

                  if (service.name) {
                    serviceInfo += ' (' + service.name + ')';
                  }
                  node.log(serviceInfo);

                  service.discoverCharacteristics([], function(error, characteristics) {
                    var characteristicIndex = 0;

                    async.whilst(
                      function () {
                        return (characteristicIndex < characteristics.length);
                      },
                      function(callback) {
                        var characteristic = characteristics[characteristicIndex];
                        var characteristicInfo = '  ' + characteristic.uuid;

                        if (characteristic.name) {
                          //characteristicInfo += ' (' + characteristic.name + ')';
                        }
                        if (characteristic.uuid === "7e400002b5a3f393e0a9e50e24dcca9e") {
                            node.log("FOUND RX");
                            characteristic.notify(true, function(err) { 
                                node.log ("Notify Error?: "+err);
                                characteristic.on('notify', cb_uart_rxNotify);
                                characteristic.on('data', cb_uart_rxData);

                            });                     
                        } else if (characteristic.uuid === "7e400003b5a3f393e0a9e50e24dcca9e") {
                            node.log("FOUND TX");
                            node.writeCharacteristic = characteristic;
                        } 

                        async.series([
                          function(callback) {
                            characteristic.discoverDescriptors(function(error, descriptors) {
                              async.detect(
                                descriptors,
                                function(descriptor, callback) {
                                  return callback(descriptor.uuid === '2901');
                                },
                                function(userDescriptionDescriptor){
                                  if (userDescriptionDescriptor) {
                                    userDescriptionDescriptor.readValue(function(error, data) {
                                      if (data) {
                                        //characteristicInfo += ' (' + data.toString() + ')';
                                      }
                                      callback();
                                    });
                                  } else {
                                    callback();
                                  }
                                }
                              );
                            });
                          },
                          function(callback) {
                            //characteristicInfo += '\n    properties  ' + characteristic.properties.join(', ');
                            callback();
                            
                            /*if (characteristic.properties.indexOf('read') !== -1) {
                              characteristic.read(function(error, data) {
                                if (data) {
                                  var string = data.toString('ascii');

                                  characteristicInfo += '\n    value       ' + data.toString('hex') + ' | \'' + string + '\'';
                                }
                                callback();
                              });
                            } else {
                              callback();
                            }*/
                          },
                          function() {
                            //node.log(characteristicInfo);
                            characteristicIndex++;
                            callback();
                          }
                        ]);
                      },
                      function(error) {
                        serviceIndex++;
                        callback();
                      }
                    );
                  });
                },
                function (err) {
                    if (err) {
                        sendEvent({
                                type: 'error',
                                errorMessage:err,
                                peripheralUuid: peripheral.uuid
                                    });
                        node.error(err);
                            peripheral.disconnect();
                  }
                }
              );
            });
          });
        }
    } // CannybotsBLE

    
    
    RED.nodes.registerType("Cannybots",CannybotsBLE);  
}
