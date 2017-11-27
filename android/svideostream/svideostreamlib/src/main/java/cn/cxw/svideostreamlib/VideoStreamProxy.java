package cn.cxw.svideostreamlib;

import android.util.Log;

import java.nio.ByteBuffer;

/**
 * Created by cxw on 2017/11/12.
 */

public class VideoStreamProxy implements VideoFrameSource.OnVideoFrameComing {
    public static String TAG = VideoStreamProxy.class.getCanonicalName();
    VideoFrameSource mVideoFrameSource = null;
    SVideoStream mSVideoStream = null;
    VideoStreamSetting mSetting = new VideoStreamSetting();

    boolean mIsStarted = false;
    //stream
    int mStreamWidth = 0;
    int mStreamHeight = 0;

    Boolean mbIsLive = false;
    public VideoStreamProxy()
    {
        mSVideoStream = new SVideoStream();
    }
    public void setVideoFrameSource(VideoFrameSource vsource)
    {
        mVideoFrameSource = vsource;
        mVideoFrameSource.setFrameCallback(this);
    }
    public void setVideoStreamSetting(VideoStreamSetting setting)
    {
        mSetting = setting;
    }
    public void setRecordPath(String path)
    {
        mbIsLive = false;
        mSVideoStream.SetStreamType(VideoStreamConstants.ST_RECORD);
        mSVideoStream.SetRecordFilename(path);
    }
    public void setPublishUrl(String url)
    {
        mbIsLive = true;
        mSVideoStream.SetStreamType(VideoStreamConstants.ST_LIVE);
        mSVideoStream.SetPublishUrl(url);
    }
    public void setStreamSize(int width, int height)
    {
        mStreamHeight = height;
        mStreamWidth = width;
        mSVideoStream.SetDstSize(mStreamWidth, mStreamHeight);
    }
    public int getState()
    {
        return mSVideoStream.getState();
    }
    public StatsReport[] getStatsReport()
    {
        return mSVideoStream.getStatsReport();
    }
    public void setStreamEventObserver(SVideoStream.IStreamEventObserver observer)
    {
        mSVideoStream.setEventObserver(observer);
    }
    public long getDuration()
    {
        return mSVideoStream.getStreamDuration();
    }
    private void streamConfig()
    {
        if (mStreamHeight == 0 || mStreamHeight == 0)
        {
            if (mVideoFrameSource.getRotiation() == VideoStreamConstants.kRotate90 ||
                    mVideoFrameSource.getRotiation() == VideoStreamConstants.kRotate270)
            {
                mStreamHeight = mVideoFrameSource.getSrcWidth();
                mStreamWidth = mVideoFrameSource.getSrcHeight();
            }
            else
            {
                mStreamHeight = mVideoFrameSource.getSrcHeight();
                mStreamWidth = mVideoFrameSource.getSrcWidth();
            }
        }

        //setting
        mSVideoStream.SetSrcImageParams(mVideoFrameSource.getSrcFormate(), mVideoFrameSource.getSrcStride(), mVideoFrameSource.getSrcWidth()
                , mVideoFrameSource.getSrcHeight());
        mSVideoStream.SetDstSize(mStreamWidth, mStreamHeight);
        mSVideoStream.SetRotation(mVideoFrameSource.getRotiation());
        mSVideoStream.SetAudioEnable(mSetting.getAudioEnable());
        mSVideoStream.setVideoEncoderType(mSetting.getVideoEncoderType());
        mSVideoStream.SetVideoEncodeParams(mSetting.getVideoBitrate(), mSetting.getVideoFps());
        //
        MediaConfig h264encoderconfigs = mSetting.getH264EncoderConfigs();
        Log.d(TAG, h264encoderconfigs.toString());
        if (mbIsLive)
        {
            h264encoderconfigs.putConfig(VideoStreamConstants.KEY_PRESET, VideoStreamConstants.VALUE_superfast);
            h264encoderconfigs.putConfig(VideoStreamConstants.KEY_TUNE, VideoStreamConstants.VALUE_zerolatency);
        }
        mSVideoStream.settingSet(VideoStreamConstants.SKV_H264ENCODERCONFIG, h264encoderconfigs);
    }
    public int startStream()
    {

        if (mVideoFrameSource == null || !mVideoFrameSource.isStarted())
        {
            Log.d(TAG, "mVideoFrameSource hasn't started");
            return -1;
        }
        if (mIsStarted)
        {
            return 0;
        }
        streamConfig();
        int ret = 0;
        ret = mSVideoStream.StartStream();
        if (ret == 0)
        {
            mIsStarted = true;
        }
        return ret;
    }

    public void  pauseStream()
    {
         mSVideoStream.pauseStream();
    }
    public void resumeStream()
    {
        mSVideoStream.resumeStream();
    }
    public int stopStream()
    {
        mStreamHeight = mStreamWidth = 0;
        int ret = mSVideoStream.StopStream();
        mIsStarted = false;
        return ret;
    }

    public void destroyStream()
    {
        mSVideoStream.destroyStream();
        mSVideoStream = null;
    }
    @Override
    public void onVideoFrameComing(ByteBuffer framedata, int stride, int height) {
        if (mIsStarted)
        {
            mSVideoStream.InpputVideoData(framedata);
        }
    }
}
