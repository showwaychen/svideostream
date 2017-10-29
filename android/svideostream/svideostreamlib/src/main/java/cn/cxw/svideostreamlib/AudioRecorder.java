package cn.cxw.svideostreamlib;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.util.Log;

/**
 * Created by cxw on 2017/10/17.
 */

public class AudioRecorder {
    private static final String TAG = "AudioRecorder";
    private int source = MediaRecorder.AudioSource.MIC;
    private int format = AudioFormat.ENCODING_PCM_16BIT;
    private int channel = AudioFormat.CHANNEL_IN_MONO;
    private int sampleRate = 44100;
    private int readBufferSize = 2048;
    private int minBufferSize;
    private int cacheBufferSize;
    private AudioRecord audioRecord;

    private boolean isRecord;
    private boolean isAudioRecordRun;
    public boolean isMute;
    private AudioRecordThread thread;
    private Listener listener;

    public interface Listener {
        void onOutputAudioBuffer(byte[] buffer, int size);
    }

    public AudioRecorder(Listener listener) {
        this.listener = listener;
        isRecord = false;
        isAudioRecordRun = false;
        isMute = false;
    }

    public boolean isRecord() {
        return isRecord;
    }

    public int getSampleRate() {
        return sampleRate;
    }

    public boolean start() {
        Log.i(TAG, "AudioRecorder start.");
        isRecord = false;
        if (!waitLastAudioRecordStop()) {
            return false;
        }
        isRecord = true;
        thread = new AudioRecordThread();
        thread.start();
        return true;
    }

    public void stop() {
        Log.i(TAG, "AudioRecorder stop.");
        isRecord = false;
    }

    private boolean waitLastAudioRecordStop() {
        int waitTimes = 0;
        while (isAudioRecordRun) {
            Log.i(TAG, "Wait last AudioRecord stop.");
            if (++waitTimes > 100) {
                Log.e(TAG, "Last AudioRecord not stop now.");
                return false;
            }

            try {
                Thread.sleep(50);
            } catch (Exception e) {
                Log.e(TAG, "AudioRecord thread wait failed.");
                return false;
            }
        }
        return true;
    }

    private class AudioRecordThread extends Thread {

        @Override
        public void run()
        {
            Log.i(TAG, "AudioRecord thread start.");
            isAudioRecordRun = true;
            try {
                minBufferSize = AudioRecord.getMinBufferSize(sampleRate, channel, format);
                cacheBufferSize = (readBufferSize / minBufferSize + 3) * minBufferSize;
                Log.i(TAG, "minBufferSize = " + minBufferSize + ", cacheBufferSize = " + cacheBufferSize);
                audioRecord = new AudioRecord(source, sampleRate, channel, format, cacheBufferSize);
                audioRecord.startRecording();
            } catch (Exception e) {
                Log.e(TAG, "AudioRecord start failed.");
                isAudioRecordRun = false;
                return;
            }

            byte[] muteBuffer = new byte[readBufferSize];
            byte[] readBuffer = new byte[readBufferSize];
            while (isRecord) {
                int readSize = audioRecord.read(readBuffer, 0, readBufferSize);
                if (readSize > 0) {
                    if (isMute) {
                        listener.onOutputAudioBuffer(muteBuffer, readBuffer.length);
                    } else {
                        listener.onOutputAudioBuffer(readBuffer, readBuffer.length);
                    }
                }
            }
            try {
                audioRecord.stop();
                audioRecord.release();
                audioRecord = null;
            } catch (Exception e) {
                Log.e(TAG, "AudioRecord stop failed.");
            }
            isAudioRecordRun = false;
            Log.i(TAG, "AudioRecord thread stop.");
        }
    }

    public boolean checkAuthorization() {
        boolean ret = false;
        // Wait last record stop
        if (!waitLastAudioRecordStop()) {
            return false;
        }
        Log.i(TAG, "AudioRecord checkAuthorization start.");
        isAudioRecordRun = true;
        try {
            minBufferSize = AudioRecord.getMinBufferSize(sampleRate, channel, format);
            cacheBufferSize = (readBufferSize / minBufferSize + 3) * minBufferSize;
            Log.i(TAG, "minBufferSize = " + minBufferSize + ", cacheBufferSize = " + cacheBufferSize);
            audioRecord = new AudioRecord(source, sampleRate, channel, format, cacheBufferSize);
            audioRecord.startRecording();
        } catch (Exception e) {
            Log.e(TAG, "AudioRecord start failed.");
            isAudioRecordRun = false;
            return false;
        }

        byte[] readBuffer = new byte[readBufferSize];
        int readSize = audioRecord.read(readBuffer, 0, readBufferSize);
        ret  = readSize > 0 ? true : false;

        try {
            audioRecord.stop();
            audioRecord.release();
            audioRecord = null;
        } catch (Exception e) {
            Log.e(TAG, "AudioRecord stop failed.");
        }
        isAudioRecordRun = false;
        return ret;
    }
}
