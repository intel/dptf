/*
* Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;

import com.intel.dptf.jhs.binder.ActionParameters;
import com.intel.dptf.jhs.binder.JhsParticipantHandle;
import com.intel.dptf.jhs.binder.DptfParticipant;
import com.intel.dptf.jhs.binder.DptfParticipantDeviceType;
import com.intel.dptf.jhs.binder.IJhsClientService;
import com.intel.dptf.jhs.binder.JhsReplyHeader;
import com.intel.dptf.jhs.binder.JhsReplyPayload;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Hashtable;

/*
 * The JhsManager class contains data structures that are common to both
 * JHS and DPTF.
 *
 * @hide
 */
public class JhsManager {
    private static ArrayList<String> sClassNameArray = new ArrayList<String>();
    private static boolean sIntelModem = false;
    private static Context sContext;
    private static final String TAG = "JHS:JhsManager";
    private static int sMaxClassNum;
    private static Hashtable<Integer, String> sDptfHandleToActionClassMap = new Hashtable<Integer, String>();
    private static Hashtable<String, Integer> sDptfParticipantNameToHandleMap = new Hashtable<String, Integer>();
    public static final int DATA_TYPE_PERCENT = 29;
    public static final int DATA_TYPE_STRING = 8;
    public static final int DATA_TYPE_TEMP = 6;
    public static final int SIZE_OF_INTEGER = 4;
    public static final int SUCCESSFUL = 1;
    public static final int UNSUCCESSFUL = -1;
    public static IJhsClientService sService = null;

    public static void init(Context con) {
        sContext = con;

        checkForIntelModem();
        getJhsClientServiceHandle();
        addClassNameToArray();
        checkParticipantActive();
    }

    private static void getJhsClientServiceHandle() {
        IBinder binder = null;
        int POLL_TIME = 1000;
        Log.d(TAG, "getJhsClientServiceHandle");
        try {
            while (sService == null) {
                binder = ServiceManager.getService("java_helper_client_service");
                sService = IJhsClientService.Stub.asInterface(binder);
                Thread.sleep(POLL_TIME);
                Log.d(TAG, "sService: " + sService);
            }
            Log.d(TAG, "sService: " + sService);
        } catch (Exception e) {
            Log.e(TAG, "Caught:" + e);
        }
    }

    private static void checkForIntelModem() {
        try {
            Class cls = Class.forName("com.intel.dptf.jhs.JhsModem");
            Constructor cons = cls.getDeclaredConstructor();
            Object obj = cons.newInstance();
            if (obj != null) {
                Log.i(TAG, "JhsModem can be used since intel modem exists");
                sIntelModem = true;
            }
            else {
                sIntelModem = false;
            }
        } catch (Exception e) {
            Log.i(TAG, "Exception: " + e);
            sIntelModem = false;
        }
    }

    private static void addClassNameToArray() {
        sClassNameArray.add("JhsDisplayBrightness");
        sClassNameArray.add("JhsSystem");
        if (sIntelModem == true)
            sClassNameArray.add("JhsModem");
        sMaxClassNum = sClassNameArray.size();
        Log.d(TAG, "className array size: " + sMaxClassNum);
    }

    public static Context getContext() {
        return sContext;
    }

    public static void dptfHandleToActionClassMap(int handle, String className) {
        sDptfHandleToActionClassMap.put(handle, className);
        Log.d(TAG, "handle: " + handle + " className: " + className);
    }

    private static void dptfParticipantNameToHandleMap(String dptfPartName, int handle) {
        sDptfParticipantNameToHandleMap.put(dptfPartName, handle);
        Log.d(TAG, "dptfPartName: " + dptfPartName + " handle: " + handle);
    }

    public static int getPartHandleFromPartName(String dptfPartName) {
        int handle = sDptfParticipantNameToHandleMap.get(dptfPartName);
        return handle;
    }

    private static void checkParticipantActive() {
        Log.d(TAG, "checkParticipantActive called");
        String className;
        int classIndex;

        for (classIndex = 0; classIndex < sMaxClassNum; classIndex++) {
            className = sClassNameArray.get(classIndex);
            Log.d(TAG, "className: " + className);
            if (className == null) {
                Log.e(TAG, "classNaame is null");
            }
            try {
                Class cls = Class.forName("com.intel.dptf.jhs." + className);
                Object obj = cls.newInstance();
                Method method = cls.getDeclaredMethod("isParticipantActive");
                method.invoke(obj);
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
    }

    public static void sendParticipantActiveToDptf(String dptfPart, int deviceType, String className) {
        int handle = -1;
        Log.d(TAG, "sendParticipantActiveToDptf, DptfParticipant: " + dptfPart + " DptfParticipantDeviceType: " + deviceType);

        DptfParticipant dptfParticipant = new DptfParticipant(dptfPart);
        DptfParticipantDeviceType partDeviceType = new DptfParticipantDeviceType();
        partDeviceType.setDptfPartDeviceType(deviceType);

        try {
            handle = sService.participantActive(dptfParticipant, partDeviceType);
            Log.e(TAG, "handle: " + handle );
            dptfHandleToActionClassMap(handle, className);
            dptfParticipantNameToHandleMap(dptfPart, handle);
        } catch (RemoteException e) {
            Log.e(TAG, "RemoteException");
        } catch (RuntimeException e) {
            Log.e(TAG, "RuntimeException");
        }
    }

    public static void sendParticipantInactiveToDptf(String dptfPart) {
        int handle = -1;
        Log.d(TAG, "sendParticipantInactiveToDptf");

        if (dptfPart != null) {
            handle = sDptfParticipantNameToHandleMap.get(dptfPart);

            // 0 is an invalid handle. Any positive number is a good handle
            if (handle > 0) {
                JhsParticipantHandle dptfHand = new JhsParticipantHandle(handle);
                try {
                    sService.participantInactive(dptfHand);
                } catch (RemoteException e) {
                    Log.e(TAG, "RemoteException");
                }
            }
            else {
                Log.e(TAG, "Invalid handle");
            }
        }
        else {
            Log.e(TAG, "String dptfPart is NULL");
        }
    }

    public static void resetJhs() {
        sService = null;
        sDptfHandleToActionClassMap.clear();
        sDptfParticipantNameToHandleMap.clear();
        getJhsClientServiceHandle();
        addClassNameToArray();
        checkParticipantActive();
    }

    public static int getValue(JhsParticipantHandle dptfHand, ActionParameters actionParams, JhsReplyHeader jhsReplyHead, JhsReplyPayload returnByteArray) {
        Log.d(TAG, "getValue called");
        String className;
        int ret = UNSUCCESSFUL;
        int dptfHandleInt = dptfHand.getJhsParticipantHandle();

        className = sDptfHandleToActionClassMap.get(dptfHandleInt);
        Log.d(TAG, "className: " + className);
        if (className == null) {
            ret = UNSUCCESSFUL;
            return ret;
        }
        try {
            Class cls = Class.forName("com.intel.dptf.jhs." + className);
            Object obj = cls.newInstance();
            Class paramTypes[] = new Class[3];
            paramTypes[0] = ActionParameters.class;
            paramTypes[1] = JhsReplyHeader.class;
            paramTypes[2] = JhsReplyPayload.class;
            Method method = cls.getDeclaredMethod("getValue", paramTypes);
            Object argList[] = new Object[3];
            argList[0] = new ActionParameters(actionParams.getActionType(), actionParams.getDomainId(), actionParams.getInstanceId(),
                    actionParams.getReplyDataType(), actionParams.getReplyBufferSize() );
            argList[1] = jhsReplyHead;
            argList[2] = returnByteArray;
            ret = (Integer) method.invoke(obj, argList);
        } catch (ClassNotFoundException e) {
            Log.e(TAG, "ClassNotFoundException");
            ret = UNSUCCESSFUL;
        } catch (InstantiationException e) {
            Log.e(TAG, "InstantiationException");
            ret = UNSUCCESSFUL;
        } catch (IllegalAccessException e) {
            Log.e(TAG, "IllegalAccessException");
            ret = UNSUCCESSFUL;
        } catch (NoSuchMethodException e) {
            Log.e(TAG, "NoSuchMethodException");
            ret = UNSUCCESSFUL;
        } catch (InvocationTargetException e) {
            Log.e(TAG, "InvocationTargetException");
            ret = UNSUCCESSFUL;
        }
        return ret;
    }

    public static int setValue(JhsParticipantHandle dptfHand, ActionParameters actionParams, int value) {
        Log.d(TAG, "setValue called");
        String className;
        int ret = UNSUCCESSFUL;
        int dptfHandleInt = dptfHand.getJhsParticipantHandle();

        className = sDptfHandleToActionClassMap.get(dptfHandleInt);
        Log.d(TAG, "className: " + className);
        if (className == null) {
            ret = UNSUCCESSFUL;
            return ret;
        }
        try {
            Class cls = Class.forName("com.intel.dptf.jhs." + className);
            Object obj = cls.newInstance();
            Class paramTypes[] = new Class[2];
            paramTypes[0] = ActionParameters.class;
            paramTypes[1] = int.class;
            Method method = cls.getDeclaredMethod("setValue", paramTypes);
            Object argList[] = new Object[2];
            argList[0] = new ActionParameters(actionParams.getActionType(), actionParams.getDomainId(), actionParams.getInstanceId(),
                    actionParams.getReplyDataType(), actionParams.getReplyBufferSize() );
            argList[1] = value;
            ret = (Integer) method.invoke(obj, argList);
        } catch (ClassNotFoundException e) {
            Log.e(TAG, "ClassNotFoundException");
            ret = UNSUCCESSFUL;
        } catch (InstantiationException e) {
            Log.e(TAG, "InstantiationException");
            ret = UNSUCCESSFUL;
        } catch (IllegalAccessException e) {
            Log.e(TAG, "IllegalAccessException");
            ret = UNSUCCESSFUL;
        } catch (NoSuchMethodException e) {
            Log.e(TAG, "NoSuchMethodException");
            ret = UNSUCCESSFUL;
        } catch (InvocationTargetException e) {
            Log.e(TAG, "InvocationTargetException");
            ret = UNSUCCESSFUL;
        }
        return ret;
    }


}

