/*jslint node: true */
"use strict";

var NobleDevice = require('noble-device');
var EventEmitter = require('events').EventEmitter;
var util = require("util");   

var BB_UART_SERVICE_UUID = '7e400001b5a3f393e0a9e50e24dcca9e';
var BB_UART_NOTIFY_CHAR  = '7e400002b5a3f393e0a9e50e24dcca9e';
var BB_UART_WRITE_CHAR   = '7e400003b5a3f393e0a9e50e24dcca9e';

var BlueBrainDevice = function(peripheral) {
    if (!(this instanceof BlueBrainDevice)) 
        return new BlueBrainDevice();

    NobleDevice.call(this, peripheral);
    EventEmitter.call(this);
    
};

BlueBrainDevice.SCAN_UUIDS = [BB_UART_SERVICE_UUID];

util.inherits(BlueBrainDevice, EventEmitter);
NobleDevice.Util.inherits(BlueBrainDevice, NobleDevice);


BlueBrainDevice.prototype.connectAndSetUp = function(callback) {
    var self = this;

    NobleDevice.prototype.connectAndSetUp.call(self, function(error){

        self.notifyCharacteristic(BB_UART_SERVICE_UUID, BB_UART_NOTIFY_CHAR, true, self._onRead.bind(self), function(err){
            self.emit('ready');
            callback(err);
        });
    }.bind(self));
};


BlueBrainDevice.prototype.onDisconnect = function() {
    this.emit("disconnect");
    NobleDevice.prototype.onDisconnect.call(this);
};


BlueBrainDevice.prototype._onRead = function(msg){
    this.emit("serial", msg);
};



BlueBrainDevice.prototype.write = function(buffer,done) {
    this.writeDataCharacteristic(BB_UART_SERVICE_UUID, BB_UART_WRITE_CHAR, buffer, done);
};



module.exports = BlueBrainDevice;

