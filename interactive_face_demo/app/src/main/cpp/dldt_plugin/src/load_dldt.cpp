// Copyright (C) 2018-2019 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

/**
* \brief The entry point for the Inference Engine interactive_face_detection demo application
* \file interactive_face_detection_demo/main.cpp
* \example interactive_face_detection_demo/main.cpp
*/

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
#include <list>

#include "detectors.hpp"
#include "face.hpp"
#include "ie_common.h"
#include "ie_iextension.h"
#include "ie_plugin_cpp.hpp"
#include "ie_plugin_dispatcher.hpp"
#include "ie_version.hpp"
#include "load_dldt.hpp"



using namespace InferenceEngine;

inline void printPluginVersion(InferenceEngine::InferenceEnginePluginPtr ptr) {
    const InferenceEngine::Version *pluginVersion = nullptr;
    ptr->GetVersion(pluginVersion);
    LOG_D("Inference Plugin Version is %d-%d" , pluginVersion->apiVersion.major, pluginVersion->apiVersion.minor);
    LOG_D("Inference Plugin build number=%s,description=%s" , pluginVersion->buildNumber, pluginVersion->description);
}

FaceDetection gFaceDetector;

/* gFaceDetector.Result.label = 0;
gFaceDetector.Result.confidence = 0;
gFaceDetector.Result.location = cv::Rect(0,0,250,250);
gFaceDetector.width = 0;
gFaceDetector.height = 0; */

static void setConfig(std::map<std::string, std::string> &config){
	return;
}
//Loading plugin to the Inference Engine 
int initDetector(const char* DeviceName,const char* ModelPath) {
         
    
    LOG_D("Loading plugin for device %s and models from %s",DeviceName,ModelPath);
    
	InferencePlugin plugin = PluginDispatcher().getPluginByDevice(DeviceName);
	//std::map<std::string, std::string> networkConfig;
	
	plugin.AddExtension(std::make_shared<Extensions::Cpu::CpuExtensions>());
	
		
    gFaceDetector.topoName = "Face Detection";
	gFaceDetector.pathToModel = "/data/local/dldtmodels/face-detection-adas-0001.xml";
	//gFaceDetector.pathToModel = "/data/local/dldtmodels/alexnet.xml";
	gFaceDetector.deviceForInference = "CPU";
    gFaceDetector.isAsync = 1;
	gFaceDetector.maxBatch = 1;  
	
	
	//Load all the Plugins
	 Load(gFaceDetector).into(plugin, false);	
    
	return 0;
}   

void DoFaceDetection(const cv::Mat &frame, std::vector<cv::Rect> RectFaces){
	
	LOG_D("Doing Face Detection with framecount=%d",++gFaceDetector.framesCounter);
	
	/* const size_t width  = static_cast<size_t>(frame.cols);
    const size_t height = static_cast<size_t>(frame.rows); */
	
	gFaceDetector.enqueue(frame);		
	gFaceDetector.submitRequest();
	
	// Retrieving face detection results for the previous frame
	gFaceDetector.wait();
	gFaceDetector.fetchResults();
	auto detection_results = gFaceDetector.results;
	
	for (size_t i = 0; i < detection_results.size(); i++) {
		auto& result = detection_results[i];
		RectFaces.push_back(result.location);
	}
	
	return ;
	/* //  Postprocessing
	std::list<Face::Ptr> prev_faces;
    
	prev_faces.insert(prev_faces.begin(), faces.begin(), faces.end());
	
	faces.clear();

	// For every detected face
	for (size_t i = 0; i < detection_results.size(); i++) {
		auto& result = detection_results[i];
		cv::Rect rect = result.location & cv::Rect(0, 0, width, height);

		Face::Ptr face;
		face = matchFace(rect, prev_faces);
		float intensity_mean = calcMean(prev_frame(rect));

		if ((face == nullptr) ||
			((face != nullptr) && ((std::abs(intensity_mean - face->_intensity_mean) / face->_intensity_mean) > 0.07f))) {
			face = std::make_shared<Face>(id++, rect);
		} else {
			prev_faces.remove(face);
		}

		face->_intensity_mean = intensity_mean;
		face->_location = rect;
			
		faces.push_back(face);
		} */
					
}
