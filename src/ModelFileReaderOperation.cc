/**
   MiracleGrue - Model Generator for toolpathing. <http://www.grue.makerbot.com>
   Copyright (C) 2011 Far McKon <Far@makerbot.com>, Hugo Boyer (hugo@makerbot.com)

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as
   published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

*/
#include <assert.h>
#include <sstream>

#include "ModelFileReaderOperation.h"

using namespace std;

ModelFileReaderOperation::ModelFileReaderOperation()
	:pStream(NULL)
{
	cout << "ModelFileReaderOperation() @"  << this<< endl;
}

ModelFileReaderOperation::~ModelFileReaderOperation()
{
	cout << "~ModelFileReaderOperation() @"  << this<< endl;
}

void ModelFileReaderOperation::processEnvelope(const DataEnvelope& envelope)
{
	// this method must exist because it is declared pure virutalin Operation
	// however the ModelFileReader does not receive any data so...
	assert(0); // this method should not be called

}

std::istream& ModelFileReaderOperation::stream() const
{
	assert(pStream);
	return *(pStream);
}


void ModelFileReaderOperation::start()
{
	const Configuration &config = configuration();
	pStream = new std::ifstream("filename.stl");
	//std::cout << "Writing to file: \"" << config.gcodeFilename << "\""<< std::endl;
	std::cout << "Reading From file \"" << "filename.stl" << "\""<< std::endl;
	readFromStl(srcfile);
}


void ModelFileReaderOperation::start(std::string &srcFile)
{
	const Configuration &config = configuration();
	//pStream = new std::ifstream(srcFile);
	std::cout << "Reading From file \"" << srcFile << "\""<< std::endl;
	DataEnvelope data = readFromStl(srcfile);
	data->lastFlag = true;
	 //pass data down the process train
	this->emit(data);
}


DataEnvelope& ModelFileReaderOperation::readFromStl(std::string srcFile)
{

	Mesh3d mesh = new Mesh3d();
	mesh.loadFromSTLFile(const char *fileName)
	DataEnvelope data = MeshEnvelope(mesh);
	return &data;
}