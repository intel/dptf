/*
* Copyright (c) 2015 Intel Corporation All Rights Reserved
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
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.ServiceManager;
import android.os.RemoteException;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.intel.dptf.jhs.binder.ActionParameters;
import com.intel.dptf.jhs.binder.JhsReplyHeader;
import com.intel.dptf.jhs.binder.JhsReplyPayload;
import com.intel.internal.telephony.OemTelephony.IOemTelephony;
import com.intel.internal.telephony.OemTelephony.OemTelephonyConstants;
import com.intel.internal.telephony.OemTelephony.OemTelephonyIntents;

import java.lang.Math;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.Set;

import static com.intel.dptf.jhs.JhsManager.DATA_TYPE_TEMP;
import static com.intel.dptf.jhs.JhsManager.SIZE_OF_INTEGER;
import static com.intel.dptf.jhs.JhsManager.SUCCESSFUL;
import static com.intel.dptf.jhs.JhsManager.UNSUCCESSFUL;

import static com.intel.internal.telephony.OemTelephony.OemTelephonyIntents.ACTION_EMERGENCY_CALL_STATUS_CHANGED;
import static com.intel.internal.telephony.OemTelephony.OemTelephonyIntents.ACTION_MODEM_SENSOR_THRESHOLD_REACHED_V2;
import static com.intel.internal.telephony.OemTelephony.OemTelephonyIntents.OEM_TELEPHONY_READY;
import static com.intel.internal.telephony.OemTelephony.OemTelephonyIntents.MODEM_SENSOR_ID_KEY;
import static com.intel.internal.telephony.OemTelephony.OemTelephonyIntents.MODEM_SENSOR_TEMPERATURE_KEY;

/*
 *JhsModem supports primitives related to modem participant
 *
 * @hide
 */

public class JhsModem {
    private static boolean sCallStatus = false;
    private static boolean sHashTableMapped = false;
    private static boolean sModemActive = false;
    private static boolean sModemParticipantActivePending = false;
    private static boolean sModemIntentRegistrationDone = false;
    private static boolean sModemReady = false;
    private static boolean sOnGoingEmergencyCall = false;
    private static Context sContext = JhsManager.getContext();
    // List all intel modem specific event types
    private static final int EVENT_MODEM_SENSOR_THRESHOLD_REACHED_V2 = 16;
    // List all intel modem specific sub event types
    private static final int EVENT_EMERGENCY_CALL_STATUS_CHANGED = 0;
    private static final int DEBOUNCE = 2000;
    private static final int ENABLE = 1;
    private static final int DISABLE = 0;
    // setting primitive ID as per esif_sdk_primitive_type.h file
    private static final int GET_TEMPERATURE = 1347245151; // decimal value for: 0x504d545f;
    private static final int SET_PERF_PRESENT_CAPABILITY = 1129336915; // decimal value for: 0x43505053;
    private static final int SET_TEMPERATURE_THRESHOLDS_0 = 810828112; // decimal value for: 0x30544150;
    private static final int SET_TEMPERATURE_THRESHOLDS_1 = 827605328; // decimal value for: 0x31544150;
    private static final int sDeviceType = 15;
    private static final String sClassName = "JhsModem";
    private static final String sParticipantName = "INT3408";
    private static final String TAG = "JHS:JhsModem";
    private static final Object sEmergencyCallLock = new Object();
    private static Hashtable<String, String> sModemEventNameToOnReceiveFunctionMap = new Hashtable<String, String>();
    private static IOemTelephony mPhoneService = null;
    private static ModemIntentBroadcastReceiver sModemIntentReceiver = null;

    JhsModem() {
        if (sHashTableMapped == false)
            modemEventNameToOnReceiveFunctionMap();
        if (sModemIntentRegistrationDone == false)
            registerModemIntentReceiver();
    }

    private static void modemEventNameToOnReceiveFunctionMap() {
        // For any new intent which has to be registered, write onReceive function
        // and map that function name to the event id
        sModemEventNameToOnReceiveFunctionMap.put(OEM_TELEPHONY_READY, "modemReadyIntentReceived");
        sModemEventNameToOnReceiveFunctionMap.put(ACTION_MODEM_SENSOR_THRESHOLD_REACHED_V2, "sensorThresholdReachedV2");
        sModemEventNameToOnReceiveFunctionMap.put(ACTION_EMERGENCY_CALL_STATUS_CHANGED, "emergencyCallStatusChanged");
        sHashTableMapped = true;
    }

    public static void isParticipantActive() {
        boolean isAirplaneModeOn = JhsEventManager.getAirplaneModeStatus();
        if (sModemReady == false || isAirplaneModeOn == true) {
            Log.d(TAG, "Modem is not Active. Modem participant active pending");
            sModemParticipantActivePending = true;
        }
        else {
            sModemParticipantActivePending = false;
            JhsManager.sendParticipantActiveToDptf(sParticipantName, sDeviceType, sClassName);
        }
    }

    public static void participantInactive() {
        sModemParticipantActivePending = true;
        JhsManager.sendParticipantInactiveToDptf(sParticipantName);
    }

    public static boolean getCallStatus() {
        return sCallStatus;
    }

    public int getValue(ActionParameters actionParams, JhsReplyHeader jhsReplyHead, JhsReplyPayload returnByteArray) {
        int actionId, ret = UNSUCCESSFUL;

        Log.d(TAG, "getValue");
        actionId = actionParams.getActionType();
        switch (actionId) {
            case GET_TEMPERATURE: ret = getSensorMaxTemp(jhsReplyHead, returnByteArray);
                 break;
            default: Log.e(TAG, "Invalid primitive id");
                ret = UNSUCCESSFUL;
        }
        return ret;
    }

    public int setValue(ActionParameters actionParams, int value) {
        int actionId, ret = UNSUCCESSFUL, alarmID = 0, instanceID = -1;

        Log.d(TAG, "setValue");
        actionId = actionParams.getActionType();
        switch (actionId) {
                // Number of trip points supported by OEM Telephony are 4 (0-3).
                // Here trip point with index(alarmID) 2 and 3 are used.
            case SET_TEMPERATURE_THRESHOLDS_0:
                                          alarmID = 2;
                                          ret = programSensorTripPoint(value, alarmID);
                                          break;
            case SET_TEMPERATURE_THRESHOLDS_1:
                                          alarmID = 3;
                                          ret = programSensorTripPoint(value, alarmID);
                                          break;
            case SET_PERF_PRESENT_CAPABILITY: ret = setModemPerfCapability(value);
                                          break;
            default: Log.e(TAG, "Invalid primitive id");
                     ret = UNSUCCESSFUL;
        }
        return ret;
    }

    private int getSensorMaxTemp(JhsReplyHeader jhsReplyHead, JhsReplyPayload returnByteArray) {
        int ret = UNSUCCESSFUL, maxTemp = -1, temp1 = -1, temp2 = -1, temp3 = -1, temp4 = -1, temp5 = -1;
        Log.d(TAG, "getSensorMaxTemp");

        mPhoneService = IOemTelephony.Stub.asInterface(ServiceManager.getService("oemtelephony"));
        if (mPhoneService == null)
            return ret;

        // get temp of all the sensors, find max of them
        // datatype 6 for type 'Temperature'
        jhsReplyHead.setJhsDataType(DATA_TYPE_TEMP);

        temp1 = readModemSensorTemp(OemTelephonyConstants.MODEM_SENSOR_RF);
        temp2 = readModemSensorTemp(OemTelephonyConstants.MODEM_SENSOR_BB);
        temp2 = readModemSensorTemp(OemTelephonyConstants.MODEM_SENSOR_PMU);
        temp4 = readModemSensorTemp(OemTelephonyConstants.MODEM_SENSOR_PA);
        ret = SUCCESSFUL;

        Log.d(TAG, "temp1: " + temp1 + " temp2: " + temp2 + " temp3: " + temp3 + " temp4: " + temp4 );
        maxTemp = Math.max(temp1, Math.max(temp2, Math.max(temp3, temp4)));
        // jhsReplyHead.setActualDataSize( sizeof(maxTemp) );
        jhsReplyHead.setActualDataSize(SIZE_OF_INTEGER);
        Log.d(TAG, "JhsDataType: "+ jhsReplyHead.getJhsDataType() + " ActualDataSize: " + jhsReplyHead.getActualDataSize() );
        returnByteArray.setJhsReplyPayload(maxTemp, SIZE_OF_INTEGER);

        return ret;

    }

    private int readModemSensorTemp(String sensorName) {
        int finalval = UNSUCCESSFUL;
        String value;

        try {
            value = mPhoneService.getThermalSensorValueV2(sensorName);
            if (value != null && value.length() > 0) {
                finalval = Integer.parseInt(value);
            }

            if (finalval == UNSUCCESSFUL) {
                Log.d(TAG, "finalval for sensor:"+ sensorName + " is invalid");
            } else {
                Log.d(TAG, "finalval for sensor:"+ sensorName + " is: " + finalval);
            }

        } catch (RemoteException e) {
            Log.e(TAG, "remote exception while reading thermal sensor value");
        }

        return finalval;
    }

    private int programSensorTripPoint(int value, int alarmID) {
        int ret = UNSUCCESSFUL;

        mPhoneService = IOemTelephony.Stub.asInterface(ServiceManager.getService("oemtelephony"));
        // check if OEM Telephony interface is sucessful
        if (mPhoneService == null)
            return ret;

        try {
            Log.d(TAG, "activating critical trip point alarm for all modem sensors");
            mPhoneService.activateThermalSensorNotificationV2(OemTelephonyConstants.MODEM_SENSOR_RF, alarmID, value, DEBOUNCE);
            mPhoneService.activateThermalSensorNotificationV2(OemTelephonyConstants.MODEM_SENSOR_BB, alarmID, value, DEBOUNCE);
            mPhoneService.activateThermalSensorNotificationV2(OemTelephonyConstants.MODEM_SENSOR_PMU, alarmID, value, DEBOUNCE);
            mPhoneService.activateThermalSensorNotificationV2(OemTelephonyConstants.MODEM_SENSOR_PA, alarmID, value, DEBOUNCE);
            ret = SUCCESSFUL;
        } catch (RemoteException e) {
            Log.e(TAG, "caught exception while programming critical trip points");
        }
        return ret;
    }

    private void registerModemIntentReceiver() {
        Log.d(TAG, "registerModemIntentReceiver");
        sModemIntentReceiver = new ModemIntentBroadcastReceiver();
        IntentFilter filter = new IntentFilter();
        Set<String> keys = sModemEventNameToOnReceiveFunctionMap.keySet();
        for(String key: keys){
            filter.addAction(key);
        }
        sContext.registerReceiver(sModemIntentReceiver, filter);
        sModemIntentRegistrationDone = true;
    }

    private static void unregisterModemIntentReceiver() {
        Log.d(TAG, "unregisterModemIntentReceiver");
        if (sModemIntentReceiver != null) {
            sContext.unregisterReceiver(sModemIntentReceiver);
            sModemIntentReceiver = null;
        }
    }

    private final class ModemIntentBroadcastReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "got intent with action: " + action);

            String eventSendingFunctionName = sModemEventNameToOnReceiveFunctionMap.get(action);
            Log.d(TAG, "Function which is going to send event to DPTF is: " + eventSendingFunctionName );

            try {
                JhsModem obj = new JhsModem();
                Method method = JhsModem.class.getDeclaredMethod(eventSendingFunctionName, Intent.class);
                method.invoke(obj, intent);
            } catch (IllegalAccessException e) {
                Log.e(TAG, "IllegalAccessException");
            } catch (NoSuchMethodException e) {
                Log.e(TAG, "NoSuchMethodException");
            } catch (InvocationTargetException e) {
                Log.e(TAG, "InvocationTargetException");
            }

        }
    }

    public static void modemReadyIntentReceived(Intent intent) {
        Log.i(TAG, "Modem is Ready");
        sModemReady = true;
        if (sModemParticipantActivePending == true) {
            isParticipantActive();
        }
    }

    public static void sensorThresholdReachedV2(Intent intent) {
        int participantHandle = 0, eventParam = -1;
        String sensorName = intent.getStringExtra(MODEM_SENSOR_ID_KEY);
        int temperature = intent.getIntExtra(MODEM_SENSOR_TEMPERATURE_KEY, 0);
        participantHandle = JhsManager.getPartHandleFromPartName(sParticipantName);
        Log.d(TAG, "Got notification for Sensor:" + sensorName + " with Current Temperature " + temperature);
        Log.d(TAG, "participantHandle: " + participantHandle );
        JhsEventManager.sendSpecificEventToDptf(EVENT_MODEM_SENSOR_THRESHOLD_REACHED_V2, participantHandle, eventParam);
    }

    public static void emergencyCallStatusChanged(Intent intent) {
        int enableFlag = -1, participantHandle = 0;
        sCallStatus = intent.getBooleanExtra("emergencyCallOngoing", false);
        enableFlag = (sCallStatus)? 1:0;
        JhsEventManager.sendGenericEventToDptf(participantHandle, EVENT_EMERGENCY_CALL_STATUS_CHANGED, enableFlag);
    }

    private int getAirplaneMode() {
        return (Settings.Global.getInt(sContext.getContentResolver(),
                Settings.Global.AIRPLANE_MODE_ON, -1) == 1) ?
                        1 : 0;
    }

    private int setModemPerfCapability(int value) {
        int ret = UNSUCCESSFUL;
        boolean emergencyCallStatus;
        boolean enable = false;

        Log.d(TAG, "setModemPerfCapability");

        // check if the device is already in requested mode
        if (getAirplaneMode() == value)
        {
            ret = SUCCESSFUL;
            return ret;
        }

        switch (value) {
            case 0: //Airplane mode OFF, data ON
                    sendAirplaneModeIntent(DISABLE);
                    setMobileDataOnOff(true);
                    ret = SUCCESSFUL;
                    break;
            case 1: // Mobile data OFF
                    setMobileDataOnOff(false);
                    ret = SUCCESSFUL;
                    break;
            case 2: // Check for emergency call, if call is not going on, put the device in airplane mode
                    emergencyCallStatus = getCallStatus();
                    if (emergencyCallStatus == false) {
                        sendAirplaneModeIntent(ENABLE);
                        ret = SUCCESSFUL;
                    }
                    else {
                        Log.e(TAG, "Emergency call going on");
                        ret = UNSUCCESSFUL;
                    }
                    break;
            default: Log.e(TAG, "Incorrect value for setting airplane mode");
                     ret = UNSUCCESSFUL;
        }

        return ret;
    }

    private void sendAirplaneModeIntent(int value) {
        boolean airplaneModeFlag = false;
        Log.d(TAG, "sendAirplaneModeIntent");

        Settings.Global.putInt(sContext.getContentResolver(), Settings.Global.AIRPLANE_MODE_ON, value);
        // Post the intent
        Intent intent = new Intent(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        airplaneModeFlag = (value == ENABLE) ? true : false;
        intent.putExtra("state", airplaneModeFlag);
        sContext.sendBroadcast(intent);
        return;
    }

    private void setMobileDataOnOff(boolean flag) {
        Log.d(TAG, "setMobileDataOnOff flag: " + flag);
        final TelephonyManager telephonyManager = (TelephonyManager) sContext.getSystemService(Context.TELEPHONY_SERVICE);
        telephonyManager.setDataEnabled(flag);
    }

}

