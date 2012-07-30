/**
   MiracleGrue - Model Generator for toolpathing. <http://www.grue.makerbot.com>
   Copyright (C) 2011 Far McKon <Far@makerbot.com>, Hugo Boyer (hugo@makerbot.com)

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as
   published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

 */


#include <iostream>
#include <sstream>
#include <fstream>


#include "configuration.h"
#include "gcoder.h"
#include "pather.h"
#include "abstractable.h"
#include "regioner.h"

namespace mgl {

using namespace std;

string Configuration::defaultFilename() {
	MyComputer computer;
	string file = computer.fileSystem.getConfigFile("miracle.config");

	if (file.length() > 0)
		return file;
	else
		return string("miracle.config");
}

double doubleCheck(const Json::Value &value, const char *name) {
	if (value.isNull()) {
		stringstream ss;
		ss << "Missing required floating point field \"" <<
				name << "\" in configuration file";
		ConfigException mixup(ss.str().c_str());
		throw mixup;
	}
	return value.asDouble();
}

double doubleCheck(const Json::Value &value, const char *name, 
		const double defaultVal) {
	if (value.isNull()) {
		return defaultVal;
	} else {
		return doubleCheck(value, name);
	}
}

unsigned int uintCheck(const Json::Value &value, const char *name) {
	if (value.isNull()) {
		stringstream ss;
		ss << "Missing required unsigned integer field \"" << 
				name << "\" in configuration file";
		ConfigException mixup(ss.str().c_str());
		throw mixup;
	}
	return value.asUInt();
}

unsigned int uintCheck(const Json::Value &value, const char *name, 
		const unsigned int defaultVal) {
	if (value.isNull()) {
		return defaultVal;
	} else {
		return uintCheck(value, name);
	}
}

string pathCheck(const Json::Value &value, const char *name,
		const std::string &defaultval) {
	if (value.isNull())
		return defaultval;
	else
		return pathCheck(value, name);
}

string pathCheck(const Json::Value &value, const char *name) {
	string result = stringCheck(value, name);
	if (result.substr(0, 10) == "default://") {
		MyComputer computer;
		result = computer.fileSystem.getDataFile(result.substr(10).c_str());
	}

	return result;
}

string stringCheck(const Json::Value &value, const char *name) {
	if (value.isNull()) {
		stringstream ss;
		ss << "Missing required string field \"" << 
				name << "\" in configuration file";
		ConfigException mixup(ss.str().c_str());
		throw mixup;
	}
	return value.asString();
}

bool boolCheck(const Json::Value &value, const char *name,
		const bool defaultval) {
	if (value.isNull())
		return defaultval;
	else
		return boolCheck(value, name);
}

bool boolCheck(const Json::Value &value, const char *name) {
	if (value.isNull()) {
		stringstream ss;
		ss << "Missing required string field \"" << 
				name << "\" in configuration file";
		ConfigException mixup(ss.str().c_str());
		throw mixup;
	}
	return value.asBool();
}

Configuration::Configuration()
: filename("") {
}

void Configuration::readFromFile(const char* filename) {
	this->filename = filename;
	std::ifstream file(filename);
	if (file) {
		Json::Reader reader;
		reader.parse(file, root);
	} else {
		string msg = "Config file: \"";
		msg += filename;
		msg += "\" can't be found";
		ConfigException mixup(msg.c_str());
		throw mixup;
	}

}

Configuration::~Configuration() {
	// not sure we need to clean up here
	// this->root.clear();
}

std::string Configuration::asJson(Json::StyledWriter writer) const {
	return writer.write(root);
}

// this is a work in progress...

void loadExtrusionProfileData(const Configuration& conf, GCoderConfig &gcoderCfg) {
	const Json::Value &extrusionsRoot = conf.root["extrusionProfiles"];
	for (Json::ValueIterator itr = extrusionsRoot.begin(); 
			itr != extrusionsRoot.end(); itr++) {
		string profileName = itr.key().asString();
		stringstream ss;
		ss << "extrusionProfiles[\"" << profileName << "\"].";
		string prefix = ss.str();
		const Json::Value &value = *itr;
		Extrusion extrusion;
		extrusion.feedrate = doubleCheck(value["feedrate"], 
				(prefix + "feedrate").c_str());

		extrusion.retractDistance = doubleCheck(value["retractDistance"], (prefix + "retractDistance").c_str());
		extrusion.retractRate = uintCheck(value["retractRate"], 
				(prefix + "retractRate").c_str());
		extrusion.restartExtraDistance = 
				doubleCheck(value["restartExtraDistance"], 
				(prefix + "restartExtraDistance").c_str());

		extrusion.flow = doubleCheck(value["flow"], 
				(prefix + "flow").c_str());
		extrusion.leadIn = doubleCheck(value["leadIn"], 
				(prefix + "leadIn").c_str());
		extrusion.leadOut = doubleCheck(value["leadOut"], 
				(prefix + "leadOut").c_str());
		extrusion.snortFeedrate = doubleCheck(value["snortFeedrate"], 
				(prefix + "snortFeedrate").c_str());
		extrusion.snortFlow = doubleCheck(value["snortFlow"], 
				(prefix + "snortFlow").c_str());
		extrusion.squirtFeedrate = doubleCheck(value["squirtFeedrate"], 
				(prefix + "squirtFeedrate").c_str());
		extrusion.squirtFlow = doubleCheck(value["squirtFlow"], 
				(prefix + "squirtFlow").c_str());

		gcoderCfg.extrusionProfiles.insert(pair<std::string, 
				Extrusion > (profileName, extrusion));
	}
}

void loadExtruderConfigFromFile(const Configuration& conf,
		ExtruderConfig &extruderCfg) {
	extruderCfg.defaultExtruder = 
			uintCheck(conf.root["extruder"]["defaultExtruder"], 
			"extruder.defaultExtruder");
}

void loadGCoderConfigFromFile(const Configuration& conf, 
		GCoderConfig &gcoderCfg) {

	gcoderCfg.programName = stringCheck(conf.root["programName"], 
			"programName");
	gcoderCfg.versionStr = stringCheck(conf.root["versionStr"], 
			"versionStr");
	gcoderCfg.machineName = stringCheck(conf.root["machineName"], 
			"machineName");
	gcoderCfg.firmware = stringCheck(conf.root["firmware"], 
			"firmware");


	gcoderCfg.gantryCfg.set_xy_max_homing(boolCheck(
			conf.root["gantry"]["xyMaxHoming"], "gantry.xyMaxHoming"));
	gcoderCfg.gantryCfg.set_z_max_homing(boolCheck(
			conf.root["gantry"]["zMaxHoming" ], "gantry.zMaxHoming"));
	gcoderCfg.gantryCfg.set_scaling_Factor(doubleCheck(
			conf.root["gantry"]["scalingFactor"],
			"gantry.scalingFactor"));
	gcoderCfg.gantryCfg.set_rapid_move_feed_rate_xy(doubleCheck(
			conf.root["gantry"]["rapidMoveFeedRateXY"],
			"gantry.rapidMoveFeedRateXY"));
	gcoderCfg.gantryCfg.set_rapid_move_feed_rate_z(doubleCheck(
			conf.root["gantry"]["rapidMoveFeedRateZ"],
			"gantry.rapidMoveFeedRateZ"));
	gcoderCfg.gantryCfg.set_homing_feed_rate_z(doubleCheck(
			conf.root["gantry"]["homingFeedRateZ"],
			"gantry.homingFeedRateZ"));
	gcoderCfg.gantryCfg.set_layer_h(doubleCheck(
			conf.root["slicer"]["layerH"], "slicer.layerH"));
	gcoderCfg.gantryCfg.set_start_x(doubleCheck(
			conf.root["gantry"]["startX"], "gantry.startX"));
	gcoderCfg.gantryCfg.set_start_y(doubleCheck(
			conf.root["gantry"]["startY"], "gantry.startY"));
	gcoderCfg.gantryCfg.set_start_z(doubleCheck(
			conf.root["gantry"]["startZ"], "gantry.startZ"));


	gcoderCfg.platform.temperature = 
			doubleCheck(conf.root["platform"]["temperature"], 
			"platform.temperature");
	gcoderCfg.platform.automated = 
			boolCheck(conf.root["platform"]["automated"], 
			"platform.automated");

	gcoderCfg.outline.enabled = 
			boolCheck(conf.root["outline"]["enabled"], 
			"outline.enabled");
	gcoderCfg.outline.distance = 
			doubleCheck(conf.root["outline"]["distance"], 
			"outline.distance");

	loadExtrusionProfileData(conf, gcoderCfg);

	if (conf.root["extruderProfiles"].size() == 0) {
		stringstream ss;
		ss << "No extruder defined in the configuration file";
		ConfigException mixup(ss.str().c_str());
		throw mixup;
		return;
	}

	int x = conf.root["extruderProfiles"].size();
	unsigned int extruderCount = (unsigned int) x;
	for (unsigned int i = 0; i < extruderCount; i++) {
		const Json::Value &value = conf.root["extruderProfiles"][i];

		stringstream ss;
		ss << "extruders[" << i << "].";
		string prefix = ss.str();

		Extruder extruder;
		extruder.coordinateSystemOffsetX = 
				doubleCheck(value["coordinateSystemOffsetX"], 
				(prefix + "coordinateSystemOffsetX").c_str());
		extruder.extrusionTemperature = 
				doubleCheck(value["extrusionTemperature"], 
				(prefix + "extrusionTemperature").c_str());
		extruder.nozzleZ = doubleCheck(value["nozzleZ"], 
				(prefix + "nozzleZ").c_str());
		extruder.zFeedRate = doubleCheck(value["zFeedRate"], 
				(prefix + "zFeedRate").c_str());

		extruder.firstLayerExtrusionProfile = 
				stringCheck(value["firstLayerExtrusionProfile"], 
				(prefix + "firstLayerExtrusionProfile").c_str());
		extruder.insetsExtrusionProfile = 
				stringCheck(value["insetsExtrusionProfile"], 
				(prefix + "insetsExtrusionProfile").c_str());
		extruder.infillsExtrusionProfile = 
				stringCheck(value["infillsExtrusionProfile"], 
				(prefix + "infillsExtrusionProfile").c_str());

		extruder.id = i;
		extruder.code = 'A' + i;

		string extrusionMode = stringCheck(value["extrusionMode"], 
				(prefix + "extrusionMode").c_str());
		if (extrusionMode == "rpm") {
			extruder.extrusionMode = Extruder::RPM_MODE;
		} else if (extrusionMode == "volumetric") {
			extruder.extrusionMode = Extruder::VOLUMETRIC_MODE;
		} else {
			stringstream ss;
			ss << "Invalid extrusion mode [" << extrusionMode << "]";
			ConfigException mixup(ss.str().c_str());
			throw mixup;
			return;
		}

		extruder.feedDiameter = doubleCheck(value["feedDiameter"], 
				(prefix + "feedDiameter").c_str());
		gcoderCfg.extruders.push_back(extruder);
	}

	gcoderCfg.header = pathCheck(conf.root["gcoder"]["header"], 
			"gcoder.header", "");
	gcoderCfg.footer = pathCheck(conf.root["gcoder"]["footer"], 
			"gcoder.footer", "");
	gcoderCfg.doOutlines = boolCheck(conf.root["gcoder"]["outline"], 
			"gcoder.outline");
	gcoderCfg.doInsets = boolCheck(conf.root["gcoder"]["insets"], 
			"gcoder.insets");
	gcoderCfg.doInfillsFirst = boolCheck(conf.root["gcoder"]["infillFirst"], 
			"gcoder.infillFirst");
	gcoderCfg.doInfills = boolCheck(conf.root["gcoder"]["infills"], 
			"gcoder.infills");
	gcoderCfg.doSupport = boolCheck(conf.root["gcoder"]["support"],
                                    "gcoder.support");
	gcoderCfg.doPrintLayerMessages = boolCheck(
			conf.root["gcoder"]["printLayerMessages"],
			"gcoder.printLayerMessages", false);
	gcoderCfg.gantryCfg.set_use_e_axis(boolCheck(
			conf.root["gcoder"]["useEAxis"],
			"gcoder.useEAxis", false));

}

void loadSlicerConfigFromFile(const Configuration &config, 
		SlicerConfig &slicerCfg) {
	//Relevant to slicer
	slicerCfg.layerH = doubleCheck(config["slicer"]["layerH"], 
			"slicer.layerH");
	slicerCfg.firstLayerZ = doubleCheck(config["slicer"]["firstLayerZ"], 
			"slicer.firstLayerZ");
}

void loadRegionerConfigFromFile(const Configuration& config, 
		RegionerConfig& regionerCfg) {
	//Relevant to regioner
	regionerCfg.infillDensity = doubleCheck(config["regioner"]["infillDensity"], 
			"regioner.infillDensity");
	//slicerCfg.angle 		= doubleCheck(config["slicer"]["angle"], "slicer.angle");
	regionerCfg.nbOfShells = uintCheck(config["regioner"]["nbOfShells"], 
			"regioner.nbOfShells");
	regionerCfg.layerWidthRatio = doubleCheck(config["regioner"]["layerWidthRatio"], 
			"regioner.layerWidthRatio");
	regionerCfg.infillShrinkingMultiplier = 
			doubleCheck(config["regioner"]["infillShrinkingMultiplier"], 
			"regioner.infillShrinkingMultiplier");
	regionerCfg.insetDistanceMultiplier = 
			doubleCheck(config["regioner"]["insetDistanceMultiplier"], 
			"regioner.insetDistanceMultiplier");
	regionerCfg.insetCuttOffMultiplier = 
			doubleCheck(config["regioner"]["insetCuttOffMultiplier"], 
			"regioner.insetCuttOffMultiplier");

	regionerCfg.roofLayerCount = 
			doubleCheck(config["regioner"]["roofLayerCount"], 
			"regioner.roofLayerCount");
	regionerCfg.floorLayerCount = 
			doubleCheck(config["regioner"]["roofLayerCount"], 
			"regioner.floorLayerCount");

	regionerCfg.writeDebugScadFiles = 
			boolCheck(config["regioner"]["writeDebugScadFiles"], 
			"regioner.writeDebugScadFiles", false);
	//Rafting Configuration
	regionerCfg.raftLayers = uintCheck(config["regioner"]["raftLayers"], 
			"regioner.raftLayers", regionerCfg.raftLayers);
	regionerCfg.raftBaseThickness = doubleCheck(
			config["regioner"]["raftBaseThickness"], 
			"regioner.raftBaseThickness", regionerCfg.raftBaseThickness);
	regionerCfg.raftInterfaceThickness = doubleCheck(
			config["regioner"]["raftInterfaceThickness"], 
			"regioner.raftInterfaceThickness",
			regionerCfg.raftInterfaceThickness);
	regionerCfg.raftOutset = doubleCheck(
			config["regioner"]["raftOutset"], 
			"regioner.raftOutset", regionerCfg.raftOutset);
    regionerCfg.doSupport = boolCheck(config["regioner"]["doSupport"],
                                      "regioner.doSupport", 
                                      regionerCfg.doSupport);
    regionerCfg.supportMargin = doubleCheck(config["regioner"]["supportMargin"],
                                            "regioner.supportMargin",
                                            regionerCfg.supportMargin);
                                            
}


}


