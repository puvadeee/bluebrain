package com.cannybots.views.joystick;
// from: https://code.google.com/p/mobile-anarchy-widgets/wiki/JoystickView


        import android.content.Context;
        import android.content.res.Resources;
        import android.graphics.Bitmap;
        import android.graphics.BitmapFactory;
        import android.graphics.Canvas;
        import android.graphics.Color;
        import android.graphics.Paint;
        import android.graphics.Rect;
        import android.graphics.drawable.Drawable;
        import android.os.Handler;
        import android.util.AttributeSet;
        import android.util.Log;
        import android.view.MotionEvent;
        import android.view.View;
        import android.widget.ImageView;

        import com.cannybots.cannybotsapp.R;

public class JoystickView extends View {
    static final boolean USE_JOYSTICK_BITMAPS = true;

    // =========================================
    // Private Members
    // =========================================

    private final String TAG = "JoystickView";
    Paint circlePaint;
    Paint handlePaint;
    double touchX, touchY;
    int innerPadding;
    int handleRadius;
    int handleInnerBoundaries;
    JoystickMovedListener listener;
    int sensitivity;


    Paint knobPaint;
    Bitmap knobBitmap;

    Paint joystickPaint;
    Bitmap joystickBitmap;



    // =========================================
    // Constructors
    // =========================================

    public JoystickView(Context context) {
        super (context);
        initJoystickView();
    }

    public JoystickView(Context context, AttributeSet attrs) {
        super (context, attrs);
        initJoystickView();
    }

    public JoystickView(Context context, AttributeSet attrs,
                        int defStyle) {
        super (context, attrs, defStyle);
        initJoystickView();
    }

    // =========================================
    // Initialization
    // =========================================

    private void initJoystickView() {
        setFocusable(true);

        if (USE_JOYSTICK_BITMAPS) {
            knobPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
            knobBitmap = BitmapFactory.decodeResource(getResources(), R.drawable.knob);
            joystickPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
            joystickBitmap = BitmapFactory.decodeResource(getResources(), R.drawable.joystick);
        }

        circlePaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        circlePaint.setColor(Color.DKGRAY);
        circlePaint.setStrokeWidth(1);
        circlePaint.setStyle(Paint.Style.FILL_AND_STROKE);

        handlePaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        handlePaint.setColor(Color.RED);
        handlePaint.setStrokeWidth(1);
        handlePaint.setStyle(Paint.Style.FILL_AND_STROKE);

        innerPadding = 10;
        sensitivity = 255;
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
        // Here we make sure that we have a perfect circle
        int measuredWidth = measure(widthMeasureSpec);
        int measuredHeight = measure(heightMeasureSpec);
        int d = Math.min(measuredWidth, measuredHeight);

        handleRadius = (int) (d * 0.25);
        handleInnerBoundaries = handleRadius;

        setMeasuredDimension(d, d);
    }

    private int measure(int measureSpec) {
        int result = 0;
        // Decode the measurement specifications.
        int specMode = MeasureSpec.getMode(measureSpec);
        int specSize = MeasureSpec.getSize(measureSpec);
        if (specMode == MeasureSpec.UNSPECIFIED) {
            // Return a default size of 200 if no bounds are specified.
            result = 200;
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
        int radius = Math.min(px, py);

        if (!USE_JOYSTICK_BITMAPS) {
            // Draw the background
            canvas.drawCircle(px, py, radius - innerPadding, circlePaint);

            // Draw the handle
            canvas.drawCircle((int) touchX + px, (int) touchY + py, handleRadius, handlePaint);
        } else {
            Rect joystickRect = new Rect(
                    px -  radius + innerPadding,
                    py -  radius + innerPadding,
                    px +  radius - innerPadding,
                    py +  radius - innerPadding);

            canvas.drawBitmap(joystickBitmap, null, joystickRect, joystickPaint);


            //canvas.drawBitmap(knobBitmap, (float) (touchX+px), (float) (touchY + py), knobPaint);
            Rect knobRect = new Rect(
                    px + (int) touchX - handleRadius,
                    py + (int) touchY - handleRadius,
                    px + (int) touchX + handleRadius,
                    py + (int) touchY + handleRadius);

            canvas.drawBitmap(knobBitmap, null, knobRect, knobPaint);


        }

        canvas.save();
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int actionType = event.getAction();
        if (actionType == MotionEvent.ACTION_MOVE) {
            int px = getMeasuredWidth() / 2;
            int py = getMeasuredHeight() / 2;
            int radius = Math.min(px, py) - handleInnerBoundaries;

            touchX = (event.getX() - px);
            touchX = Math.max(Math.min(touchX, radius), -radius);

            touchY = (event.getY() - py);
            touchY = Math.max(Math.min(touchY, radius), -radius);

            // Coordinates
            //Log.d(TAG, "X:" + touchX + "|Y:" + touchY);

            // Pressure
            if (listener != null) {
                listener.OnMoved((int) (touchX / radius * sensitivity),
                        (int) (touchY / radius * sensitivity));
            }

            invalidate();
        } else if (actionType == MotionEvent.ACTION_UP) {
            returnHandleToCenter();
            //Log.d(TAG, "X:" + touchX + "|Y:" + touchY);
        }
        return true;
    }

    private void returnHandleToCenter() {

        Handler handler = new Handler();
        int numberOfFrames = 5;
        final double intervalsX = (0 - touchX) / numberOfFrames;
        final double intervalsY = (0 - touchY) / numberOfFrames;

        for (int i = 0; i < numberOfFrames; i++) {
            handler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    touchX += intervalsX;
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
