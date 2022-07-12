/*
* Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

package com.intel.dptf.jhs.binder;

import android.os.Parcel;
import android.os.Parcelable;
import android.util.Log;

import java.nio.ByteBuffer;
import java.lang.Byte;
import java.util.ArrayList;
import java.util.Arrays;

import java.nio.ByteOrder;

public class JhsReplyPayload implements Parcelable {
    byte[] mReturnResult;
    private static final String TAG = "JhsReplyPayload";

    public JhsReplyPayload() {
    }

    public JhsReplyPayload(int result) {
        mReturnResult = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(result).array();
        Log.d(TAG, "mReturnResult: " + Arrays.toString(mReturnResult));
    }

    public int describeContents() {
        return 0;
    }

    public void setJhsReplyPayload(int result, int size) {
        //mReturnResult = new byte[size];
        mReturnResult = ByteBuffer.allocate(size).order(ByteOrder.LITTLE_ENDIAN).putInt(result).array();
        Log.d(TAG, "mReturnResult: " + Arrays.toString(mReturnResult));
    }

    public void setJhsReplyPayloadString(String result, int size) {
        mReturnResult = new byte[size];
        mReturnResult = result.getBytes();
        Log.d(TAG, "mReturnResult: " + mReturnResult);
    }

    public byte[] getJhsReplyPayload() {
        return mReturnResult;
    }

    public void writeToParcel(Parcel outPar, int flag) {
        outPar.writeByteArray(mReturnResult);
    }

    public void readFromParcel(Parcel inPar) {
        inPar.readByteArray(mReturnResult);
    }

    public static final Parcelable.Creator<JhsReplyPayload> CREATOR
            = new Parcelable.Creator<JhsReplyPayload>() {
        @Override
        public JhsReplyPayload createFromParcel(Parcel in) {
            int result = in.readInt();
            return new JhsReplyPayload(result);
        }

        @Override
        public JhsReplyPayload[] newArray(int size) {
            return new JhsReplyPayload[size];
        }
    };
}
