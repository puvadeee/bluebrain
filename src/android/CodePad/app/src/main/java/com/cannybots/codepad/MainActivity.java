package com.cannybots.codepad;




import android.app.AlertDialog;
import android.content.SharedPreferences;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.net.http.SslError;
import android.net.wifi.WifiManager;
import android.support.v4.app.ListFragment;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.pm.ActivityInfo;
import android.os.IBinder;
import android.support.v7.app.ActionBarActivity;
import android.support.v7.app.ActionBar;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentTransaction;
import android.support.v4.app.FragmentPagerAdapter;
import android.os.Bundle;
import android.support.v4.view.ViewPager;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;

import com.cannybots.ble.BLEDevicesListViewAdapter;
import com.cannybots.ble.BluetoothHelper;
import com.cannybots.ble.HexAsciiHelper;
import com.cannybots.ble.RFduinoService;
import com.cannybots.views.joystick.*;
import com.cannybots.views.web.CBWebViewClient;

import android.util.Log;
import android.webkit.SslErrorHandler;
import android.webkit.WebChromeClient;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.CompoundButton;
import android.widget.FrameLayout;
import android.widget.ListView;
import android.widget.SeekBar;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;


import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Enumeration;
import java.util.Timer;
import java.util.TimerTask;

import fi.iki.elonen.NanoHTTPD;
import fi.iki.elonen.SimpleWebServer;
import fi.iki.elonen.samples.echo.DebugWebSocketServer;

public class MainActivity extends ActionBarActivity implements ActionBar.TabListener,BluetoothAdapter.LeScanCallback {

    private static final String TAG = "CannybotsActivity";
    public static boolean confReverseFrontBack;
    public static boolean confReverseLeftRight;

    private boolean scanStarted;
    private boolean scanning;

    private BluetoothAdapter bluetoothAdapter;
    public static BluetoothDevice bluetoothDevice;

    public static RFduinoService rfduinoService;
    private boolean deviceHasBluetoothLE = false;


    // web support
    static JsHandler _jsHandler;

    private static final int PORT = 3141;
    private NanoHTTPD server;

    // BLE callbacks

    private final BroadcastReceiver bluetoothStateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, 0);
            Log.i(TAG, "BluetoothAdapter state = " + state);
        }
    };

    private final BroadcastReceiver scanModeReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.i(TAG, "scanModeReceiver.onReceive");

            scanning = (bluetoothAdapter.getScanMode() != BluetoothAdapter.SCAN_MODE_NONE);
            scanStarted &= scanning;
        }
    };

    private final ServiceConnection rfduinoServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {

            Log.i(TAG, "rfduinoServiceConnection.onServiceConnected");
            if (rfduinoService == null) {
                rfduinoService = ((RFduinoService.LocalBinder) service).getService();
                if (!rfduinoService.initialize()) {
                    Log.e(TAG, "Failed to start RFduino service.");
                }
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.i(TAG, "rfduinoServiceConnection.onServiceDisconnected");

            rfduinoService = null;
        }
    };

    private final BroadcastReceiver rfduinoReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {

            Log.i(TAG, "rfduinoReceiver.onReceive");

            final String action = intent.getAction();
            if (RFduinoService.ACTION_CONNECTED.equals(action)) {
                Joystick_startTimer();

            } else if (RFduinoService.ACTION_DISCONNECTED.equals(action)) {
                Joystick_stopTimer();
                BLE_disconnected();

            } else if (RFduinoService.ACTION_DATA_AVAILABLE.equals(action)) {
                addData(intent.getByteArrayExtra(RFduinoService.EXTRA_DATA));
            }
        }
    };


    @Override
    public void onLeScan(BluetoothDevice device, final int rssi, final byte[] scanRecord) {

        byte advData[] = BluetoothHelper.parseAdvertisementFromScanRecord(scanRecord);
        Log.i(TAG, "Potential BLE Device found name = " + device.getName());
        if ( (advData[0] == 'C') && (advData[1] == 'B') && (advData[2] == '0') && (advData[3] == '1') ) {
           BLEDevicesListViewAdapter.addDevice(device);
           Log.i(TAG, "Found Cannybot! BLE Device Info = " + device.getAddress());//BluetoothHelper.getDeviceInfoText(device, rssi, scanRecord));
        }
    }

    private void BLE_onCreate() {
        Log.i(TAG, "BLE_onCreate");

        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (bluetoothAdapter != null) {
            deviceHasBluetoothLE = true;
            bluetoothAdapter.enable();
        } else {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setMessage("No Bluetooth LE hardware detected, you won't be able to connect to a Cannybot.")
                    .setCancelable(false)
                    .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int id) {
                            //do things
                        }
                    });
            AlertDialog alert = builder.create();
            alert.show();
            deviceHasBluetoothLE = false;
        }
    }


    private void BLE_onStart() {
        Log.i(TAG, "BLE_onStart");

        if (!deviceHasBluetoothLE)
            return;
        registerReceiver(scanModeReceiver, new IntentFilter(BluetoothAdapter.ACTION_SCAN_MODE_CHANGED));
        registerReceiver(bluetoothStateReceiver, new IntentFilter(BluetoothAdapter.ACTION_STATE_CHANGED));
        registerReceiver(rfduinoReceiver, RFduinoService.getIntentFilter());

        Intent rfduinoIntent = new Intent(MainActivity.this, RFduinoService.class);
        bindService(rfduinoIntent, rfduinoServiceConnection, BIND_AUTO_CREATE);

    }

    private void BLE_onStop() {
        Log.i(TAG, "BLE_onStop");

        if (!deviceHasBluetoothLE)
            return;

        Joystick_stopTimer();
        unregisterReceiver(scanModeReceiver);
        unregisterReceiver(bluetoothStateReceiver);
        unregisterReceiver(rfduinoReceiver);
        unbindService(rfduinoServiceConnection);
    }



    private void BLE_startScanning() {
        Log.i(TAG, "BLE_startScanning");

        if (!deviceHasBluetoothLE)
            return;
        // when scan pressed
        scanStarted = true;

        // find by 16 bit short code
        //bluetoothAdapter.startLeScan(new UUID[]{RFduinoService.UUID_SERVICE}, MainActivity.this);

        // Find all (then filter in 'onLeScan' callback )
        bluetoothAdapter.startLeScan(MainActivity.this);
    }

    public void BLE_stopScanning() {
        Log.i(TAG, "BLE_stopScanning");

        if (!deviceHasBluetoothLE)
            return;
        scanStarted = false;
        bluetoothAdapter.stopLeScan(this);
    }

    Timer joypadUpdateTimer;

    // when 'connect' pressed

    public void BLE_connect(BluetoothDevice device) {
        Log.i(TAG, "BLE_connect");

        if (!deviceHasBluetoothLE)
            return;

        if (device != null) {
            bluetoothDevice = device;
            if (rfduinoService.connect(bluetoothDevice.getAddress())) {
                Log.i(TAG, "Connected to BLE device!");
            } else {
                Log.e(TAG, "Connect to BLE device failed!");
                Toast.makeText(getApplicationContext(),
                        "Failed to connect to '"
                                + device.getName()
                                +"'"
                                + " [" + device.getAddress() + "]",
                        Toast.LENGTH_LONG).show();
            }
        }
    }

    private void addData(byte[] data) {
        Log.i(TAG, "RECV: " + HexAsciiHelper.bytesToHex(data));
    }

    private static void sendBytes(byte[] bytes) {
        if ( (MainActivity.rfduinoService != null) && (MainActivity.bluetoothDevice!=null)) {
            //Log.d(TAG, "sendBytes: " + bytes);

            MainActivity.rfduinoService.send(bytes);
        }
    }



    private void BLE_disconnect() {
        Log.i(TAG, "BLE_disconnect");

        if (rfduinoService != null) {
            rfduinoService.disconnect();
            rfduinoService.close();
        }
    }

    private void BLE_disconnected() {
        Log.i(TAG, "BLE_disconnected");

        if (!deviceHasBluetoothLE)
            return;

        bluetoothDevice = null;
    }



    public void Joystick_startTimer() {
        Log.i(TAG, "Joystick_startTimer");

        joypadUpdateTimer = new Timer();
        joypadUpdateTimer.scheduleAtFixedRate(
                new TimerTask() {
                    public void run() {
                        JoypadFragment.sendJoypadUpdate(false);
                    }
                },
                0,
                75);
    }

    public void Joystick_stopTimer() {
        Log.i(TAG, "Joystick_stopTimer");

        if (joypadUpdateTimer!=null) {
            joypadUpdateTimer.cancel();
            joypadUpdateTimer.purge();

        }
    }



    @Override
    protected void onStart() {
         Log.i(TAG, "******  onStart *******");

        super.onStart();
        BLE_onStart();

        copyFileOrDir("www");

        WifiManager wifiManager = (WifiManager) getSystemService(WIFI_SERVICE);
        int ipAddress = wifiManager.getConnectionInfo().getIpAddress();
        final String formatedIpAddress = String.format("%d.%d.%d.%d", (ipAddress & 0xff), (ipAddress >> 8 & 0xff),
                (ipAddress >> 16 & 0xff), (ipAddress >> 24 & 0xff));
        Log.d(TAG, "Please access! http://" + formatedIpAddress + ":" + PORT);

        try {
            server = new SimpleWebServer(formatedIpAddress, PORT, new File("file:///data/data/com.cannybots.codepad/www/"), false );
            server.start();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    protected void onStop() {
        Log.i(TAG, "******  onStop *******");
        super.onStop();
        BLE_onStop();
        server.stop();
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
        this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

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

        loadSettings();

        BLE_onCreate();
    }

    public static final String PREFS_NAME = "CannybotsPrefsV1";

    public void loadSettings() {
        // Restore preferences
        SharedPreferences settings = getSharedPreferences(PREFS_NAME, 0);
        confReverseFrontBack = settings.getBoolean("reverseFrontBack", false);
        confReverseLeftRight = settings.getBoolean("reverseLeftRight", false);

    }

    public void saveSettings() {
        SharedPreferences settings = getSharedPreferences(PREFS_NAME, 0);
        SharedPreferences.Editor editor = settings.edit();
        editor.putBoolean("reverseFrontBack", confReverseFrontBack);
        editor.putBoolean("reverseLeftRight", confReverseLeftRight);

        // Commit the edits!
        editor.commit();
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
        /*if (id == R.id.action_settings) {
            return true;
        }*/

        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onTabSelected(ActionBar.Tab tab, FragmentTransaction fragmentTransaction) {
        // When the given tab is selected, switch to the corresponding page in
        // the ViewPager.
        mViewPager.setCurrentItem(tab.getPosition());
        if (tab.getPosition() == 0 ) {
        }

    }

    @Override
    public void onTabUnselected(ActionBar.Tab tab, FragmentTransaction fragmentTransaction) {
        Log.i(TAG, "onTabUnselected");
        if (tab.getPosition() == 0 ) {
        }

    }

    @Override
    public void onTabReselected(ActionBar.Tab tab, FragmentTransaction fragmentTransaction) {
        Log.i(TAG, "onTabReselected");

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
            // Return a JoypadFragment (defined as a static inner class below).
            switch (position) {
                case 0:
                    return JoypadFragment.newInstance();
                case 1:
                    return SettingsFragment.newInstance();
                case 2:
                    return ConnectionsFragment.newInstance();
            }
            return null;
        }

        @Override
        public int getCount() {
            // Show 3 total pages.
            return 3;
        }

        @Override
        public CharSequence getPageTitle(int position) {
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

    public static class SettingsFragment extends Fragment {

        public static SettingsFragment newInstance() {
            SettingsFragment fragment = new SettingsFragment();

            return fragment;
        }

        public SettingsFragment() {
        }


        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
                                 Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.fragment_settings, container, false);
            final MainActivity activity = (MainActivity) getActivity();


            final Switch frontBackSwitch  = (Switch) rootView.findViewById(R.id.reverseFrontBack);
            final Switch leftRightSwitch  = (Switch) rootView.findViewById(R.id.reverseLeftRight);
            final SeekBar whiteThresholdSlider = (SeekBar) rootView.findViewById(R.id.whiteThresholdSlider);
            final TextView whiteThresholdLabel = (TextView) rootView.findViewById(R.id.whiteThresholdLable);
            frontBackSwitch.setChecked(activity.confReverseFrontBack);
            leftRightSwitch.setChecked(activity.confReverseLeftRight);

            frontBackSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                    MainActivity activity = (MainActivity) getActivity();

                    Log.i(TAG, "reverseFrontBack=" + isChecked);
                    activity.confReverseFrontBack=isChecked;
                    activity.saveSettings();
                }
            });
            leftRightSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                    Log.i(TAG, "reverseLeftRight=" + isChecked);
                    MainActivity activity = (MainActivity) getActivity();
                    activity.confReverseLeftRight=isChecked;
                    activity.saveSettings();

                }
            });

            whiteThresholdSlider.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                @Override
                public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                    Log.i(TAG, "whiteThreshold=" + progress);
                    whiteThresholdLabel.setText(Integer.toString(progress));
                    activity.sendBytes(new byte[]{0x7f, 0x7f, 0x00,0x7f, (byte) (progress/4)});
                }

                @Override
                public void onStartTrackingTouch(SeekBar seekBar) {

                }

                @Override
                public void onStopTrackingTouch(SeekBar seekBar) {

                }
            });


            return rootView;
        }
    }


        /**
     * Joystick fragment view.
     */
    public static class JoypadFragment extends Fragment {

        /**
         * Returns a new instance of this fragment for the given section
         * number.
         */
        public static JoypadFragment newInstance(/*int sectionNumber*/) {
            JoypadFragment fragment = new JoypadFragment();
            //Bundle args = new Bundle();
            //args.putInt(ARG_SECTION_NUMBER, sectionNumber);
            //fragment.setArguments(args);
            return fragment;
        }

        public JoypadFragment() {
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
            //Log.d(TAG, "joypad(x,y,z)=(" + xAxisValue + "," + yAxisValue  + "," + zAxisValue + ")");
            int x = xAxisValue * (MainActivity.confReverseLeftRight==true?-1:1);
            int y = yAxisValue * (MainActivity.confReverseFrontBack==true?-1:1);

            byte xAxis = (byte) ((x+255)/2);
            byte yAxis = (byte) ((y+255)/2);
            byte zAxis = (byte) ((zAxisValue+255)/2);
            byte button = (byte) buttonValue;

            MainActivity.sendBytes(new byte[]{xAxis, yAxis, button, zAxis});

        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
                                 Bundle savedInstanceState) {

            View rootView = inflater.inflate(R.layout.fragment_cbwebview, container, false);
            FrameLayout cbWebViewLayout = (FrameLayout) rootView.findViewById(R.id.cbwebview);

            FrameLayout.LayoutParams params =
                    new FrameLayout.LayoutParams( FrameLayout.LayoutParams.MATCH_PARENT, FrameLayout.LayoutParams.MATCH_PARENT );

            WebView webView = new WebView(rootView.getContext());
            cbWebViewLayout.addView(webView);

            /*
            webView.getSettings().setUseWideViewPort(true);
            webView.getSettings().setLoadWithOverviewMode(true);
            webView.getSettings().setDomStorageEnabled(true);
            webView.getSettings().setBuiltInZoomControls(false);
            webView.getSettings().setPluginState(WebSettings.PluginState.ON);
            webView.getSettings().setAllowFileAccess(true);
            // these settings speed up page load into the webview
            webView.getSettings().setRenderPriority(WebSettings.RenderPriority.HIGH);
            webView.getSettings().setCacheMode(WebSettings.LOAD_NO_CACHE);
            webView.requestFocus(View.FOCUS_DOWN);
            */
            //webView.setWebViewClient(new CBWebViewClient());

            webView.getSettings().setJavaScriptEnabled(true);


            //webView.loadUrl("http://www.google.com/");
            webView.setWebChromeClient(new WebChromeClient());
            webView.setWebViewClient(new WebViewClient() {

                @Override
                public void onPageStarted(WebView view, String url, Bitmap favicon) {
                    // TODO Auto-generated method stub
                    super.onPageStarted(view, url, favicon);
                    //Toast.makeText(TableContentsWithDisplay.this, "url "+url, Toast.LENGTH_SHORT).show();

                }
                @Override
                public void onPageFinished(WebView view, String url) {
                    super.onPageFinished(view, url);
                    //Toast.makeText(TableContentsWithDisplay.this, "Width " + view.getWidth() +" *** " + "Height " + view.getHeight(), Toast.LENGTH_SHORT).show();
                    _jsHandler.javaFnCall("Hey, Im calling from Android-Java");

                }

                @Override
                public void onReceivedSslError(WebView view,
                                               SslErrorHandler handler, SslError error) {
                    // TODO Auto-generated method stub
                    super.onReceivedSslError(view, handler, error);
                    //Toast.makeText(TableContentsWithDisplay.this, "error "+error, Toast.LENGTH_SHORT).show();

                }

                public boolean shouldOverrideUrlLoading(WebView v, String url)
                {
                    v.loadUrl(url);
                    return true;
                }
            });

            _jsHandler = new JsHandler(this.getActivity(), webView);
            webView.addJavascriptInterface(_jsHandler, "JsHandler");

            // load the main.html file that kept in assets folder
            webView.loadUrl("http://192.168.0.55/~wayne/main.html");//file:///android_asset/main.html");


            return rootView;
        }
    }


    // see: http://www.vogella.com/tutorials/AndroidListView/article.html
    public static class ConnectionsFragment extends ListFragment {

        private static BLEDevicesListViewAdapter adapter = new BLEDevicesListViewAdapter();
        public static ConnectionsFragment newInstance() {
            ConnectionsFragment fragment = new ConnectionsFragment();
            return fragment;
        }

        public ConnectionsFragment() {
        }

        @Override
        public void onActivityCreated(Bundle savedInstanceState) {
            super.onActivityCreated(savedInstanceState);
            setListAdapter(adapter);
        }

        @Override
        public void onListItemClick(ListView l, View v, int position, long id) {
            /*Toast.makeText(l.getContext(),
                    "Click ListItem Number " + position, Toast.LENGTH_LONG)
                    .show();        }*/
            MainActivity activity = (MainActivity) getActivity();
            activity.BLE_stopScanning();
            BluetoothDevice device = (BluetoothDevice) getListAdapter().getItem(position);
            Log.i(TAG, "onListItemClick for BLE device:" + device);
            activity.BLE_connect(device);
            activity.getSupportActionBar().setSelectedNavigationItem(0);
        }

        @Override
        public void setUserVisibleHint(boolean isVisibleToUser) {
            super.setUserVisibleHint(isVisibleToUser);
            MainActivity activity = (MainActivity) getActivity();
            if (isVisibleToUser) {
                Log.i(TAG, "connections visible");
                activity.Joystick_stopTimer();
                activity.BLE_disconnect();
                activity.BLE_startScanning();
            }
            else {
                Log.i(TAG, "connections NOT visible");
                if (activity != null) {
                    activity.BLE_stopScanning();
                    adapter.clear();
                } else {
                    Log.w(TAG, "!!!activity null");
                }

            }
        }
    }

    private void copyFileOrDir(String path) {
        AssetManager assetManager = this.getAssets();
        String assets[] = null;
        try {
            assets = assetManager.list(path);
            if (assets.length == 0) {
                copyFile(path);
            } else {
                String fullPath = "/data/data/" + this.getPackageName() + "/" + path;
                Log.d(TAG,fullPath);
                File dir = new File(fullPath);
                if (!dir.exists())
                    dir.mkdir();
                for (int i = 0; i < assets.length; ++i) {
                    copyFileOrDir(path + "/" + assets[i]);
                }
            }
        } catch (IOException ex) {
            Log.e(TAG, "I/O Exception", ex);
        }
    }
    private void copyFile(String filename) {
        AssetManager assetManager = this.getAssets();
        InputStream in = null;
        OutputStream out = null;
        try {
            in = assetManager.open(filename);
            String newFileName = "/data/data/" + this.getPackageName() + "/" + filename;
            out = new FileOutputStream(newFileName);
            byte[] buffer = new byte[1024];
            int read;
            while ((read = in.read(buffer)) != -1) {
                out.write(buffer, 0, read);
            }
            in.close();
            in = null;
            out.flush();
            out.close();
            out = null;
        } catch (Exception e) {
            Log.e(TAG, e.getMessage());
        }
    }
/*
    public String getLocalIpAddress() {
        try {
            for (Enumeration en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements();) {
                NetworkInterface intf = en.nextElement();
                for (Enumeration enumIpAddr = intf.getInetAddresses(); enumIpAddr.hasMoreElements();) {
                    InetAddress inetAddress = enumIpAddr.nextElement();
                    if (!inetAddress.isLoopbackAddress()) {
                        return inetAddress.getHostAddress().toString();
                    }
                }
            }
        } catch (SocketException ex) {
            Log.e(TAG, ex.toString());
        }
        return null;
    }
    */
}
