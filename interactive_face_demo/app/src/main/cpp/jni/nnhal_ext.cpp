#include <jni.h>
#include <opencv2/core/core.hpp>
#include <android/log.h>
#include <string>
#include <sstream>
#include <opencv2/core/mat.hpp>
#include "ie_common.h"
#include "ie_iextension.h"
#include "ie_plugin_cpp.hpp"
#include "ie_plugin_dispatcher.hpp"
#include "ie_version.hpp"
#include "load_dldt.hpp"

#define LOG_TAG "NN_HAL_EXT_JNI"
#define LOG_D(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOG_E(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)


using namespace cv;
//using namespace InferenceEngine;
/*
 * JNI Functions
 */
extern "C"
JNIEXPORT jint JNICALL
Java_com_example_interactive_1face_1demo_detector_initialize(
        JNIEnv *env,
        jobject /* this */,
        jstring assetDir,
        jstring DeviceName,
        jstring ModelPath) {
    LOG_D("***** initialize started *****");
   const char* asset_dir = env->GetStringUTFChars(assetDir, 0);
    env->ReleaseStringUTFChars(assetDir, asset_dir);
   const char* device = env->GetStringUTFChars(DeviceName,0);
    env->ReleaseStringUTFChars(DeviceName,device);

    const char* modelpath = env->GetStringUTFChars(ModelPath,0);
    env->ReleaseStringUTFChars(ModelPath,modelpath);

    LOG_D("Enabling usecase is %s to run on device %s", asset_dir,device);

    int retval = initDetector(device,modelpath);
    if (retval == 0) {
        LOG_D("***** initialize finished *****");
    } else {
        LOG_D("***** initialize failed [%d] *****", retval);
    }

    return retval;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_interactive_1face_1demo_detector_detect(
        JNIEnv *env,
        jobject ,jlong frame,jlong faces) {
    LOG_D("in Java_com_example_interactive_1face_1demo_detector_detect");

    std::vector<Rect> RectFaces;
    cv::Mat locframe = *((cv::Mat *)frame);
//    cv::MatIterator_<double> _it = locframe.begin<double>();
//    for(;_it < locframe.end<double>();_it++){
//        LOG_D("values are=%f",*_it);
//    }

    DoFaceDetection(locframe, RectFaces);
    *((Mat*)faces) = Mat(RectFaces, true);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_interactive_1face_1demo_detector_terminate(
        JNIEnv *env,
        jobject /* this */) {
    LOG_D("***** terminate started *****");

}

//extern "C"
//JNIEXPORT jobjectArray JNICALL
//Java_com_intel_smartterm_fdapis_getFaceDetection(
//        JNIEnv *env,
//        jobject /* this */,
//        jlong addrImage) {
//    Mat* pInputImage = (Mat*)addrImage;
//    std::vector<FaceDetection> faces;
//
//    LOG_D("***** starting face detection *****");
//    face_recog_engine->getFaceDetections(*pInputImage, faces);
//    for (unsigned int i = 0; i < faces.size(); i++)
//        LOG_D("detection track id = %d", faces[i].track_id);
//    LOG_D("***** finished face detection *****");
//
//    return convertToJava(env, faces);
//}
//
//extern "C"
//JNIEXPORT void JNICALL
//Java_com_intel_smartterm_fdapis_registerFace(
//        JNIEnv *env,
//        jobject /* this */,
//        jlong addrImage,
//        jobject objFace) {
//    Mat* pInputImage = (Mat*)addrImage;
//    struct FaceDetection face_detect;
//    LOG_D("***** starting face register *****");
//
//    convertFDFromJava(env, objFace, &face_detect);
//
//    face_recog_engine->registerFace(*pInputImage, face_detect);
//
//    LOG_D("fdapis_registerFace label = %d", face_detect.label);
//
//    LOG_D("***** finished face register *****");
//    return convertFDToJava(env, objFace,face_detect);
//}
//
//
//extern "C"
//JNIEXPORT jlong JNICALL
//Java_com_intel_smartterm_fdapis_registerFaceWithFaceVector(
//        JNIEnv *env,
//        jobject /* this */,
//        jfloatArray faceVector) {
//
////    LOG_D("***** starting face register with feature vector *****");
//    int len = env->GetArrayLength(faceVector);
//    std::vector<float> featureVector;
//    struct FaceDetection face_detect;
//    LabelType label;
//
//    jfloat *features;
//    features  = env->GetFloatArrayElements(faceVector, 0);
//    for(int i=0; i<len; i++) {
// //       LOG_D("***** pushing  %uf*****",features[i]);
//        featureVector.push_back(features[i]);
//    }
//
//    face_recog_engine->registerFace(label, featureVector);
//    env->ReleaseFloatArrayElements(faceVector , (jfloat *)features, 0);
////    LOG_D("***** finished face register with vector  %ul*****",label);
//    return (jlong)label;
//}
//
//
//extern "C"
//JNIEXPORT jobjectArray JNICALL
//Java_com_intel_smartterm_fdapis_validateFaceDetection(
//        JNIEnv *env,
//        jobject /* this */,
//        jlong addrImage,
//        jlong addrDepthMap,
//        jobjectArray facesArray) {
//    Mat* pInputImage = (Mat*)addrImage;
//    Mat* pDepthMap = (Mat*)addrDepthMap;
//    LOG_D("***** starting face detection validation *****");
//
//    int len = env->GetArrayLength(facesArray);
//    std::vector<FaceDetection> faces;
//    struct FaceDetection face;
//
//    for(int i=0; i<len; i++) {
//        jobject obj = (jobject) env->GetObjectArrayElement(facesArray, i);
//        convertFDFromJava(env, obj, &face);
//        faces.push_back(face);
//    }
//
//    LOG_D("validateFaceDetection 0, label=%x", faces[0].label);
//    face_recog_engine->validateFaceDetection(*pInputImage, *pDepthMap, faces);
//    for (int i=0; i < faces.size(); i++) {
//        LOG_D("%u is %sa valid face", faces[i].label, faces[i].valid_face ? "" : "not ");
//    }
//    LOG_D("***** finished face detection validation *****");
//
//    return convertToJava(env, faces);
//}
//
//extern "C"
//JNIEXPORT jobjectArray JNICALL
//Java_com_intel_smartterm_fdapis_getFaceRecognition(
//        JNIEnv *env,
//        jobject /* this */,
//        jlong addrImage,
//        jobjectArray facesArray) {
//    Mat* pInputImage = (Mat*)addrImage;
//    LOG_D("***** starting face recognition *****");
//
//    int len = env->GetArrayLength(facesArray);
//    std::vector<FaceDetection> faces(len);
//    //struct FaceDetection face;
//
//    for(int i=0; i<len; i++) {
//        jobject obj = (jobject) env->GetObjectArrayElement(facesArray, i);
//        convertFDFromJava(env, obj, &faces[i]);
//    }
//
//    face_recog_engine->getFaceRecognitions(*pInputImage, faces);
//    LOG_D("face_recognition recognized %lu faces", faces.size());
//    for (int i=0; i < faces.size(); i++) {
//        LOG_D("face_recognition recognized %x", faces[i].label);
//        LOG_D("recognition track ID %d", faces[i].track_id);
//    }
//    LOG_D("***** finished face recognition *****");
//
//    return convertToJava(env, faces);
