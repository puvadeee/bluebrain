'use strict';

// Blockly blocks...

// Forward

Blockly.Blocks['cannyblocks_turtle_forward'] = {
init: function() {
    this.setHelpUrl('http://www.cannybots.com/blockly/turtle/');
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
    this.setHelpUrl('http://www.cannybots.com/blockly/turtle/');
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
    this.setHelpUrl('http://www.cannybots.com/blockly/turtle/');
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
    this.setHelpUrl('http://www.cannybots.com/blockly/turtle/');
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
