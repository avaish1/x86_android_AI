// Copyright (C) 2018-2019 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//


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
#include <ocv_common.hpp>
#include <slog.hpp>
#include <ie_iextension.h>
#include <ext_list.hpp>
#include "detectors.hpp"
#include <typeinfo>



using namespace InferenceEngine;

BaseDetection::~BaseDetection() {
	
}

ExecutableNetwork* BaseDetection::operator ->() {
    return &net;
}

void BaseDetection::submitRequest() {
    if (!enabled() || request == nullptr) return;
    LOG_D("Frame submitted for Face Detection");
	if (isAsync) {
        request->StartAsync();
    } else {
        request->Infer();
    }
}

void BaseDetection::wait() {
    if (!enabled()|| !request || !isAsync)
        return;
    request->Wait(IInferRequest::WaitMode::RESULT_READY);
}

bool BaseDetection::enabled() const  {
    if (!enablingChecked) {
        _enabled = !pathToModel.empty();
        if (!_enabled) {
            slog::info << topoName << " DISABLED" << slog::endl;
        }
        enablingChecked = true;
    }
    return _enabled;
}

void BaseDetection::printPerformanceCounts() {
    if (!enabled()) {
        return;
    }
    slog::info << "Performance counts for " << topoName << slog::endl << slog::endl;
    ::printPerformanceCounts(request->GetPerformanceCounts(), std::cout, false);
}



void FaceDetection::submitRequest() {
    if (!enquedFrames) return;
    enquedFrames = 0;
    resultsFetched = false;
    results.clear();
    BaseDetection::submitRequest();
}

void FaceDetection::enqueue(const cv::Mat &frame) {
    if (!enabled()) return;

    if (!request) {
        request = net.CreateInferRequestPtr();
    }
	
    width = static_cast<float>(frame.cols);
    height = static_cast<float>(frame.rows);
	
	LOG_D("Frames enqueued have cols=%f,rows=%f",width,height);
	
    Blob::Ptr  inputBlob = request->GetBlob(input);
	
	//Converting input in NCHW format
    matU8ToBlob<uint8_t>(frame, inputBlob);
	
	auto nelem = (inputBlob->size() > 20 ? 20 : inputBlob->size());
	
	auto fv = inputBlob->buffer().as<float *>();
	const float *detections = request->GetBlob(output)->buffer().as<float *>();
	
	for (int i = 0; i < nelem ; i++) {
		LOG_D("Enque:inBlob elements %d=%f",i,fv[i]);
		
	}
	for (int i = 0; i < nelem ; i++) {
	LOG_D("Enque:outputBlob elements %d=%f",i,detections[i]);
	}
    enquedFrames = 1;
}

CNNNetwork FaceDetection::read()  {
    LOG_D("Loading network files %s for Face Detection",pathToModel.c_str());
    
    /** Read network model **/
    netReader.ReadNetwork(pathToModel);
    /** Set batch size to 1 **/
    LOG_D("Batch size is set to ",maxBatch);
    netReader.getNetwork().setBatchSize(maxBatch);
    /** Extract model name and load its weights **/
    std::string binFileName = fileNameNoExt(pathToModel) + ".bin";
	
	LOG_D("bin filename %s for Face Detection",binFileName.c_str());
	
    netReader.ReadWeights(binFileName);
    /** Read labels (if any)**/
    std::string labelFileName = fileNameNoExt(pathToModel) + ".labels";
	LOG_D("labelFileName %s for Face Detection",labelFileName.c_str());
    std::ifstream inputFile(labelFileName);
    std::copy(std::istream_iterator<std::string>(inputFile),
              std::istream_iterator<std::string>(),
              std::back_inserter(labels));
    // -----------------------------------------------------------------------------------------------------

    /** SSD-based network should have one input and one output **/
    // ---------------------------Check inputs -------------------------------------------------------------
    LOG_D("Checking Face Detection network inputs");
    InputsDataMap inputInfo(netReader.getNetwork().getInputsInfo());
    if (inputInfo.size() != 1) {
        throw std::logic_error("Face Detection network should have only one input");
    }
    InputInfo::Ptr inputInfoFirst = inputInfo.begin()->second;
    //inputInfoFirst->setPrecision(Precision::FP32);
	inputInfoFirst->setPrecision(Precision::FP32);
	inputInfoFirst->setLayout(Layout::NCHW);
    // -----------------------------------------------------------------------------------------------------

    // ---------------------------Check outputs ------------------------------------------------------------
    LOG_D("Checking Face Detection network outputs");
    OutputsDataMap outputInfo(netReader.getNetwork().getOutputsInfo());
    if (outputInfo.size() != 1) {
        throw std::logic_error("Face Detection network should have only one output");
    }
    DataPtr& _output = outputInfo.begin()->second;
    output = outputInfo.begin()->first;

    const CNNLayerPtr outputLayer = netReader.getNetwork().getLayerByName(output.c_str());
    if (outputLayer->type != "DetectionOutput") {
        throw std::logic_error("Face Detection network output layer(" + outputLayer->name +
                               ") should be DetectionOutput, but was " +  outputLayer->type);
    }

    if (outputLayer->params.find("num_classes") == outputLayer->params.end()) {
        throw std::logic_error("Face Detection network output layer (" +
                               output + ") should have num_classes integer attribute");
    }

    const size_t num_classes = outputLayer->GetParamAsUInt("num_classes");
    if (labels.size() != num_classes) {
        if (labels.size() == (num_classes - 1))  // if network assumes default "background" class, which has no label
            labels.insert(labels.begin(), "fake");
        else
            labels.clear();
    }
    const SizeVector outputDims = _output->getTensorDesc().getDims();
    maxProposalCount = outputDims[2];
    objectSize = outputDims[3];
    if (objectSize != 7) {
        throw std::logic_error("Face Detection network output layer should have 7 as a last dimension");
    }
    if (outputDims.size() != 4) {
        throw std::logic_error("Face Detection network output dimensions not compatible shoulld be 4, but was " +
                               std::to_string(outputDims.size()));
    }
    _output->setPrecision(Precision::FP32);

    LOG_D("Loaded Face Detection model");
    input = inputInfo.begin()->first;
    return netReader.getNetwork();
}

void FaceDetection::fetchResults() {
    if (!enabled()) return;
    results.clear();
   /*  if (resultsFetched) return;
    resultsFetched = true; */
    const float *detections = request->GetBlob(output)->buffer().as<float *>();
	auto nelem = (request->GetBlob(output)->size() > 20 ? 20 : request->GetBlob(output)->size());
	const float *inval = request->GetBlob(input)->buffer().as<float *>();
	
	for(int i=0;i < nelem; i++){
			LOG_D("output elements are %d=%f",i,detections[i]);
			
	} 
	for(int i=0;i < nelem; i++){
			LOG_D("input elements are %d=%f",i,inval[i]);
	}
	
	LOG_D("maxProposalCount=%d,objectSize=%d",maxProposalCount,objectSize);
    for (int i = 0; i < maxProposalCount; i++) {
        float image_id = detections[i * objectSize + 0];
        //LOG_D("image_id is =%f",image_id);
		if (image_id < 0) {
            break;
        }
		
        Result r;
        r.label = static_cast<int>(detections[i * objectSize + 1]);
        r.confidence = detections[i * objectSize + 2];

        /* if (r.confidence <= detectionThreshold) {
            continue;
        } */

        r.location.x = static_cast<int>(detections[i * objectSize + 3] * width);
        r.location.y = static_cast<int>(detections[i * objectSize + 4] * height);
        r.location.width = static_cast<int>(detections[i * objectSize + 5] * width - r.location.x);
        r.location.height = static_cast<int>(detections[i * objectSize + 6] * height - r.location.y);

        // Make square and enlarge face bounding box for more robust operation of face analytics networks
        int bb_width = r.location.width;
        int bb_height = r.location.height;

        int bb_center_x = r.location.x + bb_width / 2;
        int bb_center_y = r.location.y + bb_height / 2;

        int max_of_sizes = std::max(bb_width, bb_height);

        int bb_new_width = static_cast<int>(bb_enlarge_coefficient * max_of_sizes);
        int bb_new_height = static_cast<int>(bb_enlarge_coefficient * max_of_sizes);

        r.location.x = bb_center_x - static_cast<int>(std::floor(bb_dx_coefficient * bb_new_width / 2));
        r.location.y = bb_center_y - static_cast<int>(std::floor(bb_dy_coefficient * bb_new_height / 2));

        r.location.width = bb_new_width;
        r.location.height = bb_new_height;
		
		LOG_D("label=%d,x=%d,y=%d,width=%d,height=%d,conf=%f,thr=%f",r.label,r.location.x,r.location.y,r.location.width,r.location.height,r.confidence,detectionThreshold);
        
		
        if (r.confidence > detectionThreshold) {
            results.push_back(r);
        }
    }
}




Load::Load(BaseDetection& detector) : detector(detector) {
}

void Load::into(InferencePlugin & plg, bool enable_dynamic_batch) const {
    if (detector.enabled()) {
        std::map<std::string, std::string> config;
        std::string pluginName = plg.GetVersion()->description;
        bool isPossibleDynBatch = pluginName.find("MKLDNN") != std::string::npos ||
                                  pluginName.find("clDNN") != std::string::npos;
        if (enable_dynamic_batch && isPossibleDynBatch) {
            config[PluginConfigParams::KEY_DYN_BATCH_ENABLED] = PluginConfigParams::YES;
        }
        detector.net = plg.LoadNetwork(detector.read(), config);
        detector.plugin = &plg;
    }
}


CallStat::CallStat():
    _number_of_calls(0), _total_duration(0.0), _last_call_duration(0.0), _smoothed_duration(-1.0) {
}

double CallStat::getSmoothedDuration() {
    // Additional check is needed for the first frame while duration of the first
    // visualisation is not calculated yet.
    if (_smoothed_duration < 0) {
        auto t = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<ms>(t - _last_call_start).count();
    }
    return _smoothed_duration;
}

double CallStat::getTotalDuration() {
    return _total_duration;
}

double CallStat::getLastCallDuration() {
    return _last_call_duration;
}

void CallStat::calculateDuration() {
    auto t = std::chrono::high_resolution_clock::now();
    _last_call_duration = std::chrono::duration_cast<ms>(t - _last_call_start).count();
    _number_of_calls++;
    _total_duration += _last_call_duration;
    if (_smoothed_duration < 0) {
        _smoothed_duration = _last_call_duration;
    }
    double alpha = 0.1;
    _smoothed_duration = _smoothed_duration * (1.0 - alpha) + _last_call_duration * alpha;
}

void CallStat::setStartTime() {
    _last_call_start = std::chrono::high_resolution_clock::now();
}


void Timer::start(const std::string& name) {
    if (_timers.find(name) == _timers.end()) {
        _timers[name] = CallStat();
    }
    _timers[name].setStartTime();
}

void Timer::finish(const std::string& name) {
    auto& timer = (*this)[name];
    timer.calculateDuration();
}

CallStat& Timer::operator[](const std::string& name) {
    if (_timers.find(name) == _timers.end()) {
        throw std::logic_error("No timer with name " + name + ".");
    }
    return _timers[name];
}
