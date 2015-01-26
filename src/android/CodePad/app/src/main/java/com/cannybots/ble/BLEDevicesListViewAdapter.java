package com.cannybots.ble;

import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import com.cannybots.codepad.R;

import java.util.ArrayList;
import java.util.Timer;
import java.util.TimerTask;

/**
 * Created by wayne on 16/12/14.
 */
public class BLEDevicesListViewAdapter extends BaseAdapter {

    private static ArrayList devices = new ArrayList(10);

    public BLEDevicesListViewAdapter() {
        Timer timer = new Timer();
        timer.scheduleAtFixedRate(
                new TimerTask() {
                    public void run() {
                        bleUpdate();
                    }
                },
                0,
                1000*1);
    }

    @Override
    public int getCount() {
        return devices.size();
    }

    @Override
    public BluetoothDevice getItem(int position) {
        return (BluetoothDevice) devices.get(position);
    }

    @Override
    public long getItemId(int position) {
        return devices.get(position).hashCode();
    }

    @Override
    public View getView(int position, View convertView, ViewGroup container) {
        if (convertView == null) {
            LayoutInflater inflater = (LayoutInflater)container.getContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);

            convertView = inflater.inflate(R.layout.fragment_connections_list_item, container, false);
        }

        BluetoothDevice device = ((BluetoothDevice)devices.get(position));
        String name = device.getName();
        if (name == null)
            name = "[no name] - " + device.getAddress();

        ((TextView) convertView.findViewById(android.R.id.text1)).setText(name);
        return convertView;
    }

    void bleUpdate() {
        // need to run on a GUI thread
        Handler refresh = new Handler(Looper.getMainLooper());
        refresh.post(new Runnable() {
            public void run()
            {
                notifyDataSetChanged();
            }
        });

    }

    public void clear() {
        devices.clear();
        bleUpdate();
    }

    synchronized public static void addDevice(final BluetoothDevice device) {
        if (device == null)
            return;

        Log.i("ble", "addDevice" +device.getName() + " -> " + device);

        // run on the gui thread
        Handler refresh = new Handler(Looper.getMainLooper());
        refresh.post(new Runnable() {
            public void run()
            {
                int index = 0 ;
                while (devices.size()> index) {
                    if( ( (BluetoothDevice)devices.get(index)).getAddress().equals(device.getAddress())) {
                        devices.set(index, device);
                        return;
                    }
                    index++ ;
                }

                devices.add(device);

            }
        });
    }
}
