////////////////////////////////////////////////////////////////////////////////
///
/// LESoundEffectsExampleActivity.java
/// ----------------------------------
///
/// LE example app contents (not to be confused with the official SDK API).
///
/// Copyright (c) 2015 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////

package demo.littleendiandemo.app;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.ViewSwitcher;

import java.io.IOException;

// http://webdesign.tutsplus.com/articles/a-tale-of-two-platforms-designing-for-both-android-and-ios--cms-23616

public class LESoundEffectsExampleActivity
        extends Activity
        implements View.OnClickListener,
        SeekBar.OnSeekBarChangeListener {
    //region Activity & View events
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.example);
        message = (TextView) findViewById(R.id.message);

        viewContainer = (ViewSwitcher) findViewById(R.id.viewContainer);
        presetList = (Spinner) findViewById(R.id.presetList);
        viewSwitcher = (Button) findViewById(R.id.viewSwitcher);
        viewSwitcher.setOnClickListener(this);
        stopRendering = (Button) findViewById(R.id.stop);
        stopRendering.setOnClickListener(this);
        renderWithFile = (Button) findViewById(R.id.renderFile);
        renderWithFile.setOnClickListener(this);
        renderWithMicrophone = (Button) findViewById(R.id.renderMicrophone);
        renderWithMicrophone.setOnClickListener(this);
        activityIndicator = (ProgressBar) findViewById(R.id.activityIndicator);

        basicView = (RelativeLayout) findViewById(R.id.basicView);
        advancedViewControls = (LinearLayout) findViewById(R.id.advancedViewControls);
        advancedViewInstruction = (TextView) findViewById(R.id.instructionAdvanced2);

        // Fill preset list with the presets in the app's assets
        try {
            ArrayAdapter<String> presetListAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, getAssets().list("presets"));
            presetListAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            presetList.setAdapter(presetListAdapter);
        } catch (IOException e) {
            Log.e("LE SW SoundEffects SDK example failure", Log.getStackTraceString(e));
            return;
        }

        try {
            System.loadLibrary("app");
            initialise(LESoundEffectsExampleActivity.this.getResources().getAssets());
        } catch (RuntimeException e) {
            Log.e("LE SW SoundEffects SDK example failure", "Error loading native library " + e);
            Log.e("LE SW SoundEffects SDK example failure", Log.getStackTraceString(e));
            return;
        }
    }

    @Override
    public void onClick(View button) {
        stopAllRendering();
        stopGUI();

        if (button == viewSwitcher) {
            viewContainer.showNext();
        } else if (button != stopRendering) {
            activityIndicator.setVisibility(View.VISIBLE);
            if (viewContainer.getCurrentView() == basicView) {
                String preset = "presets/" + (String) presetList.getSelectedItem();
                if (button == renderWithFile)
                    renderPresetWithFileRealtime(preset);
                else {
                    assert button == renderWithMicrophone;
                    renderPresetWithLiveInput(preset);
                }
            } else {
                if (button == renderWithFile) advancedFileRender();
                else advancedLiveInputRender();
            }
            startProfilerTimer();
        }
    }
    //endregion

    //region Slider events
    @Override
    public void onProgressChanged(SeekBar seekBar, int position, boolean fromUser) {
        ParameterControl control = (ParameterControl) seekBar;
        float newValue = updateParameter(control.moduleNativePtr, control.parameterIndex, position);
        message.setText(String.format("%s::%s %.1f%s", control.effectName, control.leSDKParameterName, newValue, control.parameterUnit));
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
        touchInProgress = true;
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        touchInProgress = false;
        message.setText("");
    }
    //endregion Slider events

    //region Private helpers
    private void stopGUI() {
        activityIndicator.setVisibility(View.INVISIBLE);
        stopProfilerTimer();
        message.setText("");
        advancedViewControls.removeAllViews();
        advancedViewInstruction.setVisibility(View.VISIBLE);
    }
    //endregion Private helpers

    //region Native<->Java interop

    //region Native->Java

    public void userMessage(final String newMessage) {
        if (touchInProgress)
            return;
        runOnUiThread(new Runnable() {
            public void run() {
                message.setText(newMessage);
            }
        });
    }

    private void processingStopped() {
        runOnUiThread(new Runnable() {
            public void run() {
                stopGUI();
            }
        });
    }

    private void addParameterWidget
            (
                    String effectName,
                    String uiParameterName,
                    String leSDKParameterName,
                    String parameterUnit,
                    long moduleNativePtr,
                    int parameterIndex,
                    int currentValuePercentage
            ) {
        advancedViewInstruction.setVisibility(View.INVISIBLE);

        ParameterControl control = new ParameterControl(this);

        control.setMax(100);

        control.effectName = effectName;
        control.leSDKParameterName = leSDKParameterName;
        control.parameterUnit = parameterUnit;
        control.moduleNativePtr = moduleNativePtr;
        control.parameterIndex = parameterIndex;

        control.uiParameterName.setText(uiParameterName);
        control.setProgress(currentValuePercentage);

        advancedViewControls.addView(control.uiParameterName);
        advancedViewControls.addView(control);

        control.setOnSeekBarChangeListener(this);
        control.setVisibility(View.VISIBLE);
    }

    //endregion Native->Java

    //region Java->Native

    private native void initialise(AssetManager assetManager);

    private native void renderPresetWithFileRealtime(String preset);

    private native void renderPresetWithLiveInput(String preset);

    private native void advancedLiveInputRender();

    private native void advancedFileRender();

    private native void stopAllRendering();

    private native float updateParameter(long modulePtr, int parameterIndex, int newValuePercentage);

    private native float getCPUUsage();

    private native boolean isReleaseBuild();

    //endregion Java->Native

    //endregion Native<->Java interop

    //region Profiler

    private void startProfilerTimer() {
        // Do profiling only in release builds...
        if (isReleaseBuild()) {
            timer = new Handler();
            profilerUpdater = new Runnable() {
                @Override
                public void run() {
                    userMessage(String.format("CPU %.1f%%", getCPUUsage()));
                    timer.postDelayed(this, 1000);
                }
            };
            timer.postDelayed(profilerUpdater, 3000);
        }
    }

    private void stopProfilerTimer() {
        if (timer == null)
            return;
        timer.removeCallbacks(profilerUpdater);
        timer = null;
        profilerUpdater = null;
    }

    Handler timer;
    Runnable profilerUpdater;

    //endregion Profiler

    //region Widgets

    private class ParameterControl extends SeekBar {
        public ParameterControl(final Context context) {
            super(context);
            uiParameterName = new TextView(context);
        }

        TextView uiParameterName;

        String effectName;
        String leSDKParameterName;
        String parameterUnit;
        long moduleNativePtr;
        int parameterIndex;
    }

    private ViewSwitcher viewContainer;
    private Button viewSwitcher;
    private Button stopRendering;
    private Button renderWithFile;
    private Button renderWithMicrophone;
    private Spinner presetList;
    private TextView message;
    private ProgressBar activityIndicator;
    private boolean touchInProgress;

    private RelativeLayout basicView;
    private LinearLayout advancedViewControls;
    private TextView advancedViewInstruction;

    //endregion Widgets
}
