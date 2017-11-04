package cn.cxw.svideostream.activities;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v7.app.AppCompatActivity;

import cn.cxw.svideostream.R;
import cn.cxw.svideostream.fragments.SettingsFragment;

/**
 * Created by cxw on 2017/11/2.
 */

public class SettingActivity extends AppCompatActivity implements SharedPreferences.OnSharedPreferenceChangeListener {
    private Fragment settingsFragment;
    private String keyprefVideoEncoderType;
    private String keyprefAudioEnable;
    static public void showActivity(Context context)
    {
        Intent intent = new Intent(context, SettingActivity.class);
        context.startActivity(intent);
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        keyprefVideoEncoderType = getString(R.string.pref_key_videoencodertype);
        keyprefAudioEnable  = getString(R.string.pref_key_audioenable);
        settingsFragment = new SettingsFragment();
        getSupportFragmentManager().beginTransaction()
                .replace(android.R.id.content, settingsFragment)
                .commit();
    }
//    private void updateSummary(SharedPreferences sharedPreferences, String key) {
//        Preference updatedPref = settingsFragment.findPreference(key);
        // Set summary to be the user-description for the selected value
//        updatedPref.setSummary(sharedPreferences.getString(key, ""));
//    }
//    private void updateSummaryVideoEncoderType(SharedPreferences sharedPreferences, String key) {
//        Preference updatedPref = settingsFragment.findPreference(key);
//         Set summary to be the user-description for the selected value
//        updatedPref.setSummary((sharedPreferences.getString(key, "0").compareTo("0") == 0 )?getString(R.string.videoencodertype_X264):getString(R.string.videoencodertype_MediaCodec));
//    }
    @Override
    protected void onResume() {
        super.onResume();
//        SharedPreferences sharedPreferences =
//                settingsFragment.getPreferenceScreen().getSharedPreferences();
//        sharedPreferences.registerOnSharedPreferenceChangeListener(this);
//        updateSummaryVideoEncoderType(sharedPreferences, keyprefVideoEncoderType);

    }

    @Override
    public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
//        if (key.equals(keyprefVideoEncoderType))
//        {
//            updateSummaryVideoEncoderType(sharedPreferences, keyprefVideoEncoderType);
//        }
    }

    @Override
    protected void onPause() {
        super.onPause();
//        SharedPreferences sharedPreferences =
//                settingsFragment.getPreferenceScreen().getSharedPreferences();
//        sharedPreferences.unregisterOnSharedPreferenceChangeListener(this);
    }
}
