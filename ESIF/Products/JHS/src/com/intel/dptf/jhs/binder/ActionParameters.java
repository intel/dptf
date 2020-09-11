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

package com.intel.dptf.jhs.binder;

import android.os.Parcel;
import android.os.Parcelable;
import android.util.Log;

public class ActionParameters implements Parcelable {
    int mActionType;
    int mDomainId;
    int mInstanceId;
    int mReplyDataType;
    int mReplyBufferSize;

    public ActionParameters() {
    }

    public ActionParameters(int actionType, int domain, int instance, int replyDataType, int replyBufferSize) {
        mActionType = actionType;
        mDomainId = domain;
        mInstanceId = instance;
        mReplyDataType = replyDataType;
        mReplyBufferSize = replyBufferSize;
    }

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel outPar, int flag) {
        outPar.writeInt(mActionType);
        outPar.writeInt(mDomainId);
        outPar.writeInt(mInstanceId);
        outPar.writeInt(mReplyDataType);
        outPar.writeInt(mReplyBufferSize);
    }

    public void readFromParcel(Parcel inPar) {
        mActionType = inPar.readInt();
        mDomainId = inPar.readInt();
        mInstanceId = inPar.readInt();
        mReplyDataType = inPar.readInt();
        mReplyBufferSize = inPar.readInt();
    }

    public int getActionType() {
        return mActionType;
    }

    public int getDomainId() {
        return mDomainId;
    }

    public int getInstanceId() {
        return mInstanceId;
    }

    public int getReplyDataType() {
        return mReplyDataType;
    }

    public int getReplyBufferSize() {
        return mReplyBufferSize;
    }

    public static final Parcelable.Creator<ActionParameters> CREATOR
            = new Parcelable.Creator<ActionParameters>() {
        @Override
        public ActionParameters createFromParcel(Parcel in) {
            int actionType = in.readInt();
            int domain = in.readInt();
            int instance = in.readInt();
            int replyDataType = in.readInt();
            int replyBufferSize = in.readInt();
            return new ActionParameters(actionType, domain, instance, replyDataType, replyBufferSize);
        }

        @Override
        public ActionParameters[] newArray(int size) {
            return new ActionParameters[size];
        }
    };
}
