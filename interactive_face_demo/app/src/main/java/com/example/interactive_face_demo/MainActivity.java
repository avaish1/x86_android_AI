package com.example.interactive_face_demo;

import android.Manifest;
import android.annotation.SuppressLint;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Camera;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.HandlerThread;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceView;
import android.view.Window;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.TextView;


import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraActivity;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.android.Utils;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.MatOfRect;
import org.opencv.core.Rect;
import org.opencv.core.Scalar;
import org.opencv.core.Size;
import org.opencv.imgcodecs.Imgcodecs;
import org.opencv.imgproc.Imgproc;
import org.opencv.videoio.VideoCapture;
import org.opencv.videoio.VideoWriter;
import org.opencv.videoio.Videoio;

import java.io.IOException;
import java.io.InputStream;
import java.util.Collections;
import java.util.List;

import static android.webkit.ConsoleMessage.MessageLevel.LOG;
import static java.lang.Math.round;

//import static org.opencv.videoio.Videoio.CV_CAP_PROP_FOURCC;

public class MainActivity extends CameraActivity implements CvCameraViewListener2 {
    public static final String TAG = "Face_Demo_Activity";
    private static final Scalar FACE_RECT_COLOR = new Scalar(0, 255, 0, 255);
    private static final String HANDLE_THREAD_NAME = "camerabackground";

    private detector mDetector = new detector();
    private CameraBridgeViewBase  mOpenCvCameraView;
    private  Mat mRgbaRenderFrame;
    public String mModelPath = "/vendor/etc/dldtmodels/face-detection-adas-0001.xml";
    private HandlerThread backgroundThread;
    private Handler backgroundHandler;
    private boolean useCamera=false;
    private Bitmap bmImage;


    @Override
    public void onCameraViewStarted(int width, int height) {
        Log.d(TAG, "onCameraViewStarted - camera dims: " + width + " x " + height);

        mRgbaRenderFrame = new Mat();

    }

    @Override
    public void onCameraViewStopped() {
        Log.d(TAG, "onCameraViewStopped");

         mRgbaRenderFrame.release();
    }

    @Override
    public Mat onCameraFrame(CameraBridgeViewBase.CvCameraViewFrame inputFrame) {
        mRgbaRenderFrame = inputFrame.rgba();
        //OPENVINO takes BGR format
        Imgproc.cvtColor(mRgbaRenderFrame, mRgbaRenderFrame, Imgproc.COLOR_RGBA2BGR);
        Size newsz = new Size(672,384);
        Size oldsz = new Size(mRgbaRenderFrame.cols(),mRgbaRenderFrame.rows());
        Imgproc.resize(mRgbaRenderFrame,mRgbaRenderFrame,newsz,0,0,Imgproc.INTER_AREA);
        //drawBoundingBoxes(mRgbaRenderFrame, mCurrentFaces, mCropRect);
        MatOfRect faces = new MatOfRect();
        mDetector.detect(mRgbaRenderFrame.getNativeObjAddr(),faces.getNativeObjAddr()); //lets do capture from JNI

        Rect[] facesArray = faces.toArray();
        for (int i = 0; i < facesArray.length; i++)
            Imgproc.rectangle(mRgbaRenderFrame, facesArray[i].tl(), facesArray[i].br(), FACE_RECT_COLOR, 3);

        //resize the image back to original size
        Imgproc.resize(mRgbaRenderFrame,mRgbaRenderFrame,oldsz);
        return mRgbaRenderFrame;
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        InitBackgroundThread();

        if(useCamera) {
            setContentView(R.layout.activity_main);

            mOpenCvCameraView = (CameraBridgeViewBase) findViewById(R.id.main_activity_surface_view);
            mOpenCvCameraView.setVisibility(CameraBridgeViewBase.VISIBLE);
            mOpenCvCameraView.setCvCameraViewListener(this);
        }
        else {
            setContentView(R.layout.image_activity_main);
            ImageView mImageView = (ImageView) findViewById(R.id.imageView);
            bmImage= util.copyBitmap(readFromAssets("trump2.jpg"));
            mImageView.setImageBitmap(bmImage);
        }
         //mOpenCvCameraView.onPictureTaken(data, Camera);


         // initializeFaceDetection();

    }
    private boolean initializeFaceDetection() {
        Log.i(TAG, "initializing Face detection");
        //here assetDir represents an use case, and would contain OPENVINO's sample xml and bin for that use case.
        if (mDetector.initialize("interactive_face_demo","CPU",mModelPath) == mDetector.SUCCESS) {
            Log.i(TAG, "initialized FaceDetection successfully");
            return true;
        } else {
            Log.i(TAG, "Failed to initialize FaceDetection");
            return false;
        }
    }
    @Override
    public void onPause()
    {
        super.onPause();
        if (mOpenCvCameraView != null)
            mOpenCvCameraView.disableView();
    }

    @Override
    protected List<? extends CameraBridgeViewBase> getCameraViewList() {
        return Collections.singletonList(mOpenCvCameraView);
    }
    public void onDestroy() {
        super.onDestroy();
        mOpenCvCameraView.disableView();
    }

    private BaseLoaderCallback mLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS: {
                    Log.i(TAG, "OpenCV loaded successfully");

                    mOpenCvCameraView.enableView();
                }break;
                default:
                   super.onManagerConnected(status);

            }

        }
    };

    @Override
    public void onResume () {
        super.onResume();
    if(useCamera) {
            if (!OpenCVLoader.initDebug()) {
                Log.d(TAG, "Internal OpenCV library not found. Using OpenCV Manager for initialization");
                OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION, this, mLoaderCallback);
            } else {
                Log.d(TAG, "OpenCV library found inside package. Using it!");
                mLoaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
            }
     }
    }

    private void InitBackgroundThread() {
        backgroundThread = new HandlerThread(HANDLE_THREAD_NAME);
        backgroundThread.start();
        backgroundHandler = new Handler(backgroundThread.getLooper());
        // Start the classification train & load an initial model.
//        synchronized (lock) {
//            runClassifier = true;
//        }
        backgroundHandler.post(openvino_init);

        if(useCamera == false)
              backgroundHandler.post(detectImage);

    }
    /** Init face detection with openvino */
    private Runnable openvino_init =
            new Runnable() {
                @Override
                public void run() {
                    initializeFaceDetection();
                }
            };

    private Runnable detectImage = new Runnable() {
        @Override
        public void run() {
            inferImage();
        }
    };
    private  Bitmap readFromAssets(String filename){
        Bitmap bitmap;
        AssetManager asm=getAssets();
        try {
            InputStream is=asm.open(filename);
            bitmap= BitmapFactory.decodeStream(is);
            is.close();
        } catch (IOException e) {
            Log.e("MainActivity","[*]failed to open "+filename);
            e.printStackTrace();
            return null;
        }
        return bitmap;
    }
    private void inferImage(){
        Mat mat = new Mat();
        Bitmap bmp32 = bmImage.copy(Bitmap.Config.ARGB_8888, true);
        Utils.bitmapToMat(bmp32, mat);

        MatOfRect faces = new MatOfRect();
        mDetector.detect(mat.getNativeObjAddr(),faces.getNativeObjAddr());
    }
}