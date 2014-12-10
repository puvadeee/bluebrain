package com.cannybots.views.joystick;
public interface JoystickMovedListener {

    public void OnMoved(int pan, int tilt);

    public void OnReleased();

}