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

public class JhsReplyHeader implements Parcelable {
    int mJhsDataType;
    int mActualDataSize;

    public JhsReplyHeader() {
    }

    public JhsReplyHeader(int type, int size) {
        mJhsDataType = type;
        mActualDataSize = size;
    }

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel outPar, int flag) {
        outPar.writeInt(mJhsDataType);
        outPar.writeInt(mActualDataSize);
    }

    public void readFromParcel(Parcel inPar) {
        mJhsDataType = inPar.readInt();
        mActualDataSize = inPar.readInt();
    }

    public void setJhsDataType(int type) {
        mJhsDataType = type;
    }

    public void setActualDataSize(int size) {
        mActualDataSize = size;
    }

    public int getJhsDataType() {
        return mJhsDataType;
    }

    public int getActualDataSize() {
        return mActualDataSize;
    }

    public static final Parcelable.Creator<JhsReplyHeader> CREATOR
            = new Parcelable.Creator<JhsReplyHeader>() {
        @Override
        public JhsReplyHeader createFromParcel(Parcel in) {
            int type = in.readInt();
            int size = in.readInt();
            return new JhsReplyHeader(type, size);
        }

        @Override
        public JhsReplyHeader[] newArray(int size) {
            return new JhsReplyHeader[size];
        }
    };
}
