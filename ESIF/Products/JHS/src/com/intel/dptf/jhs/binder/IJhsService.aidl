package com.intel.dptf.jhs.binder;

import com.intel.dptf.jhs.binder.JhsParticipantHandle;
import com.intel.dptf.jhs.binder.ActionParameters;
import com.intel.dptf.jhs.binder.JhsReplyHeader;
import com.intel.dptf.jhs.binder.JhsReplyPayload;

interface IJhsService {
    int getValue(in JhsParticipantHandle dptfHand, in ActionParameters actionParams, out JhsReplyHeader jhsReplyHead, out JhsReplyPayload returnByteArray);
    int setValue(in JhsParticipantHandle dptfHand, in ActionParameters actionParams, in int value);
}
