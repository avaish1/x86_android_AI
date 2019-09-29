// Copyright (C) 2018-2019 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

# pragma once


#include <functional>
#include <iostream>
#include <fstream>
#include <random>
#include <memory>
#include <chrono>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include <iterator>
#include <map>

#include <inference_engine.hpp>

#include <common.hpp>
#include <slog.hpp>

#include <ie_iextension.h>
#include <ext_list.hpp>

#include <opencv2/opencv.hpp>

// -------------------------Generic routines for detection networks-------------------------------------------------

struct BaseDetection {
	//cv::Mat recvdFrame; //frames are recieved here
    InferenceEngine::ExecutableNetwork net;
    InferenceEngine::InferencePlugin * plugin;
    InferenceEngine::InferRequest::Ptr request;
    std::string topoName;
    std::string pathToModel;
    std::string deviceForInference;
    size_t maxBatch;
    bool isBatchDynamic;
    bool isAsync;
    mutable bool enablingChecked;
    mutable bool _enabled;
    bool doRawOutputMessages;

    BaseDetection(){
		
	};

    virtual ~BaseDetection();

    InferenceEngine::ExecutableNetwork* operator ->();
    virtual InferenceEngine::CNNNetwork read() = 0;
    virtual void submitRequest();
    virtual void wait();
    bool enabled() const;
    void printPerformanceCounts();
};

struct FaceDetection : BaseDetection {
    struct Result {
        
		int label;
        float confidence;
        cv::Rect location;
		Result():label(0),confidence(0),location(cv::Rect(0,0,250,250)){};
    };
	size_t framesCounter=0;
	InferenceEngine::CNNNetReader netReader;
    std::string input;
    std::string output;
    double detectionThreshold;
    int maxProposalCount;
    int objectSize;
    int enquedFrames;
    float width;
    float height;
    float bb_enlarge_coefficient;
    float bb_dx_coefficient;
    float bb_dy_coefficient;
    bool resultsFetched;
    std::vector<std::string> labels;
    std::vector<Result> results;

    FaceDetection():width(0),height(0) {};

    InferenceEngine::CNNNetwork read() override;
    void submitRequest() override;

    void enqueue(const cv::Mat &frame);
    void fetchResults();
};

struct AgeGenderDetection : BaseDetection {
    struct Result {
        float age;
        float maleProb;
    };

    std::string input;
    std::string outputAge;
    std::string outputGender;
    size_t enquedFaces;

    AgeGenderDetection(const std::string &pathToModel,
                       const std::string &deviceForInference,
                       int maxBatch, bool isBatchDynamic, bool isAsync,
                       bool doRawOutputMessages);

    InferenceEngine::CNNNetwork read() override;
    void submitRequest() override;

    void enqueue(const cv::Mat &face);
    Result operator[] (int idx) const;
};

struct HeadPoseDetection : BaseDetection {
    struct Results {
        float angle_r;
        float angle_p;
        float angle_y;
    };

    std::string input;
    std::string outputAngleR;
    std::string outputAngleP;
    std::string outputAngleY;
    size_t enquedFaces;
    cv::Mat cameraMatrix;

    HeadPoseDetection(const std::string &pathToModel,
                      const std::string &deviceForInference,
                      int maxBatch, bool isBatchDynamic, bool isAsync,
                      bool doRawOutputMessages);

    InferenceEngine::CNNNetwork read() override;
    void submitRequest() override;

    void enqueue(const cv::Mat &face);
    Results operator[] (int idx) const;
};

struct EmotionsDetection : BaseDetection {
    std::string input;
    std::string outputEmotions;
    size_t enquedFaces;

    EmotionsDetection(const std::string &pathToModel,
                      const std::string &deviceForInference,
                      int maxBatch, bool isBatchDynamic, bool isAsync,
                      bool doRawOutputMessages);

    InferenceEngine::CNNNetwork read() override;
    void submitRequest() override;

    void enqueue(const cv::Mat &face);
    std::map<std::string, float> operator[] (int idx) const;

    const std::vector<std::string> emotionsVec = {"neutral", "happy", "sad", "surprise", "anger"};
};

struct FacialLandmarksDetection : BaseDetection {
    std::string input;
    std::string outputFacialLandmarksBlobName;
    size_t enquedFaces;
    std::vector<std::vector<float>> landmarks_results;
    std::vector<cv::Rect> faces_bounding_boxes;

    FacialLandmarksDetection(const std::string &pathToModel,
                             const std::string &deviceForInference,
                             int maxBatch, bool isBatchDynamic, bool isAsync,
                             bool doRawOutputMessages);

    InferenceEngine::CNNNetwork read() override;
    void submitRequest() override;

    void enqueue(const cv::Mat &face);
    std::vector<float> operator[] (int idx) const;
};

struct Load {
    BaseDetection& detector;

    explicit Load(BaseDetection& detector);

    void into(InferenceEngine::InferencePlugin & plg, bool enable_dynamic_batch = false) const;
};

class CallStat {
public:
    typedef std::chrono::duration<double, std::ratio<1, 1000>> ms;

    CallStat();

    double getSmoothedDuration();
    double getTotalDuration();
    double getLastCallDuration();
    void calculateDuration();
    void setStartTime();

private:
    size_t _number_of_calls;
    double _total_duration;
    double _last_call_duration;
    double _smoothed_duration;
    std::chrono::time_point<std::chrono::high_resolution_clock> _last_call_start;
};

class Timer {
public:
    void start(const std::string& name);
    void finish(const std::string& name);
    CallStat& operator[](const std::string& name);

private:
    std::map<std::string, CallStat> _timers;
};