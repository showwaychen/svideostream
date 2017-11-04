package cn.cxw.svideostreamlib;

import android.provider.Settings;
import android.util.Log;

/**
 * Created by user on 2017/10/13.
 */

public class SVideoStream implements AudioRecorder.Listener {
    static String TAG = SVideoStream.class.getCanonicalName();
    long m_NativeObject = 0;
    native long nativeCreateObject();
    native int nativeStart();
    native int nativeStop();
    native void nativeSetStreamType(int type);
    native void nativeSetSrcType(int type);
    native void nativeSetSrcImageParams(int format, int stride, int width ,int height);
    native void nativeSetDstParams(int width, int height);
    native void nativeSetRotation(int rotation);
    native void nativeSetVideoEncoderType(int type);
    native void nativeSetVideoEncodeParams(int bitrate, int fps);
    native void nativeSetAudioParams(int samplerate, int channels, int samplesize, int bitrate);
    native void nativeSetPublishUrl(String url);
    native void nativeSetRecordPath(String filename);
    native void nativeSetAudioEnable(boolean enable);

    native int nativeInputVideoData(byte[] vdata, int size, long pts);
    native int  nativeInputAudioData(byte[] adata, int size, long pts);

    native void nativeDestroy();
    static
    {
        System.loadLibrary("svideostream");
    }
    //audio
    AudioRecorder m_AudioRecord = null;
    boolean m_IsAudioOk = true;
    boolean m_AudioEnable = true;
    boolean m_AudioMute = false;
    private long mAudioSumSamples;
    private long m_LastAudioPts = 0;
    private boolean m_IsAudioDataCame = false;


    private boolean mIsPaused;
    private long mStartTime = 0;
    private long mPauseTime = 0;
    private long mPauseSumTime = 0;

    private boolean m_IsStart = false;

    //video
    private long m_LastVideoPts = 0;
    int m_VideoFrameCount = 0;
    int m_Framerate = 0;
    int m_Bitrate = 0;


    public SVideoStream()
    {
        m_NativeObject = nativeCreateObject();
    }
    public void SetStreamType(int type)
    {
        nativeSetStreamType(type);
    }
    public void SetSrcType(int type)
    {
        nativeSetSrcType(type);
    }
    public void SetSrcImageParams(int format, int stride, int width, int height)
    {
        nativeSetSrcImageParams(format, stride, width, height);
    }
    public void SetDstSize(int width, int height)
    {
        nativeSetDstParams(width, height);
    }
    public void SetRotation(int rotation)
    {
        nativeSetRotation(rotation);
    }
    public void SetAudioEnable(boolean enable)
    {
        m_AudioEnable = enable;
        nativeSetAudioEnable(m_AudioEnable);
    }
    public void SetAudioMute(boolean ismute)
    {
        m_AudioMute = ismute;
        if (m_AudioRecord != null)
        {
            m_AudioRecord.isMute = m_AudioMute;
        }
    }
    public long GetStreamDuration()
    {
        if (!m_IsStart)
        {
            return 0;
        }
        return Math.max(m_LastAudioPts, m_LastVideoPts);
    }
    private long GetClockDuration() {
        if (!m_IsStart || mStartTime == 0) {
            return 0;
        }
        return System.currentTimeMillis() - mStartTime - mPauseSumTime;
    }
    private void MemberReset()
    {
        mStartTime = 0;
        mPauseTime = 0;
        mPauseSumTime = 0;
        m_IsStart = true;
        mIsPaused = false;
        m_VideoFrameCount = 1;
    }
    public int StartStream()
    {
        if (m_AudioEnable)
        {
            m_AudioRecord = new AudioRecorder(this);
            if (!m_AudioRecord.checkAuthorization())
            {
                Log.e(TAG, "audio checkAuthorization false");
                SetAudioEnable(false);
                return -1;
            }
            else
            {
                m_AudioRecord.start();
                nativeSetAudioParams(44100, 1, 2, 64000);
            }
        }
        int ret = nativeStart();
        MemberReset();
        if (ret == 0)
        {
            m_IsStart = true;
        }
        else
        {
            if (m_AudioRecord != null)
            {
                m_AudioRecord.stop();
            }
            m_IsStart = false;
            Log.d(TAG, "start stream error ");
        }
        return ret;
    }
    public int StopStream()
    {
        if (m_AudioRecord != null)
        {
            m_AudioRecord.stop();
        }
        int ret = nativeStop();
        m_IsStart = false;
        return ret;
    }
    public void PauseStream()
    {
        if (!m_IsStart)
        {
            return ;
        }
        Log.d(TAG, "pause stream ");
        mIsPaused = true;
        if (mPauseTime == 0)
        {
            mPauseTime = System.currentTimeMillis();
        }
    }
    public void ResumeStream()
    {
        if (!mIsPaused)
        {
            return ;
        }
        Log.d(TAG, "resume stream");
        if (mPauseTime != 0)
        {
            mPauseSumTime += System.currentTimeMillis() - mPauseTime;
            mPauseTime = 0;
        }
        mIsPaused = false;
    }
    public void setVideoEncoderType(int type)
    {
        if (type != VideoStreamConstants.H264ENCODER_MEDIACODEC && type != VideoStreamConstants.H264ENCODER_X264)
        {
            return ;
        }
        nativeSetVideoEncoderType(type);
    }
    public void SetVideoEncodeParams(int bitrate, int fps)
    {
        m_Bitrate = bitrate;
        m_Framerate = fps;
        nativeSetVideoEncodeParams(m_Bitrate, m_Framerate);
    }
    public void SetPublishUrl(String url)
    {
        nativeSetPublishUrl(url);
    }
    public void SetRecordFilename(String filename)
    {
        nativeSetRecordPath(filename);
    }
    public int InpputVideoData(byte[] vdata)
    {
        if (!m_IsStart || mIsPaused) {
            return -1;
        }
        long notTime = System.currentTimeMillis();
        long clockpts = GetClockDuration();
        if (mStartTime == 0)
        {
            if (m_AudioEnable && !m_IsAudioDataCame)
            {
                return 0;
            }
            mStartTime = System.currentTimeMillis();
        }
        long diff = (m_VideoFrameCount * 1000 / m_Framerate) - (notTime - mStartTime);
        if (diff > 10)
        {
            return 0;
        }
        m_VideoFrameCount++;
        m_LastVideoPts = clockpts;
        return nativeInputVideoData(vdata, vdata.length, m_LastVideoPts);
    }
    public void DestroyStream()
    {
        StopStream();
        nativeDestroy();
        m_NativeObject = 0;
    }

    @Override
    public void onOutputAudioBuffer(byte[] buffer, int size) {
        if (!m_IsStart || mIsPaused) {
            return;
        }
        m_IsAudioDataCame = true;
        if (mStartTime == 0) {
            // wait for videodata callback
            Log.d(TAG, "wait for videodata callback");
            return;
        }
        long curclockpts = GetClockDuration();
        long calcPts = mAudioSumSamples * 1000 / 44100;
        long delay = curclockpts - calcPts;
        long pts = calcPts;
        if (delay > 500) {
            pts = curclockpts;
            mAudioSumSamples = pts * 44100 / 1000;
            Log.i(TAG, "Audio delay = " + delay + " calcPts = " + calcPts + " -> clockPts = " + curclockpts);
        } else if (delay < -20) {
            Log.w(TAG, "Audio drop this frame, delay = " + delay + " calcPts = " + calcPts + " -> clockPts = " + curclockpts);
            return;
        }
        mAudioSumSamples += size / 2;
        m_LastAudioPts = pts;
            nativeInputAudioData(buffer, size, pts);
    }
}
