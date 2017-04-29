package com.intel.dptf.jhs.binder;

import com.intel.dptf.jhs.binder.DptfAndroidEvent;
import com.intel.dptf.jhs.binder.JhsParticipantHandle;
import com.intel.dptf.jhs.binder.DptfParticipant;
import com.intel.dptf.jhs.binder.DptfParticipantDeviceType;

interface IJhsClientService {
    int participantActive(in DptfParticipant dptfPart, in DptfParticipantDeviceType dptfPartDeviceType);
    int participantInactive(in JhsParticipantHandle dptfHand);
    int sendEvent(in DptfAndroidEvent dptfEvent);
}
