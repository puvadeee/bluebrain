'use strict';

// Cannyblocks Blockly blocks...

// Cannybots

Blockly.Blocks['cannyblocks_cannybots_receive_message'] = {
    init: function() {
        this.setHelpUrl('http://www.cannybots.com/cannyblocks/cannybots/');
        this.setColour(20);
        this.appendDummyInput()
        .appendField("Cannybot Message to")
        .appendField(new Blockly.FieldVariable("variable"), "Message");
        this.setNextStatement(true, "String");
        this.setTooltip('');
    }
};


Blockly.JavaScript['cannyblocks_cannybots_receive_message'] = function(block) {
    var variable_message = Blockly.JavaScript.variableDB_.getName(block.getFieldValue('Message'), Blockly.Variables.NAME_TYPE);
    var code = variable_message +  ' = cannybotsGetMessage();\n'
    
    return code;
};



Blockly.Blocks['cannyblocks_cannybots_send_message'] = {
init: function() {
    this.setHelpUrl('http://www.cannybots.com/cannyblocks/cannybots/');
    this.setColour(20);
    this.appendValueInput("message")
    .setCheck("String")
    .appendField("Send Message");
    this.setInputsInline(true);
    this.setPreviousStatement(true);
    this.setNextStatement(true);
    this.setTooltip('');
}
};

Blockly.JavaScript['cannyblocks_cannybots_send_message'] = function(block) {
    var value_message = Blockly.JavaScript.valueToCode(block, 'message', Blockly.JavaScript.ORDER_ATOMIC);
    // TODO: Assemble JavaScript into code variable.
    var code = 'cannybotsSendMessage(' + value_message + ');\n';

    return code;
};


// Logo Turtle

// Forward

Blockly.Blocks['cannyblocks_turtle_forward'] = {
init: function() {
    this.setHelpUrl('http://www.cannybots.com/cannyblocks/turtle/');
    this.setColour(0);
    this.appendValueInput("Distance")
    .setCheck("Number")
    .setAlign(Blockly.ALIGN_CENTRE)
    .appendField("Move Turtle Forward");
    this.setPreviousStatement(true, "null");
    this.setNextStatement(true, "null");
    this.setTooltip('');
}
};

Blockly.JavaScript['cannyblocks_turtle_forward'] = function(block) {
    var value_distance = Blockly.JavaScript.valueToCode(block, 'Distance', Blockly.JavaScript.ORDER_ATOMIC);

    var code = 'turtle_forward(' + value_distance +');\n';
    return code;
};

// Backward

Blockly.Blocks['cannyblocks_turtle_backward'] = {
init: function() {
    this.setHelpUrl('http://www.cannybots.com/cannyblocks/turtle/');
    this.setColour(0);
    this.appendValueInput("Distance")
    .setCheck("Number")
    .setAlign(Blockly.ALIGN_CENTRE)
    .appendField("Move Turtle Backward");
    this.setPreviousStatement(true, "null");
    this.setNextStatement(true, "null");
    this.setTooltip('');
}
};

Blockly.JavaScript['cannyblocks_turtle_backward'] = function(block) {
    var value_distance = Blockly.JavaScript.valueToCode(block, 'Distance', Blockly.JavaScript.ORDER_ATOMIC);
    
    var code = 'turtle_backward(' + value_distance +');\n';
    return code;
};



// Right

Blockly.Blocks['cannyblocks_turtle_right'] = {
init: function() {
    this.setHelpUrl('http://www.cannybots.com/cannyblocks/turtle/');
    this.setColour(0);
    this.appendValueInput("Angle")
    .setCheck("Number")
    .setAlign(Blockly.ALIGN_CENTRE)
    .appendField("Turn Turtle Right");
    this.setPreviousStatement(true, "null");
    this.setNextStatement(true, "null");
    this.setTooltip('');
}
};

Blockly.JavaScript['cannyblocks_turtle_right'] = function(block) {
    var value_angle= Blockly.JavaScript.valueToCode(block, 'Angle', Blockly.JavaScript.ORDER_ATOMIC);
    
    var code = 'turtle_right(' + value_angle +');\n';
    return code;
};


// Left

Blockly.Blocks['cannyblocks_turtle_left'] = {
init: function() {
    this.setHelpUrl('http://www.cannybots.com/cannyblocks/turtle/');
    this.setColour(0);
    this.appendValueInput("Angle")
    .setCheck("Number")
    .setAlign(Blockly.ALIGN_CENTRE)
    .appendField("Turn Turtle Left");
    this.setPreviousStatement(true, "null");
    this.setNextStatement(true, "null");
    this.setTooltip('');
}
};

Blockly.JavaScript['cannyblocks_turtle_left'] = function(block) {
    var value_angle= Blockly.JavaScript.valueToCode(block, 'Angle', Blockly.JavaScript.ORDER_ATOMIC);
    
    var code = 'turtle_left(' + value_angle +');\n';
    return code;
};
