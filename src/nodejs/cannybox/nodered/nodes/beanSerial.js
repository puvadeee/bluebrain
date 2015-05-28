/**
 * Copyright 2014 IBM Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/

// If you use this as a template, update the copyright with your own name.

// Sample Node-RED node file

module.exports = function(RED) {

    var bleBean = require("./BlueBrainDevice");
    var events = require('events');
    var beanNode = require('./beanNodeStatusMixin.js');

    function BeanSerialNode(n) {
        // Create a RED node
        RED.nodes.createNode(this,n);

        this.log("A Cannybots Serial node is being instantiated");

        // Store local copies of the node configuration (as defined in the .html)
        this.topic = n.topic;
        this.bean = n.bean
        this.beanConfig = RED.nodes.getNode(this.bean);

        // respond to inputs....
        this.on('input', function (msg) {
            if(this.beanConfig){
                var command        = msg.payload;     
                var action         = command.action;

                if (action === 'writeHex') {
                    var data = new Buffer( String(command.data).toLowerCase(), "hex");
                    this.log("writeValue (raw hex):" + data.toString("hex"));
                    this.beanConfig.device.write(data, function(){})
                } 
                else if (action === 'writeByteArray') 
                {
                    var data = new Buffer(command.data.length);
                    for (i=0; i< command.data.length; i++)
                        data.write(command.data[i])
                    this.log("writeValue: buffer = '" + data + "'");
                    this.beanConfig.device.write(data, function(){})
                }
            }
        });

        var serialDataRxFromBean = function(data, valid){
            this.log("RX : " + data);
            this.send({payload:{ 
                type: 'handleRead',
                string: data.toString(),
                hexBytes: data.toString('hex')
                }});
        }.bind(this);

        if (this.beanConfig) {
            this.beanConfig.on("connected", function() {
                this.beanConfig.device.on('serial',serialDataRxFromBean);
            }.bind(this));
        }
        
        this.on("close", function(done) {
            done();
        });

        if (this.beanConfig) {
            beanNode.configureBeanStatuses.call(this);
        }
    }

    // Register the node by name. This must be called before overriding any of the
    // Node functions.
    RED.nodes.registerType("bean serial",BeanSerialNode);

}
