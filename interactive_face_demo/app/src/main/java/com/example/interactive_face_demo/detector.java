package com.example.interactive_face_demo;

import org.opencv.core.Mat;
import org.opencv.core.MatOfRect;

public class detector {
    static {
        try{
            //System.loadLibrary("inference_engine");
            System.loadLibrary("nnhal_ext_jni");
            System.loadLibrary("inference_engine");
            System.loadLibrary("MKLDNNPlugin");
            System.loadLibrary("cpu_extension");
            System.loadLibrary("mkldnn");
            System.loadLibrary("opencv_java4");
        }
        catch (Exception e){
            e.getMessage();
        }

    }
    public static final int SUCCESS = 0;
    public native int initialize(String AssetDir,String DeviceName,String ModelPath);
    public native void terminate();
    public native void detect(final long frame, long faces);

}
