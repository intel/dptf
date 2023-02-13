/*
* Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License"); you may not
* use this file except in compliance with the License.
*
* You may obtain a copy of the License at
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
* WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
* See the License for the specific language governing permissions and
* limitations under the License.
*/

package com.intel.dptf.jhs;

import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.hardware.display.DisplayManager;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Bundle;
import android.os.PowerManager;
import android.os.RemoteException;
import android.provider.Settings;
import android.telephony.ServiceState;
import android.util.Log;
import android.view.Display;

import com.android.internal.telephony.TelephonyIntents;

import com.intel.dptf.jhs.binder.DptfAndroidEvent;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.Set;

import static android.content.Intent.ACTION_AIRPLANE_MODE_CHANGED;
import static android.content.Intent.ACTION_BATTERY_CHANGED;
import static android.content.Intent.ACTION_CONFIGURATION_CHANGED;
import static android.content.Intent.ACTION_DOCK_EVENT;
import static android.content.Intent.ACTION_POWER_CONNECTED;
import static android.content.Intent.ACTION_POWER_DISCONNECTED;
import static android.content.Intent.ACTION_SCREEN_ON;
import static android.content.Intent.ACTION_SCREEN_OFF;
import static android.content.Intent.ACTION_SHUTDOWN;
import static android.net.ConnectivityManager.CONNECTIVITY_ACTION;
import static android.content.Intent.EXTRA_DOCK_STATE;
import static android.os.BatteryManager.EXTRA_LEVEL;
import static android.os.PowerManager.ACTION_POWER_SAVE_MODE_CHANGED;
import static com.android.internal.telephony.TelephonyIntents.ACTION_SERVICE_STATE_CHANGED;
import static com.intel.dptf.jhs.JhsManager.sService;

/*
 * The JhsEventManager class registers for intents and
 * sends event to DPTF upon receiving these intents
 *
 * @hide
 */
public class JhsEventManager {
    private static boolean sCallStatus = false;
    private static boolean sIsAirplaneModeOn = false;
    private static Context sContext;
    // List all event types
    private static final int EVENT_DISPLAY_ORIENTATION_CHANGED = 26;
    private static final int EVENT_POWER_SOURCE_CHANGED = 37;
    private static final int EVENT_BATTERY_CHANGED = 46;
    private static final int EVENT_DOCK_MODE_CHANGED = 48;
    private static final int ESIF_EVENT_OS_MOBILE_NOTIFICATION = 73;
    // List all sub event types
    private static final int EVENT_AIRPLANE_MODE_CHANGED = 1;
    private static final int EVENT_SERVICE_STATE_CHANGED = 2;
    private static final int EVENT_REQUEST_SHUTDOWN = 3;
    private static final int EVENT_CONNECTIVITY_CHANGED = 4;
    private static final int EVENT_SCREEN_CHANGED = 6;
    private static final int EVENT_POWER_SAVE_MODE_CHANGED = 7;
    private static final String TAG = "JHS:JhsEventManager";
    private static Hashtable<String, String> sEventNameToOnReceiveFunctionMap = new Hashtable<String, String>();
    private static int sBatteryLevelLast = -1;
    private static int sConnectivityStatusLast = -1;
    private static int sScreenOrientationLast = -1;
    private static IntentBroadcastReceiver sIntentReceiver = null;
    private static PowerManager sPowerManager;

    public static void init(Context con) {
        sContext = con;
        eventNameToOnReceiveFunctionMap();
        setAirplaneModeStatus();
        checkInitSettingsOnBoot();
        registerIntents();
    }

    private static void eventNameToOnReceiveFunctionMap() {
        // For any new intent which has to be registered, write onReceive function
        // and map that function name to the event id
        sEventNameToOnReceiveFunctionMap.put(ACTION_BATTERY_CHANGED, "batteryChanged");
        sEventNameToOnReceiveFunctionMap.put(ACTION_AIRPLANE_MODE_CHANGED, "airplaneModeChanged");
        sEventNameToOnReceiveFunctionMap.put(ACTION_SERVICE_STATE_CHANGED, "serviceStateChanged");
        sEventNameToOnReceiveFunctionMap.put(ACTION_SHUTDOWN, "requestShutdown");
        sEventNameToOnReceiveFunctionMap.put(CONNECTIVITY_ACTION, "connectivityChanged");
        sEventNameToOnReceiveFunctionMap.put(ACTION_POWER_CONNECTED, "powerConnected");
        sEventNameToOnReceiveFunctionMap.put(ACTION_POWER_DISCONNECTED, "powerDisconnected");
        sEventNameToOnReceiveFunctionMap.put(ACTION_SCREEN_ON, "screenChangedOn");
        sEventNameToOnReceiveFunctionMap.put(ACTION_SCREEN_OFF, "screenChangedOff");
        sEventNameToOnReceiveFunctionMap.put(ACTION_CONFIGURATION_CHANGED, "displayOrientationChanged");
        sEventNameToOnReceiveFunctionMap.put(ACTION_DOCK_EVENT, "dockModeChanged");
        sEventNameToOnReceiveFunctionMap.put(ACTION_POWER_SAVE_MODE_CHANGED, "powerSaveModeChanged");
    }

    public static void checkInitSettingsOnBoot() {
        checkScreenOnOff();
        checkDisplayOrientation();
        checkPowerSaveMode();
        checkWifiMobileDataConnectivity();
    }

    public static void sendSpecificEventToDptf(int eventType, int participantHandle, int eventParam) {
        int subEventType = 0;
        sendEventToDptf(eventType, participantHandle, subEventType, eventParam);
    }

    public static void sendGenericEventToDptf(int participantHandle, int subEventType, int eventParam) {
        int eventType = ESIF_EVENT_OS_MOBILE_NOTIFICATION;
        sendEventToDptf(eventType, participantHandle, subEventType, eventParam);
    }

    public static void sendEventToDptf(int eventType, int participantHandle, int subEventType, int eventParam) {
        DptfAndroidEvent eventObj = new DptfAndroidEvent(eventType, participantHandle, subEventType, eventParam);
        Log.d(TAG, "eventType: " + eventType + " participantHandle: " + participantHandle + " subEventType: " + subEventType + " eventParam: " + eventParam);
        eventObj.setEventType(eventType);
        eventObj.setParticipant(participantHandle);
        eventObj.setSubEventType(subEventType);
        eventObj.setEventParam(eventParam);
        if (sService != null) {
            try {
                sService.sendEvent(eventObj);
            } catch (RemoteException e) {
                Log.e(TAG, "RemoteException");
            }
        }
        else {
            Log.e(TAG, "Not able to get JHS Client Service");
        }
    }

    private static void registerIntents() {
        int enableFlag = -1, participantHandle = 0;
        Log.d(TAG, "Register for all intents DPTF is interested in");

        if (sIntentReceiver == null) {
            sIntentReceiver = new IntentBroadcastReceiver();
            IntentFilter intentFilter = new IntentFilter();
            Set<String> keys = sEventNameToOnReceiveFunctionMap.keySet();
            for(String key: keys){
                intentFilter.addAction(key);
            }
            sContext.registerReceiver(sIntentReceiver, intentFilter);
        }
    }

    private static void unregisterIntents() {
        Log.d(TAG, "unregisterIntents");
        if (sIntentReceiver != null) {
            sContext.unregisterReceiver(sIntentReceiver);
            sIntentReceiver = null;
        }
    }

    public static void batteryChanged(Intent intent) {
        int batteryLevel = -1, participantHandle = 0;
        batteryLevel = intent.getIntExtra(EXTRA_LEVEL, -1);
        if (batteryLevel != sBatteryLevelLast) {
            sendSpecificEventToDptf(EVENT_BATTERY_CHANGED, participantHandle, batteryLevel);
            sBatteryLevelLast = batteryLevel;
        }
    }

    private static void setAirplaneModeStatus() {
        ContentResolver cr = sContext.getContentResolver();
        if ( Settings.System.getInt(cr, Settings.System.AIRPLANE_MODE_ON, 0) != 0 ) {
            sIsAirplaneModeOn = true;
        }
        Log.d(TAG, "sIsAirplaneModeOn: " + sIsAirplaneModeOn);
    }

    public static boolean getAirplaneModeStatus() {
        return sIsAirplaneModeOn;
    }

    public static void airplaneModeChanged(Intent intent) {
        int enableFlag = -1, participantHandle = 0;
        sIsAirplaneModeOn = intent.getBooleanExtra("state", false);
        enableFlag = (sIsAirplaneModeOn)? 1:0;
        sendGenericEventToDptf(participantHandle, EVENT_AIRPLANE_MODE_CHANGED, enableFlag);
        try {
            Class cls = Class.forName("com.intel.dptf.jhs.JhsModem");
            Object obj = cls.newInstance();
            if (enableFlag == 1) {
                Method method = cls.getDeclaredMethod("participantInactive");
                method.invoke(obj);
            }
            else {
                Method method = cls.getDeclaredMethod("isParticipantActive");
                method.invoke(obj);
            }
        } catch (ClassNotFoundException e) {
            Log.e(TAG, "ClassNotFoundException");
        } catch (InstantiationException e) {
            Log.e(TAG, "InstantiationException");
        } catch (IllegalAccessException e) {
            Log.e(TAG, "IllegalAccessException");
        } catch (NoSuchMethodException e) {
            Log.e(TAG, "NoSuchMethodException");
        } catch (InvocationTargetException e) {
            Log.e(TAG, "InvocationTargetException");
        }
    }

    public static void serviceStateChanged(Intent intent) {
        int serviceStateInt = -1, participantHandle = 0;
        Bundle extras = intent.getExtras();
        if (extras == null) {
            return;
        }
        ServiceState ss = ServiceState.newFromBundle(extras);
        if (ss == null) return;
        serviceStateInt = ss.getState();
        sendGenericEventToDptf(participantHandle, EVENT_SERVICE_STATE_CHANGED, serviceStateInt);
    }

    public static void requestShutdown(Intent intent) {
        int shutdownFlag = -1, participantHandle = 0;
        sendGenericEventToDptf(participantHandle, EVENT_REQUEST_SHUTDOWN, shutdownFlag);
    }

    public static void connectivityChanged(Intent intent) {
        int participantHandle = 0, connectivityStatus = -1;
        int mobileFlag = 0, wifiFlag = 0;
        ConnectivityManager connMan = (ConnectivityManager)sContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        android.net.NetworkInfo mobile = connMan.getNetworkInfo(ConnectivityManager.TYPE_MOBILE);
        android.net.NetworkInfo wifi = connMan.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
        if (mobile != null && mobile.isAvailable() && mobile.isConnected()) {
            Log.d(TAG, "Found Mobile Internet Network");
            connectivityStatus = 1;
            mobileFlag = 1;
        } else mobileFlag = 0;

        if (wifi != null && wifi.isAvailable() && wifi.isConnected()) {
            Log.d(TAG, "Found WI-FI Network");
            connectivityStatus = 2;
            wifiFlag = 1;
        } else wifiFlag = 0;

        if (mobileFlag == 1 && wifiFlag == 1) {
            connectivityStatus = 3;
        }
        if (mobileFlag == 0 && wifiFlag == 0) {
            connectivityStatus = 0;
        }

        if (sConnectivityStatusLast != connectivityStatus) {
            sendGenericEventToDptf(participantHandle, EVENT_CONNECTIVITY_CHANGED, connectivityStatus);
            sConnectivityStatusLast = connectivityStatus;
        }
    }

    public static void powerConnected(Intent intent) {
        int participantHandle = 0, chargingStatus = 0; //0 for cable connected
        sendSpecificEventToDptf(participantHandle, EVENT_POWER_SOURCE_CHANGED, chargingStatus);
    }

    public static void powerDisconnected(Intent intent) {
        int participantHandle = 0, chargingStatus = 1; //1 for running on battery
        sendSpecificEventToDptf(participantHandle, EVENT_POWER_SOURCE_CHANGED, chargingStatus);
    }

    public static void screenChangedOn(Intent intent) {
        int participantHandle = 0, screenStatus = 1;
        sendGenericEventToDptf(participantHandle, EVENT_SCREEN_CHANGED, screenStatus);
    }

    public static void screenChangedOff(Intent intent) {
        int participantHandle = 0, screenStatus = 0;
        sendGenericEventToDptf(participantHandle, EVENT_SCREEN_CHANGED, screenStatus);
    }

    public static void displayOrientationChanged(Intent intent) {
        int participantHandle = 0, screenOrientation = -1, screenOrientationDptf = -1;
        screenOrientation = sContext.getResources().getConfiguration().orientation;
        if (sScreenOrientationLast != screenOrientation) {
            // In Android, 1:Portrait, 2:Landscape
            // In DPTF, 0:Landscape, 1:Portrait
            if (screenOrientation == 1) screenOrientationDptf = 1;
            else if (screenOrientation == 2) screenOrientationDptf = 0;
            sendSpecificEventToDptf(participantHandle, EVENT_DISPLAY_ORIENTATION_CHANGED, screenOrientationDptf);
            sScreenOrientationLast = screenOrientation;
        }
    }

    public static void dockModeChanged(Intent intent) {
        int participantHandle = 0, dockStatus = -1, dockStatusDptf = 0;
        dockStatus = intent.getIntExtra(EXTRA_DOCK_STATE, -1);
        // In DPTF, 0:Invalid dock mode, 1:Undocked, 2:Docked
        if (dockStatus == 0) dockStatusDptf = 1;
        else if (dockStatus > 0 && dockStatus < 5) dockStatusDptf = 2;
        sendSpecificEventToDptf(participantHandle, EVENT_DOCK_MODE_CHANGED, dockStatusDptf);
    }

    private static void checkPowerSaveMode() {
        int participantHandle = 0, powerSaveMode = -1;
        sPowerManager = (PowerManager) sContext.getSystemService(Context.POWER_SERVICE);
        if (sPowerManager.isPowerSaveMode()) {
            powerSaveMode = 1;
        }
        else {
            powerSaveMode = 0;
        }
        Log.d(TAG, "powerSaveMode: " + powerSaveMode);
        sendGenericEventToDptf(participantHandle, EVENT_POWER_SAVE_MODE_CHANGED, powerSaveMode);
    }

    public static void powerSaveModeChanged(Intent intent) {
        int participantHandle = 0, powerSaveMode = -1;
        if (sPowerManager.isPowerSaveMode()) {
            Log.d(TAG, "Power Save Mode enabled");
            powerSaveMode = 1;
        } else {
            Log.d(TAG, "Power Save Mode disabled");
            powerSaveMode = 0;
        }
        sendGenericEventToDptf(participantHandle, EVENT_POWER_SAVE_MODE_CHANGED, powerSaveMode);
    }

    private static void checkScreenOnOff() {
        int participantHandle = 0, displayStatus = -1;
        DisplayManager dm = (DisplayManager) sContext.getSystemService(Context.DISPLAY_SERVICE);
        // status of the first display is sent
        for (Display display : dm.getDisplays()) {
            if (display.getState() == Display.STATE_ON) {
                displayStatus = 1;
            }
            else {
                displayStatus = 0;
            }
            Log.d(TAG, "displayStatus: " + displayStatus);
            sendGenericEventToDptf(participantHandle, EVENT_SCREEN_CHANGED, displayStatus);
            return;
        }
    }

    private static void checkDisplayOrientation() {
        int participantHandle = 0, screenOrientation = -1, screenOrientationDptf = -1;
        screenOrientation = sContext.getResources().getConfiguration().orientation;
        // In Android, 1:Portrait, 2:Landscape
        // In DPTF, 0:Landscape, 1:Portrait
        if (screenOrientation == 1) screenOrientationDptf = 1;
        else if (screenOrientation == 2) screenOrientationDptf = 0;
        Log.d(TAG, "screenOrientationDptf: " + screenOrientationDptf);
        sendSpecificEventToDptf(participantHandle, EVENT_DISPLAY_ORIENTATION_CHANGED, screenOrientationDptf);
        sScreenOrientationLast = screenOrientation;
    }

    private static void checkWifiMobileDataConnectivity() {
        int participantHandle = 0, connectivityStatus = -1;
        int mobileFlag = 0, wifiFlag = 0;
        ConnectivityManager connMan = (ConnectivityManager)sContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        android.net.NetworkInfo mobile = connMan.getNetworkInfo(ConnectivityManager.TYPE_MOBILE);
        android.net.NetworkInfo wifi = connMan.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
        if (mobile != null && mobile.isAvailable() && mobile.isConnected()) {
            Log.d(TAG, "Found Mobile Internet Network");
            connectivityStatus = 1;
            mobileFlag = 1;
        } else mobileFlag = 0;

        if (wifi != null && wifi.isAvailable() && wifi.isConnected()) {
            Log.d(TAG, "Found WI-FI Network");
            connectivityStatus = 2;
            wifiFlag = 1;
        } else wifiFlag = 0;

        if (mobileFlag == 1 && wifiFlag == 1) {
            connectivityStatus = 3;
        }
        if (mobileFlag == 0 && wifiFlag == 0) {
            connectivityStatus = 0;
        }
        Log.d(TAG, "connectivityStatus: " + connectivityStatus);
        sendGenericEventToDptf(participantHandle, EVENT_CONNECTIVITY_CHANGED, connectivityStatus);
        sConnectivityStatusLast = connectivityStatus;
    }

    private static final class IntentBroadcastReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "got intent with action: " + action);
            String eventSendingFunctionName = sEventNameToOnReceiveFunctionMap.get(action);
            Log.d(TAG, "Function which is going to send event to DPTF is: " + eventSendingFunctionName );

            try {
                JhsEventManager obj = new JhsEventManager();
                Method method = JhsEventManager.class.getDeclaredMethod(eventSendingFunctionName, Intent.class);
                method.invoke(obj, intent);
            } catch (IllegalAccessException e) {
                Log.e(TAG, "IllegalAccessException");
            } catch (NoSuchMethodException e) {
                Log.e(TAG, "NoSuchMethodException");
            } catch (InvocationTargetException e) {
                Log.e(TAG, "InvocationTargetException");
            }

        }
    };

}

