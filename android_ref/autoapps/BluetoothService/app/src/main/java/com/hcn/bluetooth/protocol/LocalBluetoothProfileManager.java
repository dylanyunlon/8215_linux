/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.hcn.bluetooth.protocol;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import android.bluetooth.BluetoothA2dp;
import android.bluetooth.BluetoothA2dpSink;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothHeadset;
import android.bluetooth.BluetoothHeadsetClient;
import android.bluetooth.BluetoothPan;
import android.bluetooth.BluetoothPbapClient;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.BluetoothUuid;
import android.content.Context;
import android.content.Intent;
import android.os.ParcelUuid;
import android.util.ArraySet;
import android.util.Log;
/**
 * LocalBluetoothProfileManager provides access to the LocalBluetoothProfile
 * objects for the available Bluetooth profiles.
 */
public  class LocalBluetoothProfileManager {
    private static final String TAG = "Car_LocalBluetoothProfileManager";
    private static final boolean DEBUG = true;

    public static final String ACTION_PROFILE_UPDATE ="com.mtk.bluetooth.common.profiles_updater";
    private static final String BLUETOOTH_PERM = android.Manifest.permission.BLUETOOTH;
    // private static Set<Profile> mConenctedProfileList;

    /** Used when obtaining a reference to the singleton instance. */
    private static Object INSTANCE_LOCK = new Object();

    /**
     * An interface for notifying BluetoothHeadset IPC clients when they have
     * been connected to the BluetoothHeadset service.
     * Only used by com.android.settings.bluetooth.DockService.
     */
    public interface ServiceListener {
        /**
         * Called to notify the client when this proxy object has been
         * connected to the BluetoothHeadset service. Clients must wait for
         * this callback before making IPC calls on the BluetoothHeadset
         * service.
         */
        void onServiceConnected();

        /**
         * Called to notify the client that this proxy object has been
         * disconnected from the BluetoothHeadset service. Clients must not
         * make IPC calls on the BluetoothHeadset service after this callback.
         * This callback will currently only occur if the application hosting
         * the BluetoothHeadset service, but may be called more often in future.
         */
        void onServiceDisconnected();
    }

    private final Context mContext;
    private final LocalBluetoothAdapter mLocalAdapter;
    private final CachedBluetoothDeviceManager mDeviceManager;
    private final BluetoothEventManager mEventManager;

    private A2dpProfile mA2dpProfile;
    /**
     * Add for A2dpSink
     */
    private A2dpSinkProfile mA2dpSinkProfile;
    private HeadsetProfile mHeadsetProfile;
    private HeadsetClientProfile mHeadsetClientProfile;
    //private OppProfile mOppProfile;
    private PbapClientProfile mPbapClientProfile;
    private PbapServerProfile mPbapProfile;
    private final boolean mUsePbapPce;

    /**
     * Add for Pan
     */
    private PanProfile mPanProfile;

    /**
     * Mapping from profile name, e.g. "HEADSET" to profile object.
     */
    private final Map<String, LocalBluetoothProfile>
            mProfileNameMap = new HashMap<String, LocalBluetoothProfile>();

    LocalBluetoothProfileManager(Context context,
            LocalBluetoothAdapter adapter,
            CachedBluetoothDeviceManager deviceManager,
            BluetoothEventManager eventManager) {
        mContext = context;

        mLocalAdapter = adapter;
        mDeviceManager = deviceManager;
        mEventManager = eventManager;
        mUsePbapPce = true;//mContext.getResources().getBoolean(R.bool.enable_pbap_pce_profile);
        // pass this reference to adapter and event manager (circular dependency)
        mLocalAdapter.setProfileManager(this);
        mEventManager.setProfileManager(this);

        ParcelUuid[] uuids = adapter.getUuids();

        // uuids may be null if Bluetooth is turned off
        if (uuids != null) {
            updateLocalProfiles(uuids);
        }

       
        mPanProfile = new PanProfile(context);
        addPanProfile(mPanProfile, PanProfile.NAME,
                BluetoothPan.ACTION_CONNECTION_STATE_CHANGED);

       //Create PBAP server profile, but do not add it to list of profiles
       // as we do not need to monitor the profile as part of profile list
        mPbapProfile = new PbapServerProfile(context);

        if (DEBUG) Log.d(TAG, "LocalBluetoothProfileManager construction complete");
    }

    /**
     * Initialize or update the local profile objects. If a UUID was previously
     * present but has been removed, we print a warning but don't remove the
     * profile object as it might be referenced elsewhere, or the UUID might
     * come back and we don't want multiple copies of the profile objects.
     *
     * @param uuids
     */
    void updateLocalProfiles(ParcelUuid[] uuids) {
        // A2DP SRC
        if (BluetoothUuid.isUuidPresent(uuids, BluetoothUuid.AudioSource)) {
            if (mA2dpProfile == null) {
                if(DEBUG) Log.d(TAG, "Adding local A2DP SRC profile");
                mA2dpProfile = new A2dpProfile(mContext, mLocalAdapter, mDeviceManager, this);
                addProfile(mA2dpProfile, A2dpProfile.NAME,
                        BluetoothA2dp.ACTION_CONNECTION_STATE_CHANGED);
            }
        } else if (mA2dpProfile != null) {
            Log.w(TAG, "Warning: A2DP profile was previously added but the UUID is now missing.");
        }

        // A2DP SINK
        if (BluetoothUuid.isUuidPresent(uuids, BluetoothUuid.AudioSink)) {
            if (mA2dpSinkProfile == null) {
                if(DEBUG) Log.d(TAG, "Adding local A2DP Sink profile");
                mA2dpSinkProfile = new A2dpSinkProfile(mContext, mLocalAdapter, mDeviceManager, this);
                addProfile(mA2dpSinkProfile, A2dpSinkProfile.NAME,
                        BluetoothA2dpSink.ACTION_CONNECTION_STATE_CHANGED);
            }
        } else if (mA2dpSinkProfile != null) {
            Log.w(TAG, "Warning: A2DP Sink profile was previously added but the UUID is now missing.");
        }

        // Headset / Handsfree
        if (BluetoothUuid.isUuidPresent(uuids, BluetoothUuid.Handsfree_AG) ||
            BluetoothUuid.isUuidPresent(uuids, BluetoothUuid.HSP_AG)) {
            if (mHeadsetProfile == null) {
                if (DEBUG) Log.d(TAG, "Adding local HEADSET profile");
                mHeadsetProfile = new HeadsetProfile(mContext, mLocalAdapter,
                        mDeviceManager, this);
                addProfile(mHeadsetProfile, HeadsetProfile.NAME,
                        BluetoothHeadset.ACTION_CONNECTION_STATE_CHANGED);
            }
        } else if (mHeadsetProfile != null) {
            Log.w(TAG, "Warning: HEADSET profile was previously added but the UUID is now missing.");
        }

        // Headset HF
        if (BluetoothUuid.isUuidPresent(uuids, BluetoothUuid.Handsfree)) {
            if (mHeadsetClientProfile == null) {
                if (DEBUG) Log.d(TAG, "Adding local HEADSET CLIENT profile");
                    mHeadsetClientProfile = new HeadsetClientProfile(mContext, mLocalAdapter,
                            mDeviceManager, this);
                    addProfile(mHeadsetClientProfile, HeadsetClientProfile.NAME,
                            BluetoothHeadsetClient.ACTION_CONNECTION_STATE_CHANGED);
            }
        } else if (mHeadsetClientProfile != null) {
                Log.w(TAG, "Warning: HEADSET CLIENT profile was previously added but the UUID is now missing.");
        } else {
            Log.d(TAG, "Handsfree Uuid not found.");
        }
        // OPP

        //PBAP Client
        if (mUsePbapPce) {
            if (mPbapClientProfile == null) {
                if(DEBUG) Log.d(TAG, "Adding local PBAP Client profile");
                mPbapClientProfile = new PbapClientProfile(mContext, mLocalAdapter, mDeviceManager,
                        this);
                addProfile(mPbapClientProfile, PbapClientProfile.NAME,
                        BluetoothPbapClient.ACTION_CONNECTION_STATE_CHANGED);
            }
        } else if (mPbapClientProfile != null) {
            Log.w(TAG,
                "Warning: PBAP Client profile was previously added but the UUID is now missing.");
        }
		  
		  mEventManager.registerProfileIntentReceiver();
	
		  // There is no local SDP record for HID and Settings app doesn't control PBAP
	  }

    private final Collection<ServiceListener> mServiceListeners =
            new ArrayList<ServiceListener>();

    private void addProfile(LocalBluetoothProfile profile,
            String profileName, String stateChangedAction) {
        mEventManager.addProfileHandler(stateChangedAction, new StateChangedHandler(profile));
        mProfileNameMap.put(profileName, profile);
    }

    private void addPanProfile(LocalBluetoothProfile profile,
                               String profileName, String stateChangedAction) {
        mEventManager.addProfileHandler(stateChangedAction,
                new PanStateChangedHandler(profile));
        mProfileNameMap.put(profileName, profile);
    }

    public LocalBluetoothProfile getProfileByName(String name) {
        return mProfileNameMap.get(name);
    }

    // Called from LocalBluetoothAdapter when state changes to ON
    void setBluetoothStateOn() {
        ParcelUuid[] uuids = mLocalAdapter.getUuids();
        if (uuids != null) {
            updateLocalProfiles(uuids);
        }
        mEventManager.readPairedDevices();
    }

    /**
     * Generic handler for connection state change events for the specified profile.
     */
    private class StateChangedHandler implements BluetoothEventManager.Handler {
        final LocalBluetoothProfile mProfile;

        StateChangedHandler(LocalBluetoothProfile profile) {
            mProfile = profile;
        }

        public void onReceive(Context context, Intent intent, BluetoothDevice device) {
            if (null == device) {
                return;
            }
            CachedBluetoothDevice cachedDevice = mDeviceManager.findDevice(device);
            Log.d(TAG,"StateChangedHandler cachedDevice =="+cachedDevice);
            if (cachedDevice == null) {
                Log.w(TAG, "StateChangedHandler found new device: " + device);
                cachedDevice = mDeviceManager.addDevice(mLocalAdapter,
                        LocalBluetoothProfileManager.this, device);
            }
            int newState = intent.getIntExtra(BluetoothProfile.EXTRA_STATE, 0);
            int oldState = intent.getIntExtra(BluetoothProfile.EXTRA_PREVIOUS_STATE, 0);
            if (newState == BluetoothProfile.STATE_DISCONNECTED &&
                    oldState == BluetoothProfile.STATE_CONNECTING) {
                Log.i(TAG, "Failed to connect " + mProfile + " device");
            }

            cachedDevice.onProfileStateChanged(mProfile, newState);
            cachedDevice.refresh();
        }
    }

    /** State change handler for NAP and PANU profiles. */
    private class PanStateChangedHandler extends StateChangedHandler {

        PanStateChangedHandler(LocalBluetoothProfile profile) {
            super(profile);
        }

        @Override
        public void onReceive(Context context, Intent intent, BluetoothDevice device) {
            PanProfile panProfile = (PanProfile) mProfile;
            int role = intent.getIntExtra(BluetoothPan.EXTRA_LOCAL_ROLE, 0);
            panProfile.setLocalRole(device, role);
            Log.d(TAG, "pan profile state change, role is " + role);
            super.onReceive(context, intent, device);
        }
    }

    // called from DockService
    public void addServiceListener(ServiceListener l) {
        mServiceListeners.add(l);
    }

    // called from DockService
    public void removeServiceListener(ServiceListener l) {
        mServiceListeners.remove(l);
    }

    // not synchronized: use only from UI thread! (TODO: verify)
    void callServiceConnectedListeners() {
        for (ServiceListener l : mServiceListeners) {
            l.onServiceConnected();
        }
    }

    // not synchronized: use only from UI thread! (TODO: verify)
    void callServiceDisconnectedListeners() {
        for (ServiceListener listener : mServiceListeners) {
            listener.onServiceDisconnected();
        }
    }

    // This is called by DockService, so check Headset and A2DP.
    public synchronized boolean isManagerReady() {
        // Getting just the headset profile is fine for now. Will need to deal with A2DP
        // and others if they aren't always in a ready state.
        LocalBluetoothProfile profile = mHeadsetProfile;
        if (profile != null) {
            return profile.isProfileReady();
        }
        profile = mA2dpProfile;
        if (profile != null) {
            return profile.isProfileReady();
        }
        profile = mA2dpSinkProfile;
        if (profile != null) {
            return profile.isProfileReady();
        }
        return false;
    }

    public A2dpProfile getA2dpProfile() {
        Log.d(TAG, "getA2dpProfile, mA2dpProfile = " + mA2dpProfile);
        return mA2dpProfile;
    }

    public PanProfile getPanProfile() {
        Log.d(TAG, "getPanProfile, mPanProfile = " + mPanProfile);
        return mPanProfile;
    }


    /**
     * Add for A2dpSink
     */
    public A2dpSinkProfile getA2dpSinkProfile() {
        Log.d(TAG, "getA2dpSinkProfile, mA2dpSinkProfile = " + mA2dpSinkProfile);
        if ((mA2dpSinkProfile != null) && (mA2dpSinkProfile.isProfileReady())) {
            return mA2dpSinkProfile;
        } else {
            return null;
        }
    }

    public HeadsetProfile getHeadsetProfile() {
        Log.d(TAG, "getHeadsetProfile, mHeadsetProfile = " + mHeadsetProfile);
        return mHeadsetProfile;
    }

	public HeadsetClientProfile getHeadsetClientProfile() {
        if ((mHeadsetClientProfile != null) && (mHeadsetClientProfile.isProfileReady())) {
            return mHeadsetClientProfile;
        } else {
          return null;
        }
    }

    public PbapClientProfile getPbapClientProfile() {
        return mPbapClientProfile;
    }

    public PbapServerProfile getPbapProfile() {
        Log.d(TAG, "getPbapProfile, mPbapProfile = " + mPbapProfile);
        return mPbapProfile;
    }

    /**
     * Fill in a list of LocalBluetoothProfile objects that are supported by
     * the local device and the remote device.
     *
     * @param uuids of the remote device
     * @param localUuids UUIDs of the local device
     * @param profiles The list of profiles to fill
     * @param removedProfiles list of profiles that were removed
     */
    synchronized void updateProfiles(ParcelUuid[] uuids, ParcelUuid[] localUuids,
            Collection<LocalBluetoothProfile> profiles,
            Collection<LocalBluetoothProfile> removedProfiles,
            boolean isPanNapConnected, BluetoothDevice device) {
        // Copy previous profile list into removedProfiles
        removedProfiles.clear();
        removedProfiles.addAll(profiles);
        if (DEBUG) {
            Log.d(TAG,"Current Profiles" + profiles.toString());
        }
        profiles.clear();

        if (uuids == null) {
            Log.d(TAG, "remote device uuid is null");
            return;
        }

        if (mHeadsetProfile != null) {
            if ((BluetoothUuid.isUuidPresent(localUuids, BluetoothUuid.HSP_AG) &&
                 BluetoothUuid.isUuidPresent(uuids, BluetoothUuid.HSP)) ||
                 (BluetoothUuid.isUuidPresent(localUuids, BluetoothUuid.Handsfree_AG) &&
                  BluetoothUuid.isUuidPresent(uuids, BluetoothUuid.Handsfree))) {
                Log.d(TAG, "Add HeadsetProfile to connectable profile list");
                profiles.add(mHeadsetProfile);
                removedProfiles.remove(mHeadsetProfile);
             }
        }

        if ((mHeadsetClientProfile != null) &&
                BluetoothUuid.isUuidPresent(uuids, BluetoothUuid.Handsfree_AG) &&
                BluetoothUuid.isUuidPresent(localUuids, BluetoothUuid.Handsfree)) {
            profiles.add(mHeadsetClientProfile);
            removedProfiles.remove(mHeadsetClientProfile);
        }

        if (BluetoothUuid.containsAnyUuid(uuids, A2dpProfile.SINK_UUIDS) &&
            mA2dpProfile != null) {
            Log.d(TAG, "Add A2dpProfile to connectable profile list");
            profiles.add(mA2dpProfile);
            removedProfiles.remove(mA2dpProfile);
        }


          /// Add for A2dpSink
        if (BluetoothUuid.containsAnyUuid(uuids, A2dpSinkProfile.SRC_UUIDS) &&
                mA2dpSinkProfile != null) {
            Log.d(TAG, "[updateProfiles] remote device support source, add the a2dp sink profile");
            Log.d(TAG, "[updateProfiles] contains the SINK UUID");
                profiles.add(mA2dpSinkProfile);
                removedProfiles.remove(mA2dpSinkProfile);
        }

        if(isPanNapConnected)
            if(DEBUG) Log.d(TAG, "Valid PAN-NAP connection exists.");
        if ((BluetoothUuid.isUuidPresent(uuids, BluetoothUuid.NAP) &&
            mPanProfile != null) || isPanNapConnected) {
            profiles.add(mPanProfile);
            removedProfiles.remove(mPanProfile);
        }
        if (BluetoothUuid.isUuidPresent(uuids, BluetoothUuid.PBAP_PSE) &&
            mPbapProfile != null) {
            Log.d(TAG, "Add PbapProfile to connectable profile list");
            profiles.add(mPbapProfile);
            removedProfiles.remove(mPbapProfile);
            if (mUsePbapPce) {
                profiles.add(mPbapClientProfile);
                removedProfiles.remove(mPbapClientProfile);
                profiles.remove(mPbapProfile);
                removedProfiles.add(mPbapProfile);
            }
        }

        Log.d(TAG, "updateProfiles end profiles" + profiles );
    }

    private void log(String info) {
        if (true) {
            Log.v(TAG, "[BT][profile manager]" + info);
        }
    }
    /* MTK Added : End */

    public boolean isBusy() {
        int[] state = new int[]{
                BluetoothProfile.STATE_CONNECTING,
                BluetoothProfile.STATE_DISCONNECTING};
        if (null != mHeadsetClientProfile) {
            List<BluetoothDevice> devices =
                    mHeadsetClientProfile.getDevicesMatchingConnectionStates(state);
            if (!devices.isEmpty()) {
                return true;
            }
        }
        if (null != mA2dpSinkProfile) {
            List<BluetoothDevice> devices = mA2dpSinkProfile.getDevicesMatchingConnectionStates(
                    state);
            if (!devices.isEmpty()) {
                return true;
            }
        }
        if (null != mPbapClientProfile) {
            List<BluetoothDevice> devices = mPbapClientProfile.getDevicesMatchingConnectionStates(
                    state);
            if (!devices.isEmpty()) {
                return true;
            }
        }
        return false;
    }

    public ArraySet<BluetoothDevice> getConnectedDevices() {
        ArraySet<BluetoothDevice> set = new ArraySet<>();
        int[] state = new int[]{BluetoothProfile.STATE_CONNECTED};
        if(mHeadsetClientProfile == null){
            return set;
        }
        List<BluetoothDevice> devices = mHeadsetClientProfile.getDevicesMatchingConnectionStates(
                state);
        set.addAll(devices);
        devices = mA2dpSinkProfile.getDevicesMatchingConnectionStates(state);
        set.addAll(devices);
        devices = mPbapClientProfile.getDevicesMatchingConnectionStates(state);
        set.addAll(devices);
        return set;
    }
}
