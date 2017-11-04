package cn.cxw.svideostreamlib;

/**
 * Created by cxw on 2017/11/3.
 */

public class VideoStreamSetting {
    public int getVideoEncoderType()
    {
        return VideoStreamConstants.H264ENCODER_X264;
    }
    public boolean getAudioEnable()
    {
        return true;
    }
    public int getVideoBitrate()
    {
        return 300000;
    }
    public int getVideoFps()
    {
        return 15;
    }
}
