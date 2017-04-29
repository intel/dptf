/*
* Copyright (c) 2015 Intel Corporation All Rights Reserved
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

public class DptfParticipantDeviceType implements Parcelable {
    int mDptfPartDeviceType;
/*
    DEVICE_PTYP_WIFI = 7,
    DEVICE_PTYP_WWAN = 15
*/
    public DptfParticipantDeviceType() {
    }

    public DptfParticipantDeviceType(int dptfPartDeviceType) {
        mDptfPartDeviceType = dptfPartDeviceType;
    }

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel outPar, int flag) {
        outPar.writeInt(mDptfPartDeviceType);
    }

    public void readFromParcel(Parcel inPar) {
        mDptfPartDeviceType = inPar.readInt();
    }

    public void setDptfPartDeviceType(int deviceType) {
        mDptfPartDeviceType = deviceType;
    }

    public int getDptfPartDeviceType() {
        return mDptfPartDeviceType;
    }

    public static final Parcelable.Creator<DptfParticipantDeviceType> CREATOR
            = new Parcelable.Creator<DptfParticipantDeviceType>() {
        @Override
        public DptfParticipantDeviceType createFromParcel(Parcel in) {
            int deviceType = in.readInt();
            return new DptfParticipantDeviceType(deviceType);
        }

        @Override
        public DptfParticipantDeviceType[] newArray(int size) {
            return new DptfParticipantDeviceType[size];
        }
    };
}

