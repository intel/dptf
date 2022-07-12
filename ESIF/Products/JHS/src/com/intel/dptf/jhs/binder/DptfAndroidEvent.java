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

public class DptfAndroidEvent implements Parcelable {
    int mEventType;
    int mEventParticipant;
    int mEventSubType;
    int mEventParam;

    public DptfAndroidEvent() {
    }

    public DptfAndroidEvent(int type, int participant, int subType, int eventParam) {
        mEventType = type;
        mEventParticipant = participant;
        mEventSubType = subType;
        mEventParam = eventParam;
    }

    public int describeContents() {
        return 0;
    }

    public void setEventType(int type) {
        mEventType = type;
    }

    public void setParticipant(int participant) {
        mEventParticipant = participant;
    }

    public void setSubEventType(int subType) {
        mEventSubType = subType;
    }

    public void setEventParam(int eventParam) {
        mEventParam = eventParam;
    }

    public void writeToParcel(Parcel outPar, int flag) {
        outPar.writeInt(mEventType);
        outPar.writeInt(mEventParticipant);
        outPar.writeInt(mEventSubType);
        outPar.writeInt(mEventParam);
    }

    public void readFromParcel(Parcel inPar) {
        mEventType = inPar.readInt();
        mEventParticipant = inPar.readInt();
        mEventSubType = inPar.readInt();
        mEventParam = inPar.readInt();
    }

    public static final Parcelable.Creator<DptfAndroidEvent> CREATOR
            = new Parcelable.Creator<DptfAndroidEvent>() {
        @Override
        public DptfAndroidEvent createFromParcel(Parcel in) {
            int type = in.readInt();
            int participant = in.readInt();
            int subType = in.readInt();
            int eventParam = in.readInt();
            return new DptfAndroidEvent(type, participant, subType, eventParam);
        }

        @Override
        public DptfAndroidEvent[] newArray(int size) {
            return new DptfAndroidEvent[size];
        }
    };
}
