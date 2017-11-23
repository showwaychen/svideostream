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
    public MediaConfig getH264EncoderConfigs()
    {
        MediaConfig configs = new MediaConfig();
        configs.putConfig(VideoStreamConstants.KEY_PROFILE, VideoStreamConstants.VALUE_BASELINE);
        configs.putConfig(VideoStreamConstants.KEY_PRESET, VideoStreamConstants.VALUE_superfast);
        configs.putConfig(VideoStreamConstants.KEY_TUNE, VideoStreamConstants.VALUE_zerolatency);
        configs.putConfig(VideoStreamConstants.KEY_RC_METHOD, VideoStreamConstants.VALUE_rc_abr);
        return configs;
    }
}
