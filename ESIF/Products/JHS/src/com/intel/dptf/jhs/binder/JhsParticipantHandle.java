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

package com.intel.dptf.jhs.binder;

import android.os.Parcel;
import android.os.Parcelable;
import android.util.Log;

public class JhsParticipantHandle implements Parcelable {
    int mParticipantId;

    public JhsParticipantHandle() {
    }

    public JhsParticipantHandle(int id) {
        mParticipantId = id;
    }

    public int describeContents() {
        return 0;
    }

    public int getJhsParticipantHandle() {
        return mParticipantId;
    }

    public void writeToParcel(Parcel outPar, int flag) {
        outPar.writeInt(mParticipantId);
    }

    public void readFromParcel(Parcel inPar) {
        mParticipantId = inPar.readInt();
    }

    public static final Parcelable.Creator<JhsParticipantHandle> CREATOR
            = new Parcelable.Creator<JhsParticipantHandle>() {
        @Override
        public JhsParticipantHandle createFromParcel(Parcel in) {
            int id = in.readInt();
            return new JhsParticipantHandle(id);
        }

        @Override
        public JhsParticipantHandle[] newArray(int size) {
            return new JhsParticipantHandle[size];
        }
    };
}
