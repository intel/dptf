/*
* Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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

package com.intel.dptf;

import android.app.Application;
import android.content.Intent;
import android.os.SystemProperties;
import android.util.Log;

import com.intel.dptf.jhs.JavaHelperService;

/*
 * jhsApp will start Java Helper Service
 *
 * @hide
 */
public class jhsApp extends Application {

    static final String TAG = "jhsApp";

    public jhsApp() {

    }

    @Override
    public void onCreate() {
        super.onCreate();

        // start Java Helper Service
        JavaHelperService.init(getApplicationContext());
        Log.w(TAG, "JHS enabled");

    }
}
