package com.cannybots.views.joystick;
// from: https://code.google.com/p/mobile-anarchy-widgets/wiki/JoystickView


import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.RectF;
import android.os.Handler;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;

import com.cannybots.cannybotsapp.R;

public class ThrottleView extends JoystickView {

    // =========================================
    // Private Members
    // =========================================

    private final String TAG = "ThrottleView";
    private final int MIN_THROTTLE = 0;

    Paint throttleKnobPaint;
    Bitmap throttleKnobBitmap;
    Paint throttleLevelPaint;
    // =========================================
    // Constructors
    // =========================================

    public ThrottleView(Context context) {
        super (context);
        initThrottleView();
    }

    public ThrottleView(Context context, AttributeSet attrs) {
        super (context, attrs);
        initThrottleView();
    }

    public ThrottleView(Context context, AttributeSet attrs,
                        int defStyle) {
        super (context, attrs, defStyle);
        initThrottleView();
    }

    // =========================================
    // Initialization
    // =========================================

    private void initThrottleView() {
        setFocusable(true);

        circlePaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        circlePaint.setColor(Color.DKGRAY);
        circlePaint.setStrokeWidth(1);
        circlePaint.setStyle(Paint.Style.FILL_AND_STROKE);

        handlePaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        handlePaint.setColor(Color.RED);
        handlePaint.setStrokeWidth(1);
        handlePaint.setStyle(Paint.Style.FILL_AND_STROKE);

        throttleLevelPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        throttleLevelPaint.setColor(Color.rgb(234,168,40));
        throttleLevelPaint.setStrokeWidth(1);
        throttleLevelPaint.setStyle(Paint.Style.FILL_AND_STROKE);


        innerPadding = 10;
        sensitivity = 255;

        touchY = 180;

        if (USE_JOYSTICK_BITMAPS) {
            throttleKnobPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
            throttleKnobBitmap = BitmapFactory.decodeResource(getResources(), R.drawable.throttle_knob);
        }
    }

    // =========================================
    // Public Methods
    // =========================================

    public void setOnJostickMovedListener(JoystickMovedListener listener) {
        this .listener = listener;
    }

    // =========================================
    // Drawing Functionality
    // =========================================

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {

        int measuredWidth = measure(widthMeasureSpec);
        int measuredHeight = measure(heightMeasureSpec);
        int d = Math.min(measuredWidth, measuredHeight);

        handleRadius = (int) (d * 0.1);
        handleInnerBoundaries = handleRadius*2;

        setMeasuredDimension(d, d-handleRadius);
    }

    private int measure(int measureSpec) {
        int result = 0;
        // Decode the measurement specifications.
        int specMode = MeasureSpec.getMode(measureSpec);
        int specSize = MeasureSpec.getSize(measureSpec);
        if (specMode == MeasureSpec.UNSPECIFIED) {
            // Return a default size of 200 if no bounds are specified.
            result = 100;
        } else {
            // As you want to fill the available space
            // always return the full available bounds.
            result = specSize;
        }
        return result;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        int px = getMeasuredWidth() / 2;
        int py = getMeasuredHeight() / 2;

        int width = innerPadding;

        // Draw the background
        canvas.drawRect(px-width, handleRadius, px+width, getMeasuredHeight()-handleRadius, circlePaint);

        canvas.drawRect(px-width, (int) touchY + py - handleRadius, px+width, getMeasuredHeight()-handleRadius, throttleLevelPaint);

        if (!USE_JOYSTICK_BITMAPS) {
            // Draw the handle
            RectF rect = new RectF(
                    (int) px - handleRadius,
                    (int) touchY + py - handleRadius,
                    (int) px+handleRadius,
                    (int) touchY + py
            );
            canvas.drawOval(rect, handlePaint);
        } else {
            Rect throttleKnobRect = new Rect(
                    px + (int) touchX - handleRadius,
                    py + (int) touchY - handleRadius,
                    px + (int) touchX + handleRadius,
                    py + (int) touchY + handleRadius/2);

            canvas.drawBitmap(throttleKnobBitmap, null, throttleKnobRect, throttleKnobPaint);


        }


        canvas.save();
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int actionType = event.getAction();
        if (actionType == MotionEvent.ACTION_MOVE) {
            int py = getMeasuredHeight() / 2;
            int radius = py - handleInnerBoundaries;

            touchY = (event.getY() - py);
            //touchY = Math.max(Math.min(touchY, radius), -radius);

            // Coordinates
            Log.d(TAG, "Y:" + touchY);

            // Pressure
            if (listener != null) {
                listener.OnMoved(0,(int) (touchY / radius * sensitivity));
            }

            invalidate();
        } else if (actionType == MotionEvent.ACTION_UP) {
            returnHandleToCenter();
            Log.d(TAG, "Y:" + touchY);
        }
        return true;
    }

    private void returnHandleToCenter() {

        Handler handler = new Handler();
        int numberOfFrames = 5;
        final double intervalsY = (sensitivity - touchY-handleRadius) / numberOfFrames;

        for (int i = 0; i < numberOfFrames; i++) {
            handler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    touchY += intervalsY;
                    invalidate();
                }
            }, i * 40);
        }

        if (listener != null) {
            listener.OnReleased();
        }
    }
}
