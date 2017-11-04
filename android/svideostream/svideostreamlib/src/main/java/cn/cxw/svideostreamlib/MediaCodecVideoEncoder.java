package cn.cxw.svideostreamlib;

import android.annotation.TargetApi;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaFormat;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;

import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.List;

/**
 * Created by user on 2017/11/1.
 */

public class MediaCodecVideoEncoder {
    private static final String TAG = MediaCodecVideoEncoder.class.getCanonicalName();
    private static final int DEQUEUE_TIMEOUT = 0;  // Non-blocking, no wait.
    private static final String H264_MIME_TYPE = "video/avc";
    // List of supported HW H.264 codecs.
    private static final String[] supportedH264HwCodecPrefixes =
            {"OMX.qcom." };
    // List of devices with poor H.264 encoder quality.
    private static final String[] H264_HW_EXCEPTION_MODELS = new String[] {
            // HW H.264 encoder on below devices has poor bitrate control - actual
            // bitrates deviates a lot from the target value.
            "SAMSUNG-SGH-I337",
            "Nexus 7",
            "Nexus 4"
    };
    public static final String VCODEC = "video/avc";
    private MediaCodec mediaCodec;
    private MediaCodecInfo vmci;
    private MediaCodec.BufferInfo vebi = new MediaCodec.BufferInfo();
    // SPS and PPS NALs (Config frame) for H.264.
    private ByteBuffer configData = null;
    //native using
    private int mVideoColorFormat = VideoStreamConstants.IMAGE_FORMAT_I420;

    // Bitrate modes - should be in sync with OMX_VIDEO_CONTROLRATETYPE defined
    // in OMX_Video.h
    private static final int VIDEO_ControlRateConstant = 2;
    // NV12 color format supported by QCOM codec, but not declared in MediaCodec -
    // see /hardware/qcom/media/mm-core/inc/OMX_QCOMExtns.h
    private static final int
            COLOR_QCOM_FORMATYUV420PackedSemiPlanar32m = 0x7FA30C04;
    // Allowable color formats supported by codec - in order of preference.
    private static final int[] supportedColorList = {
            MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar,
            MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420SemiPlanar,
            MediaCodecInfo.CodecCapabilities.COLOR_QCOM_FormatYUV420SemiPlanar,
            COLOR_QCOM_FORMATYUV420PackedSemiPlanar32m
    };

    int width = 0;
    int height = 0;
//    int bps = 0;
//    int fps = 0;
    private ByteBuffer[] outputBuffers;
    // Helper struct for findHwEncoder() below.
    private static class EncoderProperties {
        public EncoderProperties(String codecName, int colorFormat) {
            this.codecName = codecName;
            this.colorFormat = colorFormat;
        }
        public final String codecName; // OpenMax component name for HW codec.
        public final int colorFormat;  // Color format supported by codec.
    }
    private static EncoderProperties findHwEncoder(
            String mime, String[] supportedHwCodecPrefixes, int[] colorList) {
        // MediaCodec.setParameters is missing for JB and below, so bitrate
        // can not be adjusted dynamically.
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.KITKAT) {
            return null;
        }

//         Check if device is in H.264 exception list.
        if (mime.equals(H264_MIME_TYPE)) {
            List<String> exceptionModels = Arrays.asList(H264_HW_EXCEPTION_MODELS);
            if (exceptionModels.contains(Build.MODEL)) {
                Log.w(TAG, "Model: " + Build.MODEL + " has black listed H.264 encoder.");
                return null;
            }
        }

        for (int i = 0; i < MediaCodecList.getCodecCount(); ++i) {
            MediaCodecInfo info = MediaCodecList.getCodecInfoAt(i);
            if (!info.isEncoder()) {
                continue;
            }
            String name = null;
            for (String mimeType : info.getSupportedTypes()) {
                if (mimeType.equals(mime)) {
                    name = info.getName();
                    break;
                }
            }
            if (name == null) {
                continue;  // No HW support in this codec; try the next one.
            }
//            Logging.v(TAG, "Found candidate encoder " + name);

            // Check if this is supported HW encoder.
            boolean supportedCodec = false;
            for (String hwCodecPrefix : supportedHwCodecPrefixes) {
                if (name.startsWith(hwCodecPrefix)) {
                    supportedCodec = true;
                    break;
                }
            }
            if (!supportedCodec) {
                continue;
            }

            MediaCodecInfo.CodecCapabilities capabilities = info.getCapabilitiesForType(mime);
            for (int colorFormat : capabilities.colorFormats) {
//                Logging.v(TAG, "   Color: 0x" + Integer.toHexString(colorFormat));
            }

            for (int supportedColorFormat : colorList) {
                for (int codecColorFormat : capabilities.colorFormats) {
                    if (codecColorFormat == supportedColorFormat) {
                        // Found supported HW encoder.
//                        Logging.d(TAG, "Found target encoder for mime " + mime + " : " + name +
//                                ". Color: 0x" + Integer.toHexString(codecColorFormat));
                        return new EncoderProperties(name, codecColorFormat);
                    }
                }
            }
        }
        return null;  // No HW encoder.
    }
    static MediaCodec createByCodecName(String codecName) {
        try {
            // In the L-SDK this call can throw IOException so in order to work in
            // both cases catch an exception.
            return MediaCodec.createByCodecName(codecName);
        } catch (Exception e) {
            return null;
        }
    }
    public void resetBitrate(int mbitrate) {

    }

    @TargetApi(Build.VERSION_CODES.KITKAT)
    boolean encodeBuffer(
            boolean isKeyframe, int inputBuffer, int size,
            long presentationTimestampUs) {
        Log.d(TAG, "inputBufferindex = " + inputBuffer + " size = " + size + " pts = " + presentationTimestampUs);
        try {
            if (isKeyframe) {
                // Ideally MediaCodec would honor BUFFER_FLAG_SYNC_FRAME so we could
                // indicate this in queueInputBuffer() below and guarantee _this_ frame
                // be encoded as a key frame, but sadly that flag is ignored.  Instead,
                // we request a key frame "soon".
                Log.d(TAG, "Sync frame request");
                Bundle b = new Bundle();
                b.putInt(MediaCodec.PARAMETER_KEY_REQUEST_SYNC_FRAME, 0);
                mediaCodec.setParameters(b);
            }
            mediaCodec.queueInputBuffer(
                    inputBuffer, 0, size, presentationTimestampUs, 0);
            return true;
        }
        catch (IllegalStateException e) {
            Log.e(TAG, "encodeBuffer failed", e);
            return false;
        }
    }
    static class OutputBufferInfo {
        public OutputBufferInfo(
                int index, ByteBuffer buffer,
                boolean isKeyFrame, long presentationTimestampUs) {
            this.index = index;
            this.buffer = buffer;
            this.isKeyFrame = isKeyFrame;
            this.presentationTimestampUs = presentationTimestampUs;
        }
        public final int index;
        public final ByteBuffer buffer;
        public final boolean isKeyFrame;
        public final long presentationTimestampUs;
    }
    OutputBufferInfo dequeueOutputBuffer() {
        try {
            MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();
            int result = mediaCodec.dequeueOutputBuffer(info, DEQUEUE_TIMEOUT);
            Log.d(TAG, "dequeueOutputBuffer result = " + result);
            // Check if this is config frame and save configuration data.
            if (result >= 0) {
                boolean isConfigFrame =
                        (info.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) != 0;
                if (isConfigFrame) {
                    Log.d(TAG, "Config frame generated. Offset: " + info.offset +
                            ". Size: " + info.size);
                    configData = ByteBuffer.allocateDirect(info.size);
                    outputBuffers[result].position(info.offset);
                    outputBuffers[result].limit(info.offset + info.size);
                    configData.put(outputBuffers[result]);
                    // Release buffer back.
//                    mediaCodec.releaseOutputBuffer(result, false);
                    // Query next output.
//                    result = mediaCodec.dequeueOutputBuffer(info, DEQUEUE_TIMEOUT);
                    ByteBuffer outputBuffer = outputBuffers[result].duplicate();
                    outputBuffer.position(info.offset);
                    outputBuffer.limit(info.offset + info.size);
                    return new OutputBufferInfo(result, outputBuffer.slice(),
                            true, -1);
                }
            }
            if (result >= 0) {
                // MediaCodec doesn't care about Buffer position/remaining/etc so we can
                // mess with them to get a slice and avoid having to pass extra
                // (BufferInfo-related) parameters back to C++.
                ByteBuffer outputBuffer = outputBuffers[result].duplicate();
                outputBuffer.position(info.offset);
                outputBuffer.limit(info.offset + info.size);
                // Check key frame flag.
                boolean isKeyFrame =
                        (info.flags & MediaCodec.BUFFER_FLAG_SYNC_FRAME) != 0;
                if (isKeyFrame) {
                    Log.d(TAG, "Sync frame generated");
                }
                    return new OutputBufferInfo(result, outputBuffer.slice(),
                            isKeyFrame, info.presentationTimeUs);
//                }
            }else if (result == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {
                outputBuffers = mediaCodec.getOutputBuffers();
                return dequeueOutputBuffer();
            } else if (result == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                return dequeueOutputBuffer();
            } else if (result == MediaCodec.INFO_TRY_AGAIN_LATER) {
                return null;
            }
            throw new RuntimeException("dequeueOutputBuffer: " + result);
        } catch (IllegalStateException e) {
            Log.e(TAG, "dequeueOutputBuffer failed", e);
            return new OutputBufferInfo(-1, null, false, -1);
        }
    }
    void setSupportColorFormat(int format)
    {
        switch (format)
        {
            case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar:
                mVideoColorFormat = VideoStreamConstants.IMAGE_FORMAT_I420;
                break;
            case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420SemiPlanar:
                mVideoColorFormat = VideoStreamConstants.IMAGE_FORMAT_NV12;
                break;
            case MediaCodecInfo.CodecCapabilities.COLOR_QCOM_FormatYUV420SemiPlanar:
                mVideoColorFormat = VideoStreamConstants.IMAGE_FORMAT_NV12;
                break;
            case COLOR_QCOM_FORMATYUV420PackedSemiPlanar32m:
                mVideoColorFormat = VideoStreamConstants.IMAGE_FORMAT_NV12;
                break;
        }
    }
    public int openEncode(int width, int height, int kbps, int fps) {
        Log.d(TAG, " " + width + " " + height + " " + kbps + " " + fps);
        if (width <= 0 || kbps <= 0 || height <= 0 || fps <= 0)
        {
            return -1;
        }
        this.width = width;
        this.height = height;
        EncoderProperties properties = null;
        String mime = null;
        int keyFrameIntervalSec = 0;
        mime = H264_MIME_TYPE;
        properties = findHwEncoder(H264_MIME_TYPE, supportedH264HwCodecPrefixes, supportedColorList);
        keyFrameIntervalSec = 20;
        if (properties == null) {
            Log.d(TAG, "findHwEncoder failed");
            return -1;
        }
        Log.d(TAG, "find a encoder codec name = " +  properties.codecName + " colorformat = " + properties.colorFormat);
        try {
            setSupportColorFormat(properties.colorFormat);
            MediaFormat format = MediaFormat.createVideoFormat(mime, width, height);
            format.setInteger(MediaFormat.KEY_BIT_RATE, kbps);
//            format.setInteger("bitrate-mode", VIDEO_ControlRateConstant);
            format.setInteger(MediaFormat.KEY_BITRATE_MODE,
                    MediaCodecInfo.EncoderCapabilities.BITRATE_MODE_CQ);
            format.setInteger(MediaFormat.KEY_COLOR_FORMAT, properties.colorFormat);
            format.setInteger(MediaFormat.KEY_FRAME_RATE, fps);
            format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, keyFrameIntervalSec);
//            Logging.d(TAG, "  Format: " + format);
            mediaCodec = createByCodecName(properties.codecName);
//            this.type = type;
            if (mediaCodec == null) {
                Log.e(TAG, "Can not create media encoder");
                return -1;
            }
            mediaCodec.configure(
                    format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
            mediaCodec.start();
            outputBuffers = mediaCodec.getOutputBuffers();
            Log.d(TAG, "Output buffers: " + outputBuffers.length);

        } catch (IllegalStateException e) {
            Log.e(TAG, "initEncode failed", e);
            return -1;
        }
        Log.d(TAG, "open encoder success");
        return 0;
    }
    ByteBuffer[]  getInputBuffers() {
        ByteBuffer[] inputBuffers = mediaCodec.getInputBuffers();
        Log.d(TAG, "Input buffers: " + inputBuffers.length);
        return inputBuffers;
    }
    public int closeEncode() {
        try {
            mediaCodec.stop();
            mediaCodec.release();
        } catch (Exception e){
            e.printStackTrace();
        }
        Log.d(TAG,"close encoder success");
        return 0;
    }

    int dequeueInputBuffer() {
        try {
            return mediaCodec.dequeueInputBuffer(DEQUEUE_TIMEOUT);
        } catch (IllegalStateException e) {
            Log.e(TAG, "dequeueIntputBuffer failed", e);
            return -2;
        }
    }
    boolean releaseOutputBuffer(int index) {
        Log.d(TAG, "releaseOutputBuffer  index = "+ index);
        try {
            mediaCodec.releaseOutputBuffer(index, false);
            return true;
        } catch (IllegalStateException e) {
            Log.e(TAG, "releaseOutputBuffer failed", e);
            return false;
        }
    }
    // choose the right supported color format. @see below:
    private int chooseVideoEncoder() {
        // choose the encoder "video/avc":
        //      1. select default one when type matched.
        //      2. google avc is unusable.
        //      3. choose qcom avc.
        vmci = chooseVideoEncoder(null);
        //vmci = chooseVideoEncoder("google");
        //vmci = chooseVideoEncoder("qcom");

        int matchedColorFormat = 0;
        MediaCodecInfo.CodecCapabilities cc = vmci.getCapabilitiesForType(VCODEC);
        for (int i = 0; i < cc.colorFormats.length; i++) {
            int cf = cc.colorFormats[i];
            Log.i(TAG, String.format("vencoder %s supports color fomart 0x%x(%d)", vmci.getName(), cf, cf));

            // choose YUV for h.264, prefer the bigger one.
            // corresponding to the color space transform in onPreviewFrame
            if (cf >= cc.COLOR_FormatYUV420Planar && cf <= cc.COLOR_FormatYUV420SemiPlanar) {
                if (cf > matchedColorFormat) {
                    matchedColorFormat = cf;
                }
            }
        }

        for (int i = 0; i < cc.profileLevels.length; i++) {
            MediaCodecInfo.CodecProfileLevel pl = cc.profileLevels[i];
            Log.i(TAG, String.format("vencoder %s support profile %d, level %d", vmci.getName(), pl.profile, pl.level));
        }

        Log.i(TAG, String.format("vencoder %s choose color format 0x%x(%d)", vmci.getName(), matchedColorFormat, matchedColorFormat));
//        return matchedColorFormat;
        return cc.COLOR_FormatYUV420Planar;
    }
    // choose the video encoder by name.
    private MediaCodecInfo chooseVideoEncoder(String name) {
        int nbCodecs = MediaCodecList.getCodecCount();
        for (int i = 0; i < nbCodecs; i++) {
            MediaCodecInfo mci = MediaCodecList.getCodecInfoAt(i);
            if (!mci.isEncoder()) {
                continue;
            }

            String[] types = mci.getSupportedTypes();
            for (int j = 0; j < types.length; j++) {
                if (types[j].equalsIgnoreCase(VCODEC)) {
                    Log.i(TAG, String.format("vencoder %s types: %s", mci.getName(), types[j]));
                    if (name == null) {
                        return mci;
                    }

                    if (mci.getName().contains(name)) {
                        return mci;
                    }
                }
            }
        }

        return null;
    }

}
