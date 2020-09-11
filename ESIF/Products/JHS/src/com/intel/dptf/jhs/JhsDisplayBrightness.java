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

import android.content.ContentResolver;
import android.content.Context;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.util.Log;

import java.lang.Math;

import static android.provider.Settings.System.SCREEN_BRIGHTNESS_MODE;
import static android.provider.Settings.System.SCREEN_BRIGHTNESS_MODE_AUTOMATIC;
import static android.provider.Settings.System.SCREEN_BRIGHTNESS_MODE_MANUAL;

import com.intel.dptf.jhs.binder.ActionParameters;
import com.intel.dptf.jhs.binder.JhsReplyHeader;
import com.intel.dptf.jhs.binder.JhsReplyPayload;

import static com.intel.dptf.jhs.JhsManager.DATA_TYPE_PERCENT;
import static com.intel.dptf.jhs.JhsManager.SIZE_OF_INTEGER;
import static com.intel.dptf.jhs.JhsManager.SUCCESSFUL;
import static com.intel.dptf.jhs.JhsManager.UNSUCCESSFUL;


/*
 * JhsDisplayBrightness supports the primitives related to display participant
 *
 * @hide
 */
public class JhsDisplayBrightness {
    private Context lContext = null;
    private static final int GET_DISPLAY_BRIGHTNESS = 1129398879; // Decimal value for: 0x4351425f;
    private static final int MAX_BRIGHTNESS = 255;
    private static final int sDeviceType = 0;
    private static final int SET_DISPLAY_BRIGHTNESS = 1296253535; // Decimal value for: 0x4d43425f;
    private static final String sClassName = "JhsDisplayBrightness";
    private static final String sParticipantName = "INT3406";
    private static final String TAG = "JHS:JhsDisplayBrightness";

    public void isParticipantActive() {
        Log.d(TAG, "isParticipantActive");
        JhsManager.sendParticipantActiveToDptf(sParticipantName, sDeviceType, sClassName);
    }

    public int getValue(ActionParameters actionParams, JhsReplyHeader jhsReplyHead, JhsReplyPayload returnByteArray) {
        int actionId, ret = UNSUCCESSFUL, retVal = -1;

        Log.d(TAG, "getValue");
        actionId = actionParams.getActionType();
        switch (actionId) {
            case GET_DISPLAY_BRIGHTNESS: ret = getDisplayBrightness(jhsReplyHead, returnByteArray);
                      break;
            default: Log.e(TAG, "Invalid action id");
                     ret = UNSUCCESSFUL;
        }
        return ret;
    }

    public int setValue(ActionParameters actionParams, int value) {
        int actionId, ret = UNSUCCESSFUL, retVal = -1;

        Log.d(TAG, "setValue");
        actionId = actionParams.getActionType();
        switch (actionId) {
            case SET_DISPLAY_BRIGHTNESS: ret = setDisplayBrightness(value);
                      break;
            default: Log.e(TAG, "Invalid action id");
                     ret = UNSUCCESSFUL;
                     return ret;
        }
        return ret;
    }

    public int getDisplayBrightnessMode() {
        int brightness, curBrightness, curMode = -1, ret = UNSUCCESSFUL;
        lContext = JhsManager.getContext();
        if (lContext == null)
            return ret;
        ContentResolver cr = lContext.getContentResolver();
        try {
            curMode = Settings.System.getInt(cr, Settings.System.SCREEN_BRIGHTNESS_MODE);
        }
        catch(SettingNotFoundException e) {
            Log.e(TAG, "Caught " + e);
        }

        return curMode;
    }

    // setp 355 D0 255 <value> (SET_DISPLAY_BRIGHTNESS)
    public int setDisplayBrightness(int percentBrightness) {
        int ret = UNSUCCESSFUL, brightnessValue, displayBrightnessMode;
        float autoBrightnessValue;
        lContext = JhsManager.getContext();
        if (lContext == null) {
            ret = UNSUCCESSFUL;
            return ret;
        }
        ContentResolver cr = lContext.getContentResolver();
        Log.d(TAG, "setDisplayBrightness");

        displayBrightnessMode = getDisplayBrightnessMode();
        if (displayBrightnessMode ==  SCREEN_BRIGHTNESS_MODE_AUTOMATIC) {
            autoBrightnessValue = (((float)percentBrightness * 2) / 100) - (float)1.0;
            Log.d(TAG, "setDisplayBrightness percentBrightness: " + percentBrightness + " autoBrightnessValue: " + autoBrightnessValue);
            Settings.System.putFloat(cr, Settings.System.SCREEN_AUTO_BRIGHTNESS_ADJ, autoBrightnessValue);
            ret = SUCCESSFUL;
        }

        else if (displayBrightnessMode ==  SCREEN_BRIGHTNESS_MODE_MANUAL) {
            brightnessValue = (MAX_BRIGHTNESS * percentBrightness) / 100;
            Log.d(TAG, "setDisplayBrightness percentBrightness: " + percentBrightness + " brightnessValue: " + brightnessValue);
            Settings.System.putInt(cr, Settings.System.SCREEN_BRIGHTNESS, brightnessValue);
            ret = SUCCESSFUL;
        }

        return ret;
    }

    // getp 356 D0 255 (GET_DISPLAY_BRIGHTNESS)
    public int getDisplayBrightness(JhsReplyHeader jhsReplyHead, JhsReplyPayload returnByteArray) {
        int curBrightness, curBrightnessPercent = -1, displayBrightnessMode, ret = UNSUCCESSFUL;
        float autoCurBrightness;

        jhsReplyHead.setJhsDataType(DATA_TYPE_PERCENT);

        lContext = JhsManager.getContext();
        if (lContext == null) {
            ret = UNSUCCESSFUL;
            return ret;
        }
        ContentResolver cr = lContext.getContentResolver();
        Log.d(TAG, "getDisplayBrightness");

        try {
            displayBrightnessMode = getDisplayBrightnessMode();
            if (displayBrightnessMode ==  SCREEN_BRIGHTNESS_MODE_AUTOMATIC) {
                autoCurBrightness = Settings.System.getFloat(cr, Settings.System.SCREEN_AUTO_BRIGHTNESS_ADJ);
                curBrightnessPercent = Math.round((float)(autoCurBrightness + 1) * (float)(100 / 2) );
                Log.d(TAG, "getDisplayBrightness autoCurBrightness: " + autoCurBrightness + " curBrightnessPercent: " + curBrightnessPercent);
                ret = SUCCESSFUL;
            }

            else if (displayBrightnessMode ==  SCREEN_BRIGHTNESS_MODE_MANUAL) {
                curBrightness = Settings.System.getInt(cr, Settings.System.SCREEN_BRIGHTNESS);
                curBrightnessPercent = Math.round((float)(100 * curBrightness) / MAX_BRIGHTNESS);
                Log.d(TAG, "getDisplayBrightness curBrightness: " + curBrightness + " curBrightnessPercent: " + curBrightnessPercent);
                ret = SUCCESSFUL;
            }
            jhsReplyHead.setActualDataSize(SIZE_OF_INTEGER);
            returnByteArray.setJhsReplyPayload(curBrightnessPercent, SIZE_OF_INTEGER);
            return ret;
        }
        catch(SettingNotFoundException e) {
            Log.e(TAG, "Caught " + e);
            ret = UNSUCCESSFUL;
            return ret;
        }
    }

}
