/*
* Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.os.ServiceManager;
import android.util.Log;

import com.intel.dptf.jhs.binder.ActionParameters;
import com.intel.dptf.jhs.binder.JhsParticipantHandle;
import com.intel.dptf.jhs.binder.IJhsService;
import com.intel.dptf.jhs.binder.JhsReplyHeader;
import com.intel.dptf.jhs.binder.JhsReplyPayload;

import java.nio.ByteBuffer;
import java.util.Arrays;

import static com.intel.dptf.jhs.JhsManager.SUCCESSFUL;
import static com.intel.dptf.jhs.JhsManager.UNSUCCESSFUL;

/*
 * JavaHelperService will provide services for DPTF in Android
 *
 * @hide
 */
public class JavaHelperService extends IJhsService.Stub {
    private static final String TAG = "JHS:JavaHelperService";
    private static JavaHelperService sJhsInstance;

    public static void init (Context con) {
        synchronized (JavaHelperService.class) {
            if (sJhsInstance == null) {
                sJhsInstance = new JavaHelperService();
            }
        }
        JhsEventManager.init(con);
        JhsManager.init(con);
    }

    private JavaHelperService() {
        publish();
    }

    private void publish() {
        Log.d(TAG, "Publish JavaHelperService");
        ServiceManager.addService("java_helper_service", this);
    }

    public int getValue(JhsParticipantHandle dptfHand, ActionParameters actionParams, JhsReplyHeader jhsReplyHead, JhsReplyPayload returnByteArray){
        // Action based on dptf type/name
        int ret = UNSUCCESSFUL;
        Log.d(TAG, "getValue API called");
        Log.d(TAG, "Handle: " + dptfHand.getJhsParticipantHandle() + " Action Type: " + actionParams.getActionType() + " Domain: " +
                        actionParams.getDomainId() + " Instance: " + actionParams.getInstanceId() + " Reply Type: " +
                        actionParams.getReplyDataType() + " Reply buffer size: " + actionParams.getReplyBufferSize() );
        Log.d(TAG, "actionTypeHex: " + Integer.toHexString( actionParams.getActionType() ) );

        ret = JhsManager.getValue(dptfHand, actionParams, jhsReplyHead, returnByteArray);
        Log.d(TAG, "getValue returned: " + ret);
        Log.d(TAG, "Reply data type: " + jhsReplyHead.getJhsDataType() + " actual data size: " + jhsReplyHead.getActualDataSize() );
        Log.d(TAG, "returnByteArray: " + Arrays.toString(returnByteArray.getJhsReplyPayload()) );
        return ret;
    }

    public int setValue(JhsParticipantHandle dptfHand, ActionParameters actionParams, int value) {
        //Action based on dptf type/name
        int ret = UNSUCCESSFUL;
        Log.d(TAG, "setValue API called");
        Log.d(TAG, "Handle: " + dptfHand.getJhsParticipantHandle() + " Action Type: " + actionParams.getActionType() + " Domain: " +
                        actionParams.getDomainId() + " Instance: " + actionParams.getInstanceId() +
                        actionParams.getReplyDataType() + " Reply buffer size: " + actionParams.getReplyBufferSize() + " value: " + value);
        Log.d(TAG, "actionTypeHex: " + Integer.toHexString( actionParams.getActionType() ) );

        ret = JhsManager.setValue(dptfHand, actionParams, value);
        Log.d(TAG, "setValue returned: " + ret);
        return ret;
    }

}

