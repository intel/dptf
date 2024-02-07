/*
* Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

public class DptfParticipant implements Parcelable {
    String mParticipantName;

    public DptfParticipant (String name) {
        mParticipantName = name;
    }

    public int describeContents() {
        return 0;
    }

    public void setDptfParticipantName(String partName) {
        mParticipantName = partName;
    }

    public String getDptfParticipantName() {
        return mParticipantName;
    }

    public void writeToParcel(Parcel outPar, int flag) {
        outPar.writeString(mParticipantName);
    }

    public void readFromParcel(Parcel inPar) {
        mParticipantName = inPar.readString();
    }

    public static final Parcelable.Creator<DptfParticipant> CREATOR
            = new Parcelable.Creator<DptfParticipant>() {
        @Override
        public DptfParticipant createFromParcel(Parcel in) {
            String name = in.readString();
            return new DptfParticipant(name);
        }

        @Override
        public DptfParticipant[] newArray(int size) {
            return new DptfParticipant[size];
        }
    };
}
