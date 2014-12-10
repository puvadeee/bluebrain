package com.cannybots.cannybotsapp;




import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.support.v7.app.ActionBarActivity;
import android.support.v7.app.ActionBar;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentTransaction;
import android.support.v4.app.FragmentPagerAdapter;
import android.os.Bundle;
import android.support.v4.view.ViewPager;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;

import com.cannybots.ble.BluetoothHelper;
import com.cannybots.ble.HexAsciiHelper;
import com.cannybots.ble.RFduinoService;
import com.cannybots.views.joystick.*;

import android.util.Log;
import android.widget.FrameLayout;


import java.util.Timer;
import java.util.TimerTask;
import java.util.UUID;

// Disable Swiping, see: https://blog.svpino.com/2011/08/29/disabling-pagingswiping-on-android
// also see in project: /res/layout/activity_main.xml
class CustomViewPager extends ViewPager {

    private boolean enabled;

    public CustomViewPager(Context context, AttributeSet attrs) {
        super(context, attrs);
        this.enabled = false;
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (this.enabled) {
            return super.onTouchEvent(event);
        }

        return false;
    }

    @Override
    public boolean onInterceptTouchEvent(MotionEvent event) {
        if (this.enabled) {
            return super.onInterceptTouchEvent(event);
        }

        return false;
    }

    public void setPagingEnabled(boolean enabled) {
        this.enabled = enabled;
    }
}

public class MainActivity extends ActionBarActivity implements ActionBar.TabListener,BluetoothAdapter.LeScanCallback {

    private static final String TAG = "CannybotsActivity";

    /////////////////////////////////////////////
    /// BLE
    // State machine
    final private static int STATE_BLUETOOTH_OFF = 1;
    final private static int STATE_DISCONNECTED = 2;
    final private static int STATE_CONNECTING = 3;
    final private static int STATE_CONNECTED = 4;

    private int state;

    private boolean scanStarted;
    private boolean scanning;

    private BluetoothAdapter bluetoothAdapter;
    private BluetoothDevice bluetoothDevice;

    public static RFduinoService rfduinoService;

    // BLE state managment

    private void upgradeState(int newState) {
        if (newState > state) {
            updateState(newState);
        }
    }

    private void downgradeState(int newState) {
        if (newState < state) {
            updateState(newState);
        }
    }

    private void updateState(int newState) {
        state = newState;
        //updateUi();
    }

    // BLE callbacks

    private final BroadcastReceiver bluetoothStateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, 0);
            if (state == BluetoothAdapter.STATE_ON) {
                upgradeState(STATE_DISCONNECTED);
            } else if (state == BluetoothAdapter.STATE_OFF) {
                downgradeState(STATE_BLUETOOTH_OFF);
            }
        }
    };

    private final BroadcastReceiver scanModeReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            scanning = (bluetoothAdapter.getScanMode() != BluetoothAdapter.SCAN_MODE_NONE);
            scanStarted &= scanning;
            //updateUi();
        }
    };

    private final ServiceConnection rfduinoServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            rfduinoService = ((RFduinoService.LocalBinder) service).getService();
            if (rfduinoService.initialize()) {
                if (bluetoothDevice != null) {
                    if (rfduinoService.connect(bluetoothDevice.getAddress())) {
                        upgradeState(STATE_CONNECTING);
                    }
                }
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            rfduinoService = null;
            downgradeState(STATE_DISCONNECTED);
        }
    };

    private final BroadcastReceiver rfduinoReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            if (RFduinoService.ACTION_CONNECTED.equals(action)) {
                upgradeState(STATE_CONNECTED);
            } else if (RFduinoService.ACTION_DISCONNECTED.equals(action)) {
                downgradeState(STATE_DISCONNECTED);
                BLE_disconnected();

            } else if (RFduinoService.ACTION_DATA_AVAILABLE.equals(action)) {
                addData(intent.getByteArrayExtra(RFduinoService.EXTRA_DATA));
            }
        }
    };


    @Override
    public void onLeScan(BluetoothDevice device, final int rssi, final byte[] scanRecord) {
        byte advData[] = BluetoothHelper.parseAdvertisementFromScanRecord(scanRecord);
        Log.d(TAG, "Potential BLE Device found name = " + device.getName());
        if ( (advData[0] == 'C') && (advData[1] == 'B') && (advData[2] == '0') && (advData[3] == '1') ) {
           bluetoothDevice = device;
           BLE_stopScanning();
           BLE_connect();
           Log.d(TAG, "Found Cannybot! BLE Device Info = " + BluetoothHelper.getDeviceInfoText(device, rssi, scanRecord));
        }
    }

    private void BLE_onCreate() {
        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        bluetoothAdapter.enable();
    }


    private void BLE_onStart() {
        registerReceiver(scanModeReceiver, new IntentFilter(BluetoothAdapter.ACTION_SCAN_MODE_CHANGED));
        registerReceiver(bluetoothStateReceiver, new IntentFilter(BluetoothAdapter.ACTION_STATE_CHANGED));
        registerReceiver(rfduinoReceiver, RFduinoService.getIntentFilter());

        updateState(bluetoothAdapter.isEnabled() ? STATE_DISCONNECTED : STATE_BLUETOOTH_OFF);

        BLE_startScanning();
        Joystick_stopTimer();

        joypadUpdateTimer = new Timer();

        joypadUpdateTimer.scheduleAtFixedRate(
                new TimerTask() {
                    public void run() {
                        PlaceholderFragment.sendJoypadUpdate(false);
                    }
                },
                0,
                50);
    }


    private void Joystick_stopTimer() {
        if (joypadUpdateTimer!=null) {
            joypadUpdateTimer.cancel();
            joypadUpdateTimer.purge();

        }
    }

    private void BLE_onStop() {
        downgradeState(STATE_DISCONNECTED);

        Joystick_stopTimer();
        BLE_stopScanning();
        unregisterReceiver(scanModeReceiver);
        unregisterReceiver(bluetoothStateReceiver);
        unregisterReceiver(rfduinoReceiver);
    }



    private void BLE_startScanning() {
        // when scan pressed
        scanStarted = true;

        // find by 16 bit short code
        //bluetoothAdapter.startLeScan(new UUID[]{RFduinoService.UUID_SERVICE}, MainActivity.this);

        // Find all (then filter in 'onLeScan' callback )
        bluetoothAdapter.startLeScan(MainActivity.this);
    }

    private void BLE_stopScanning() {
        scanStarted = false;
        bluetoothAdapter.stopLeScan(this);
    }

    Timer joypadUpdateTimer;

    private void BLE_connect() {
        // when 'connect' pressed
        Intent rfduinoIntent = new Intent(MainActivity.this, RFduinoService.class);
        bindService(rfduinoIntent, rfduinoServiceConnection, BIND_AUTO_CREATE);
    }

    // BLE UI
    private void addData(byte[] data) {
        Log.i(TAG, "RECV: " + HexAsciiHelper.bytesToHex(data));
    }


    private void BLE_disconnected() {
        Log.i(TAG, "******  BLE_disconnected *******");

        bluetoothDevice = null;
        BLE_onStop();
        BLE_onStart();
    }


    @Override
    protected void onStart() {
        super.onStart();
        BLE_onStart();
    }

    @Override
    protected void onStop() {
        super.onStop();
        BLE_onStop();
    }

    /////////////////////////////////////////////




    /**
     * The {@link android.support.v4.view.PagerAdapter} that will provide
     * fragments for each of the sections. We use a
     * {@link FragmentPagerAdapter} derivative, which will keep every
     * loaded fragment in memory. If this becomes too memory intensive, it
     * may be best to switch to a
     * {@link android.support.v4.app.FragmentStatePagerAdapter}.
     */
    SectionsPagerAdapter mSectionsPagerAdapter;

    /**
     * The {@link ViewPager} that will host the section contents.
     */
    CustomViewPager mViewPager;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Set up the action bar.
        final ActionBar actionBar = getSupportActionBar();
        actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);

        // Create the adapter that will return a fragment for each of the three
        // primary sections of the activity.
        mSectionsPagerAdapter = new SectionsPagerAdapter(getSupportFragmentManager());

        // Set up the ViewPager with the sections adapter.
        mViewPager = (CustomViewPager) findViewById(R.id.pager);
        mViewPager.setAdapter(mSectionsPagerAdapter);

        // When swiping between different sections, select the corresponding
        // tab. We can also use ActionBar.Tab#select() to do this if we have
        // a reference to the Tab.
        mViewPager.setOnPageChangeListener(new ViewPager.SimpleOnPageChangeListener() {
            @Override
            public void onPageSelected(int position) {
                actionBar.setSelectedNavigationItem(position);
            }
        });

        // For each of the sections in the app, add a tab to the action bar.
        for (int i = 0; i < mSectionsPagerAdapter.getCount(); i++) {
            // Create a tab with text corresponding to the page title defined by
            // the adapter. Also specify this Activity object, which implements
            // the TabListener interface, as the callback (listener) for when
            // this tab is selected.
            actionBar.addTab(
                    actionBar.newTab()
                            .setText(mSectionsPagerAdapter.getPageTitle(i))
                            .setTabListener(this));
        }

        BLE_onCreate();
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onTabSelected(ActionBar.Tab tab, FragmentTransaction fragmentTransaction) {
        // When the given tab is selected, switch to the corresponding page in
        // the ViewPager.
        mViewPager.setCurrentItem(tab.getPosition());
    }

    @Override
    public void onTabUnselected(ActionBar.Tab tab, FragmentTransaction fragmentTransaction) {
    }

    @Override
    public void onTabReselected(ActionBar.Tab tab, FragmentTransaction fragmentTransaction) {
    }

    /**
     * A {@link FragmentPagerAdapter} that returns a fragment corresponding to
     * one of the sections/tabs/pages.
     */
    public class SectionsPagerAdapter extends FragmentPagerAdapter {

        public SectionsPagerAdapter(FragmentManager fm) {
            super(fm);
        }

        @Override
        public Fragment getItem(int position) {
            // getItem is called to instantiate the fragment for the given page.
            // Return a PlaceholderFragment (defined as a static inner class below).
            return PlaceholderFragment.newInstance(position + 1);
        }

        @Override
        public int getCount() {
            // Show 3 total pages.
            return 1;
        }

        @Override
        public CharSequence getPageTitle(int position) {
            Log.i(TAG, "wrtf!!!");
            switch (position) {
                case 0:
                    return getString(R.string.title_section1).toUpperCase();
                case 1:
                    return getString(R.string.title_section2).toUpperCase();
                case 2:
                    return getString(R.string.title_section3).toUpperCase();
            }
            return null;
        }


    }

    /**
     * A placeholder fragment containing a simple view.
     */
    public static class PlaceholderFragment extends Fragment {
        /**
         * The fragment argument representing the section number for this
         * fragment.
         */
        private static final String ARG_SECTION_NUMBER = "section_number";

        /**
         * Returns a new instance of this fragment for the given section
         * number.
         */
        public static PlaceholderFragment newInstance(int sectionNumber) {
            PlaceholderFragment fragment = new PlaceholderFragment();
            Bundle args = new Bundle();
            args.putInt(ARG_SECTION_NUMBER, sectionNumber);
            fragment.setArguments(args);
            return fragment;
        }

        public PlaceholderFragment() {
        }

        //static long lastTime   =0;
        static int xAxisValue  =0;
        static int yAxisValue  =0;
        static int zAxisValue  =0;
        static int buttonValue =0;

        static public void sendJoypadUpdate(boolean force) {
            /*long thisTime =  System.currentTimeMillis();
            if ( !force && (thisTime - lastTime ) < 100) {
                return;
            }
            lastTime=thisTime;*/
            Log.i(TAG, "joypad(x,y,z)=(" + xAxisValue + "," + yAxisValue  + "," + zAxisValue + ")");

            byte xAxis = (byte) ((xAxisValue+255)/2);
            byte yAxis = (byte) ((yAxisValue+255)/2);
            byte zAxis = (byte) ((zAxisValue+255)/2);
            byte button = (byte) buttonValue;

            if (MainActivity.rfduinoService != null ) {
                MainActivity.rfduinoService.send(new byte[]{xAxis,yAxis,button,zAxis});
            }
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
                                 Bundle savedInstanceState) {

            View rootView = inflater.inflate(R.layout.fragment_main, container, false);
            FrameLayout joystickLayout = (FrameLayout) rootView.findViewById(R.id.joystick);
            FrameLayout throttleLayout = (FrameLayout) rootView.findViewById(R.id.throttle);

            FrameLayout.LayoutParams params =
                    new FrameLayout.LayoutParams( FrameLayout.LayoutParams.MATCH_PARENT, FrameLayout.LayoutParams.MATCH_PARENT );


            JoystickView joystickView = new JoystickView(rootView.getContext());
            ThrottleView throttleView = new ThrottleView(rootView.getContext());
            throttleView.setLayoutParams(params);
            joystickLayout.addView(joystickView);


            throttleLayout.addView(throttleView, params);

            // add callbacks
            joystickView.setOnJostickMovedListener(new JoystickMovedListener() {
                @Override
                public void OnMoved(int pan, int tilt) {
                    xAxisValue=pan;
                    yAxisValue=-tilt;
                }
                @Override
                public void OnReleased() {
                    xAxisValue=0;
                    yAxisValue=0;
                    //sendJoypadUpdate(true);
                }
            });
            throttleView.setOnJostickMovedListener(new JoystickMovedListener() {
                @Override
                public void OnMoved(int pan, int tilt) {
                    zAxisValue=tilt;
                }
                @Override
                public void OnReleased() {
                    zAxisValue=255-75;
                    //sendJoypadUpdate(true);
                }
            });
            return rootView;
        }
    }
}
