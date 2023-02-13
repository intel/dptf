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

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.util.Log;

import static java.lang.Math.PI;

/*
 * The JhsMotionSensorManager class registers for motion sensor listeners
 * and sends event to DPTF upon receiving the callback
 *
 * @hide
 */

public class JhsMotionSensorManager {
    private static Context sContext;
    // Add all motion sensor event IDs
    private static final int EVENT_DEVICE_ORIENTATION_CHANGED = 27;
    private static final int EVENT_MOTION_CHANGED = 28;
    private static final String TAG = "JHS:JhsMotionSensorManager";

    public static final class DeviceMotionSensorListener implements SensorEventListener {
        private boolean isDeviceMovingCurrentValue = false;
        private int SAMPLING_RATE = 15000000, MAX_REPORT_LATENCY = 15000000;
        private static boolean sIsDeviceMovingLastValue = false;
        private static int sDeviceOrientationLast = -1;
        private static float[] mGeomagnetic = null;
        private static float[] mGravity = null;
        private static int deviceOrientation = -1;
        private static SensorManager mSensorManager;
        private static Sensor mLinearAcceleration, mMagneticField, mAccelerometer;

        public DeviceMotionSensorListener(Context con) {
            Log.d(TAG, "DeviceOrientationListener");
            sContext = con;
            mSensorManager = (SensorManager)sContext.getSystemService(Context.SENSOR_SERVICE);
            mLinearAcceleration = mSensorManager.getDefaultSensor(Sensor.TYPE_LINEAR_ACCELERATION);
            mMagneticField = mSensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD);
            mAccelerometer = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
            mSensorManager.registerListener(this, mLinearAcceleration, SAMPLING_RATE, MAX_REPORT_LATENCY);
            mSensorManager.registerListener(this, mMagneticField, SAMPLING_RATE);
            mSensorManager.registerListener(this, mAccelerometer, SAMPLING_RATE);
        }

        public void onSensorChanged(SensorEvent event) {
            int participantHandle = 0, deviceOrientation = -1, deviceInMotion = -1;
            int deviceOrientationX = -1, deviceOrientationY = -1;
            float azimuth = -99, pitch = -99, roll = -99;
            float orientation[] = new float[3];
            float R[] = new float[9];
            float I[] = new float[9];
            Log.d(TAG, "onSensorChanged");

            if (event.sensor.getType() == Sensor.TYPE_LINEAR_ACCELERATION) {
                Log.d(TAG, "event.values[0]: " + event.values[0] + " event.values[1]: " + event.values[1] + " event.values[2]: " + event.values[2]);
                if ( event.values[0] != 0 || event.values[1] != 0 || event.values[2] != 0 ) {
                    isDeviceMovingCurrentValue = true;
                }
                else isDeviceMovingCurrentValue = false;
                // Event is sent to DPTF when the device state changes from stationary to moving and moving to stationary
                if (sIsDeviceMovingLastValue != isDeviceMovingCurrentValue) {
                    deviceInMotion = isDeviceMovingCurrentValue ? 1 : 0;
                    JhsEventManager.sendSpecificEventToDptf(participantHandle, EVENT_MOTION_CHANGED, deviceInMotion);
                    sIsDeviceMovingLastValue = isDeviceMovingCurrentValue;
                }
            }

            if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER) {
                mGravity = event.values;
            }
            if (event.sensor.getType() == Sensor.TYPE_MAGNETIC_FIELD) {
                mGeomagnetic = event.values;
            }
            if (mGravity != null && mGeomagnetic != null) {
                boolean success = SensorManager.getRotationMatrix(R, I, mGravity, mGeomagnetic);
                if (success) {
                    SensorManager.getOrientation(R, orientation);
                    azimuth = orientation[0]; // orientation contains: azimuth, pitch and roll
                    pitch = orientation[1];
                    roll = orientation[2];
                    Log.d(TAG, "azimuth: " + azimuth + " pitch: " + pitch + " roll: " + roll );
                }
            }

            // Calculate orientation based on pitch and roll
            // Roll varies from -PI to PI
            if ( roll >= -PI/4 && roll <= PI/4 ) {
                Log.d(TAG, "deviceOrientationY = 2, Horizontal, facing up");
                deviceOrientationY = 2; // Horizontal, facing up
            }
            else if ( (roll >= 3*PI/4 && roll <= PI) || (roll >= -PI && roll <= -3*PI/4) ) {
                Log.d(TAG, "deviceOrientationY = 3, Horizontal, facing down");
                deviceOrientationY = 3; // Horizontal, facing down
            }
            else if ( (roll > PI/4 && roll < 3*PI/4) || (roll > -3*PI/4 && roll < -PI/4) ) {
                Log.d(TAG, "deviceOrientationY = 4, Vertical");
                deviceOrientationY = 4; // Vertical
            }
            // pitch value varies from -PI/2 to PI/2
            if ( pitch >= -PI/2 && pitch <= -PI/4 ) {
                Log.d(TAG, "deviceOrientationX = 5, Vertical, upright");
                deviceOrientationX = 5; // Vertical, upright
            }
            else if ( pitch >= PI/4 && pitch <= PI/2 ) {
                Log.d(TAG, "deviceOrientationX = 6, Vertical, upside down");
                deviceOrientationX = 6; // Vertical, upside down
            }
            else if ( pitch > -PI/4 && pitch < PI/4 ) {
                Log.d(TAG, "deviceOrientationX = 1, Horizontal");
                deviceOrientationX = 1; // Horizontal
            }

            // Compare pitch and roll and decide device orientation. Greater the number, higher the priority.
            deviceOrientation = deviceOrientationX > deviceOrientationY ? deviceOrientationX : deviceOrientationY;
            Log.d(TAG, "deviceOrientation = " + deviceOrientation);
            // If the current orientation differs from the last one, send event to DPTF
            if (sDeviceOrientationLast != deviceOrientation) {
                JhsEventManager.sendSpecificEventToDptf(participantHandle, EVENT_DEVICE_ORIENTATION_CHANGED, deviceOrientation);
                sDeviceOrientationLast = deviceOrientation;
            }
        }

        public void onAccuracyChanged(Sensor sensor, int accuracy) {
            Log.d(TAG, "onAccuracyChanged");
        }

        public void unregisterMotionSensorListener() {
            Log.d(TAG, "unregisterMotionSensorListener");
            mSensorManager.unregisterListener(this);
        }

    }

}
