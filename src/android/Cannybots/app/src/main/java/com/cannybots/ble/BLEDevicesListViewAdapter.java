package com.cannybots.ble;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import com.cannybots.cannybotsapp.R;

import java.util.Timer;
import java.util.TimerTask;

import static java.lang.Math.random;

/**
 * Created by wayne on 16/12/14.
 */
public class BLEDevicesListViewAdapter extends BaseAdapter {
    public String[] CHEESES = {"Abbaye de Belloc", "Abbaye du Mont des Cats", "Abertam", "Abbaye de Belloc", "Abbaye du Mont des Cats", "Abertam", "Abbaye de Belloc", "Abbaye du Mont des Cats", "Abertam", "Abbaye de Belloc", "Abbaye du Mont des Cats", "Abertam", "Abbaye de Belloc", "Abbaye du Mont des Cats", "Abertam"};

    public BLEDevicesListViewAdapter() {
        Timer timer = new Timer();

        timer.scheduleAtFixedRate(
                new TimerTask() {
                    public void run() {
                        bleUpdate();
                    }
                },
                0,
                1000*2);
    }

    @Override
    public int getCount() {
        return CHEESES.length;
    }

    @Override
    public String getItem(int position) {
        return CHEESES[position];
    }

    @Override
    public long getItemId(int position) {
        return CHEESES[position].hashCode();
    }

    @Override
    public View getView(int position, View convertView, ViewGroup container) {
        if (convertView == null) {
            LayoutInflater inflater = (LayoutInflater)container.getContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);

            convertView = inflater.inflate(R.layout.fragment_connections_list_item, container, false);
        }

        ((TextView) convertView.findViewById(android.R.id.text1))
                .setText(getItem(position));
        return convertView;
    }

    void bleUpdate() {
        if (random()>0.5) {
            CHEESES = new String[]{"A", "B", "C"};
        } else {
            CHEESES = new String[]{"1", "2"};
        }

        // need to run on a GUI thread
        Handler refresh = new Handler(Looper.getMainLooper());
        refresh.post(new Runnable() {
            public void run()
            {
                notifyDataSetChanged();
            }
        });

    }
}
