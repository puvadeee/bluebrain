package com.cannybots.ble;

import android.bluetooth.BluetoothDevice;
import android.util.Log;

import java.util.UUID;

// see: http://evothings.com/forum/viewtopic.php?f=8&t=102

// Android bug:  https://code.google.com/p/android/issues/detail?id=59490
// wont parse 128bit UUID's,  but 16bit ones are reserved!

public class BluetoothHelper {
    public static String shortUuidFormat   = "0000%04X-0000-1000-8000-00805F9B34FB";
    public static String longUuidFormat    = "7e40%04X-b5a3-f393-e0a9-e50e24dcca9e";

    public static UUID longUuid(long longUuid) {
        UUID uuid = UUID.fromString(String.format(longUuidFormat, longUuid));

        //Log.i("BluetoothHelper", "UUID = " + uuid.toString());

        return uuid;

    }

    public static UUID sixteenBitUuid(long shortUuid) {
        assert shortUuid >= 0 && shortUuid <= 0xFFFF;
        return UUID.fromString(String.format(shortUuidFormat, shortUuid & 0xFFFF));
    }

    public static String getDeviceInfoText(BluetoothDevice device, int rssi, byte[] scanRecord) {
        return new StringBuilder()
                .append("Name: ").append(device.getName())
                .append("\nMAC: ").append(device.getAddress())
                .append("\nRSSI: ").append(rssi)
                .append("\nScan Record:").append(parseScanRecord(scanRecord))
                .toString();
    }

    // Bluetooth Spec V4.0 - Vol 3, Part C, section 8
    public static String parseScanRecord(byte[] scanRecord) {
        StringBuilder output = new StringBuilder();
        int i = 0;
        while (i < scanRecord.length) {
            int len = scanRecord[i++] & 0xFF;
            if (len == 0) break;
            switch (scanRecord[i] & 0xFF) {
                // https://www.bluetooth.org/en-us/specification/assigned-numbers/generic-access-profile
                case 0x0A: // Tx Power
                    output.append("\n  Tx Power: ").append(scanRecord[i+1]);
                    break;
                case 0xFF: // Manufacturer Specific data (RFduinoBLE.advertisementData)
                    output.append("\n  Advertisement Data: ")
                            .append(HexAsciiHelper.bytesToHex(scanRecord, i + 3, len));

                    String ascii = HexAsciiHelper.bytesToAsciiMaybe(scanRecord, i + 3, len);
                    if (ascii != null) {
                        output.append(" (\"").append(ascii).append("\")");
                    }
                    break;
            }
            i += len;
        }
        return output.toString();
    }

    public static byte[] parseAdvertisementFromScanRecord(byte[] scanRecord) {
        StringBuilder output = new StringBuilder();
        int i = 0;
        while (i < scanRecord.length) {
            int len = scanRecord[i++] & 0xFF;
            if (len == 0) break;
            switch (scanRecord[i] & 0xFF) {
                // https://www.bluetooth.org/en-us/specification/assigned-numbers/generic-access-profile
                case 0xFF: // Manufacturer Specific data (RFduinoBLE.advertisementData)
                    byte advData[] = new byte[len];
                    System.arraycopy(scanRecord, i + 3, advData, 0, len);
                    return advData;

            }
            i += len;
        }
        return new byte[0];
    }
}
