
function turtle_forward(distance) {
    cannybots.sendCommand("f",distance);
}

function turtle_backward(distance) {
    cannybots.sendCommand("b",distance);
}

function turtle_right(angle) {
    cannybots.sendCommand("r",angle);
}

function turtle_left(angle) {
    cannybots.sendCommand("l",angle);
}

function turtle_stop() {
    cannybots.sendCommand("s",0);
}