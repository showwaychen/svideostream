package cn.cxw.svideostream.fragments;

import android.os.Bundle;
import android.support.v7.preference.PreferenceFragmentCompat;

import cn.cxw.svideostream.R;

/**
 * Created by cxw on 2017/11/2.
 */

public class SettingsFragment extends PreferenceFragmentCompat {

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        addPreferencesFromResource(R.xml.preferences);

    }
}
