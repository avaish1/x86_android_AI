package com.example.interactive_face_demo;

import android.content.Context;
import android.hardware.Camera;
import android.util.AttributeSet;
import android.util.Log;

import org.opencv.android.JavaCameraView;
import org.opencv.core.Size;

import java.util.Iterator;
import java.util.List;

public class CustomCameraView extends JavaCameraView {
    private static final String  TAG ="Face_DEMO:CameraView";
    private float mCustomScale = 0;

    public CustomCameraView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }


    public void setPreviewFPS(double fps){
        int setFps = (int) fps*1000;
        Camera.Parameters params = mCamera.getParameters();
        List<int[]> supportedFPS = params.getSupportedPreviewFpsRange();
        Iterator supportedFPSIterator = supportedFPS.iterator();
        int[] curr = (int[]) supportedFPSIterator.next();
        int currMin = curr[params.PREVIEW_FPS_MIN_INDEX];
        int currMax = curr[params.PREVIEW_FPS_MAX_INDEX];
        int prevRange = curr[params.PREVIEW_FPS_MAX_INDEX] - curr[params.PREVIEW_FPS_MIN_INDEX];
        while(supportedFPSIterator.hasNext()) {
            Log.d(TAG, "min: " + currMin + " max: " + currMax);
            curr = (int[]) supportedFPSIterator.next();
            currMin = curr[params.PREVIEW_FPS_MIN_INDEX];
            currMax = curr[params.PREVIEW_FPS_MAX_INDEX];
            if (setFps >= currMin && setFps <= currMax && (currMax - currMin) >= prevRange) {
                Log.d(TAG, "Change FPS range");
                params.setPreviewFpsRange(currMin, currMax);
            }
            prevRange = currMax - currMin;
        }
        Log.d(TAG, "min: " + currMin + " max: " + currMax);
        int[] range = new int[2];
        params.getPreviewFpsRange(range);
        Log.d(TAG, "Setting Camera FPS range:[" + range[params.PREVIEW_FPS_MIN_INDEX] + ", " + range[params.PREVIEW_FPS_MAX_INDEX] + "]");
        mCamera.setParameters(params);
    }

    @Override
    protected Size calculateCameraFrameSize(List<?> supportedSizes, ListItemAccessor accessor, int surfaceWidth, int surfaceHeight) {
        Log.d(TAG, "calculateCameraFrameSize");
        if (mMaxWidth > 0 && mMaxHeight > 0) {
            return new Size(mMaxWidth, mMaxHeight);
        } else {
            return super.calculateCameraFrameSize(supportedSizes, accessor, surfaceWidth, surfaceHeight);
        }
    }

    public void setScale(float scale) {
        mCustomScale = scale;
    }

    @Override
    protected void deliverAndDrawFrame(CvCameraViewFrame frame) {
        if (mCustomScale > 0) {
            mScale = mCustomScale;
        }
        super.deliverAndDrawFrame(frame);
    }

}
