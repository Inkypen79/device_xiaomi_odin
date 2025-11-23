/*
 * Copyright (C) 2015 The CyanogenMod Project
 *               2017-2023 The LineageOS Project
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

package org.lineageos.settings.doze;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import androidx.appcompat.app.AlertDialog;
import androidx.preference.ListPreference;
import androidx.preference.Preference;
import androidx.preference.Preference.OnPreferenceChangeListener;
import androidx.preference.PreferenceCategory;
import androidx.preference.SwitchPreferenceCompat;
import com.android.settingslib.widget.MainSwitchPreference;
import com.android.settingslib.widget.SettingsBasePreferenceFragment;
import org.lineageos.settings.R;
import org.lineageos.settings.utils.FileUtils;

public class DozeSettingsFragment
        extends SettingsBasePreferenceFragment implements OnPreferenceChangeListener {
    private SwitchPreferenceCompat mAlwaysOnDisplayPreference;
    private ListPreference mDozeBrightnessPreference;
    private SwitchPreferenceCompat mWakeOnGesturePreference;
    private SwitchPreferenceCompat mPickUpPreference;

    private Handler mHandler = new Handler();

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        setPreferencesFromResource(R.xml.doze_settings, rootKey);

        SharedPreferences prefs =
                getActivity().getSharedPreferences("doze_settings", Activity.MODE_PRIVATE);
        if (savedInstanceState == null && !prefs.getBoolean("first_help_shown", false)) {
            showHelp();
        }

        boolean dozeEnabled = DozeUtils.isDozeEnabled(getActivity());

        MainSwitchPreference switchBar = findPreference(DozeUtils.DOZE_ENABLE);
        switchBar.setOnPreferenceChangeListener(this);
        switchBar.setChecked(dozeEnabled);

        mAlwaysOnDisplayPreference =
                (SwitchPreferenceCompat) findPreference(DozeUtils.ALWAYS_ON_DISPLAY);
        mAlwaysOnDisplayPreference.setEnabled(dozeEnabled);
        mAlwaysOnDisplayPreference.setChecked(DozeUtils.isAlwaysOnEnabled(getActivity()));
        mAlwaysOnDisplayPreference.setOnPreferenceChangeListener(this);

        mDozeBrightnessPreference = (ListPreference) findPreference(DozeUtils.DOZE_BRIGHTNESS_KEY);
        mDozeBrightnessPreference.setEnabled(
                dozeEnabled && DozeUtils.isAlwaysOnEnabled(getActivity()));
        mDozeBrightnessPreference.setOnPreferenceChangeListener(this);

        mWakeOnGesturePreference =
                (SwitchPreferenceCompat) findPreference(DozeUtils.WAKE_ON_GESTURE_KEY);
        mWakeOnGesturePreference.setEnabled(dozeEnabled);
        mWakeOnGesturePreference.setOnPreferenceChangeListener(this);

        PreferenceCategory pickupSensorCategory =
                (PreferenceCategory) getPreferenceScreen().findPreference(
                        DozeUtils.CATEG_PICKUP_SENSOR);

        mPickUpPreference = (SwitchPreferenceCompat) findPreference(DozeUtils.GESTURE_PICK_UP_KEY);
        mPickUpPreference.setEnabled(dozeEnabled);
        mPickUpPreference.setOnPreferenceChangeListener(this);

        // Hide AOD and doze brightness if not supported and set all its dependents otherwise
        if (!DozeUtils.alwaysOnDisplayAvailable(getActivity())) {
            getPreferenceScreen().removePreference(mAlwaysOnDisplayPreference);
            getPreferenceScreen().removePreference(mDozeBrightnessPreference);
        } else {
            if (!FileUtils.isFileWritable(DozeUtils.DOZE_MODE_PATH)) {
                getPreferenceScreen().removePreference(mDozeBrightnessPreference);
            } else {
                DozeUtils.updateDozeBrightnessIcon(getContext(), mDozeBrightnessPreference);
            }
            mWakeOnGesturePreference.setDependency(DozeUtils.ALWAYS_ON_DISPLAY);
            pickupSensorCategory.setDependency(DozeUtils.ALWAYS_ON_DISPLAY);
        }
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        if (DozeUtils.DOZE_ENABLE.equals(preference.getKey())) {
            boolean isChecked = (Boolean) newValue;
            DozeUtils.enableDoze(getActivity(), isChecked);

            if (!isChecked) {
                DozeUtils.enableAlwaysOn(getActivity(), false);
                mAlwaysOnDisplayPreference.setChecked(false);
                mDozeBrightnessPreference.setValue(DozeUtils.DOZE_BRIGHTNESS_LBM);
                DozeUtils.updateDozeBrightnessIcon(getContext(), mDozeBrightnessPreference);
                mPickUpPreference.setChecked(false);
            }
            mAlwaysOnDisplayPreference.setEnabled(isChecked);
            mDozeBrightnessPreference.setEnabled(
                    isChecked && DozeUtils.isAlwaysOnEnabled(getActivity()));
            mWakeOnGesturePreference.setEnabled(isChecked);
            mPickUpPreference.setEnabled(isChecked);
        } else if (DozeUtils.ALWAYS_ON_DISPLAY.equals(preference.getKey())) {
            DozeUtils.enableAlwaysOn(getActivity(), (Boolean) newValue);
            if (!(Boolean) newValue) {
                mDozeBrightnessPreference.setValue(DozeUtils.DOZE_BRIGHTNESS_LBM);
                DozeUtils.setDozeMode(DozeUtils.DOZE_BRIGHTNESS_LBM);
            } else {
                mPickUpPreference.setChecked(false);
            }
            mDozeBrightnessPreference.setEnabled((Boolean) newValue);
        } else if (DozeUtils.DOZE_BRIGHTNESS_KEY.equals(preference.getKey())) {
            if (!DozeUtils.DOZE_BRIGHTNESS_AUTO.equals((String) newValue)) {
                DozeUtils.setDozeMode((String) newValue);
            }
        }

        mHandler.post(() -> {
            DozeUtils.checkDozeService(getActivity());
            DozeUtils.updateDozeBrightnessIcon(getContext(), mDozeBrightnessPreference);
        });

        return true;
    }

    private void showHelp() {
        AlertDialog helpDialog = new AlertDialog.Builder(getActivity())
                                         .setTitle(R.string.doze_settings_help_title)
                                         .setMessage(R.string.doze_settings_help_text)
                                         .setPositiveButton(R.string.dialog_ok,
                                                 (dialog, which) -> {
                                                     getActivity()
                                                             .getSharedPreferences("doze_settings",
                                                                     Activity.MODE_PRIVATE)
                                                             .edit()
                                                             .putBoolean("first_help_shown", true)
                                                             .commit();
                                                     dialog.cancel();
                                                 })
                                         .create();
        helpDialog.show();
    }
}
