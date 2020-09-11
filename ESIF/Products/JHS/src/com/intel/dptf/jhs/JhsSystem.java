/*
* Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;
import android.os.SystemProperties;
import android.util.Log;

import com.intel.dptf.jhs.binder.ActionParameters;
import com.intel.dptf.jhs.binder.JhsReplyHeader;
import com.intel.dptf.jhs.binder.JhsReplyPayload;

import static com.intel.dptf.jhs.JhsManager.DATA_TYPE_STRING;
import static com.intel.dptf.jhs.JhsManager.SUCCESSFUL;
import static com.intel.dptf.jhs.JhsManager.UNSUCCESSFUL;

import java.util.List;

/*
 * JhsSystem will provide functions to support system primitives
 *
 * @hide
 */
public class JhsSystem {
    private static Context sContext = null;
    private static final int sDeviceType = 0;
    private static final int GET_FOREGROUND_APPLICATION_NAME = 1095190087; // Decimal value for: 0x41474647;
    private static final int SET_SYSTEM_SHUTDOWN = 1313097811; // Decimal value for: 0x4e444853;
    private static final int SET_RESET_JHS = 1447381587; //Decimal value for 0x56454A53
    private static final int SET_MOTION_SENSOR = 1213418067; // Decimal value for 0x48534A53
    private static final String sClassName = "JhsSystem";
    private static final String sParticipantName = "INT3400";
    private static final String TAG = "JHS:JhsSystem";
    private static int sMotionSensorRegFlag = 0;
    private static JhsMotionSensorManager.DeviceMotionSensorListener sMotionSensorObj = null;

    public void isParticipantActive() {
        Log.d(TAG, "isParticipantActive");
        JhsManager.sendParticipantActiveToDptf(sParticipantName, sDeviceType, sClassName);
        sContext = JhsManager.getContext();
    }

    public int getValue(ActionParameters actionParams, JhsReplyHeader jhsReplyHead, JhsReplyPayload returnByteArray) {
        int actionId, ret = UNSUCCESSFUL, retVal = -1;

        Log.d(TAG, "getValue");
        actionId = actionParams.getActionType();
        switch (actionId) {
            case GET_FOREGROUND_APPLICATION_NAME: ret = getForegroundApplicationName(jhsReplyHead, returnByteArray);
                      break;
            default: Log.e(TAG, "Invalid action id");
                     ret = UNSUCCESSFUL;
        }
        return ret;
    }

    public int setValue(ActionParameters actionParams, int value) {
        int actionId, ret = UNSUCCESSFUL;
        Log.d(TAG, "setValue");
        actionId = actionParams.getActionType();
        switch (actionId) {
            case SET_SYSTEM_SHUTDOWN: ret = doShutdown();
                break;
            case SET_RESET_JHS:
                ret = setResetJhs();
                break;
            case SET_MOTION_SENSOR: ret = motionSenorRegisterUnregister(value);
                break;
            default: Log.e(TAG, "Invalid action id");
                ret = UNSUCCESSFUL;
        }
        return ret;
    }

    private int getForegroundApplicationName(JhsReplyHeader jhsReplyHead, JhsReplyPayload returnByteArray)
    {
        int ret = UNSUCCESSFUL;
        String foregroundApp = "NULL";
        ActivityManager am = (ActivityManager)sContext.getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningTaskInfo> taskInfo = am.getRunningTasks(1);
        foregroundApp = taskInfo.get(0).topActivity.getClassName();
        Log.d(TAG, "Current Activity :" + foregroundApp);

        jhsReplyHead.setJhsDataType(DATA_TYPE_STRING);
        jhsReplyHead.setActualDataSize(foregroundApp.length());
        returnByteArray.setJhsReplyPayloadString(foregroundApp, foregroundApp.length());

        return SUCCESSFUL;
    }

    private int doShutdown() {
        int ret = UNSUCCESSFUL;

        SystemProperties.set("sys.powerctl.no.shutdown", "0");
        // Set this system property so that the device with any PMIC version
        // will not reboot to COS upon thermal shutdown, even when the charger is connected
        SystemProperties.set("sys.powerctl.criticalshutdown", "1");
        Intent criticalIntent = new Intent(Intent.ACTION_REQUEST_SHUTDOWN);
        criticalIntent.putExtra(Intent.EXTRA_KEY_CONFIRM, false);
        criticalIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        Log.i(TAG, "Initiating shutdown");

        if (sContext == null) {
            ret = UNSUCCESSFUL;
        }
        else {
            sContext.startActivity(criticalIntent);
            ret = SUCCESSFUL;
        }
        return ret;
    }

    private int motionSenorRegisterUnregister(int value) {
        int ret = UNSUCCESSFUL;
        if (value == 1 && sMotionSensorRegFlag == 0) {
            sMotionSensorObj = new JhsMotionSensorManager.DeviceMotionSensorListener(sContext);
            ret = SUCCESSFUL;
            sMotionSensorRegFlag = 1;
        }
        else if (value == 0 && sMotionSensorRegFlag == 1) {
            sMotionSensorObj.unregisterMotionSensorListener();
            ret = SUCCESSFUL;
            sMotionSensorRegFlag = 0;
        }
        else {
            Log.e(TAG, "Can not register/unregister motion sensor");
            ret = UNSUCCESSFUL;
        }
        return ret;
    }

    private int setResetJhs() {
        // reset data structures and get interface of java_helper_client_service
        Log.i(TAG, "Reinitializing, getting new interface to java_helper_client_service");
        JhsManager.resetJhs();
        return SUCCESSFUL;
    }
}

