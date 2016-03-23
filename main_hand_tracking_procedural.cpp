/*******************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2013-2014 Intel Corporation. All Rights Reserved.

*******************************************************************************/
/* 
Description:
This is the hand tracking and gesture recognition procedural sample that shows how to enable tracking the joints , alert tracking and gesture recognition.

*/

#include <windows.h>
#include "pxcsensemanager.h"
#include "pxchandconfiguration.h"
#include "handtracking_render.h" 
#include "Definitions.h"
#include <iostream>
#include <string>
#include<fstream>
#include <pxccapture.h>
#include <pxcsession.h>



int wmain(int argc, WCHAR* argv[]) {

	int mode = 0;
	printf("1)Real time mode  2)Play back mode ....>");
	scanf("%d", &mode);
	
	if (mode == 1)
	{
		//File
		FILE *fp = fopen("data.txt", "w");

		// error checking Status
		pxcStatus sts;

		// initialize the util render 
		HandRender *renderer = new HandRender(L"PROCEDURAL HAND TRACKING");

		// create the PXCSenseManager
		PXCSenseManager *psm = 0;
		psm = PXCSenseManager::CreateInstance();
		if (!psm) {
			wprintf_s(L"Unable to create the PXCSenseManager\n");
			return 1;
		}

		// enable hand analysis in the multimodal pipeline
		sts = psm->EnableHand();
		if (sts < PXC_STATUS_NO_ERROR) {
			wprintf_s(L"Unable to enable Hand Tracking\n");
			return 2;
		}

		// retrieve hand module if ready - called in the setup stage before AcquireFrame
		PXCHandModule* handAnalyzer = psm->QueryHand();
		if (!handAnalyzer) {
			wprintf_s(L"Unable to retrieve hand results\n");
			return 3;
		}

		// initialize the PXCSenseManager
		if (psm->Init() < PXC_STATUS_NO_ERROR) return 4;

		// retrieves an instance of the PXCHandData interface
		PXCHandData* outputData = handAnalyzer->CreateOutput();


		// retrieves an instance of the PXCHandData PXCHandConfiguration
		PXCHandConfiguration* config = handAnalyzer->CreateActiveConfiguration();

		// enable or disable features in hand module
		config->EnableAlert(PXCHandData::AlertType::ALERT_HAND_DETECTED);
		config->EnableAlert(PXCHandData::AlertType::ALERT_HAND_NOT_DETECTED);
		config->EnableAllGestures();

		//make the config changes effective
		config->ApplyChanges();


		/*ADD mirror*/
		PXCCaptureManager *cm = psm->QueryCaptureManager();
		PXCCapture::Device *device = cm->QueryDevice();
		device->SetMirrorMode(PXCCapture::Device::MirrorMode::MIRROR_MODE_HORIZONTAL);

		// stream data
		PXCImage *depthIm = NULL; //init depth im

		while (psm->AcquireFrame(true) >= PXC_STATUS_NO_ERROR) {

			//update the output data to the latest availible
			outputData->Update();



			// create data structs for storing data
			PXCHandData::JointData nodes[2][PXCHandData::NUMBER_OF_JOINTS] = {};

			// iterate through hands
			int numOfHands = outputData->QueryNumberOfHands();


			for (int i = 0; i < numOfHands; i++)
			{
				// get hand joints by time of appearence
				PXCHandData::IHand* handData;
				if (outputData->QueryHandData(PXCHandData::ACCESS_ORDER_BY_TIME, i, handData) == PXC_STATUS_NO_ERROR)
				{
					std::string handSide = "Unknown Hand";
					handSide = handData->QueryBodySide() == PXCHandData::BODY_SIDE_LEFT ? "Left Hand" : "Right Hand";
					std::printf("%s\n==============\n", handSide.c_str());
					std::fprintf(fp, "%s\n==============\n", handSide.c_str());


					PXCHandData::JointData jointData;
					// iterate through Joints and get joint data 
					for (int j = 0; j < PXCHandData::NUMBER_OF_JOINTS; j++)
					{
						handData->QueryTrackedJoint((PXCHandData::JointType)j, jointData);
						nodes[i][j] = jointData;
						std::printf("     %s)\tX: %f Y: %f Z: %f %f %f\n", Definitions::JointToString((PXCHandData::JointType)j).c_str(), jointData.positionWorld.x, jointData.positionWorld.y, jointData.positionWorld.z,  jointData.positionImage.x, jointData.positionImage.y);
						std::fprintf(fp, "     %s)\tX: %f Y: %f Z: %f %f %f\n", Definitions::JointToString((PXCHandData::JointType)j).c_str(), jointData.positionWorld.x, jointData.positionWorld.y, jointData.positionWorld.z, jointData.positionImage.x, jointData.positionImage.y);
					}
				}
			}

			//render Joints
			renderer->DrawJoints(nodes);

			int numOfGestures = outputData->QueryFiredGesturesNumber();
			if (numOfGestures > 0) {
				// iterate through fired gestures
				for (int i = 0; i < numOfGestures; i++) {
					// initialize data
					PXCHandData::GestureData gestureData;
					// get fired gesture data
					if (outputData->QueryFiredGestureData(i, gestureData) == PXC_STATUS_NO_ERROR) {
						// get hand data related to fired gesture
						PXCHandData::IHand* handData;
						if (outputData->QueryHandDataById(gestureData.handId, handData) == PXC_STATUS_NO_ERROR) {
							// save gesture only if you know that its right/left hand
							if (handData->QueryBodySide() != PXCHandData::BodySideType::BODY_SIDE_UNKNOWN)
								renderer->NotifyGestures(handData->QueryBodySide(), gestureData.name);
						}

					}
				}
			}

			// iterate through Alerts 
			PXCHandData::AlertData alertData;
			for (int i = 0; i < outputData->QueryFiredAlertsNumber(); i++)
			{
				sts = outputData->QueryFiredAlertData(i, alertData);
				if (sts == PXC_STATUS_NO_ERROR)
				{
					// Display last alert - see AlertData::Label for all available alerts
					renderer->NotifyAlerts(alertData.label);

				}
			}



			// retrieve all available image samples
			PXCCapture::Sample *sample = psm->QueryHandSample();

			// retrieve the image or frame by type
			depthIm = sample->depth;

			// render the frame
			if (!renderer->RenderFrame(depthIm)) break;

			// release or unlock the current frame to go fetch the next frame
			psm->ReleaseFrame();
		}

		// delete the HandRender instance
		delete renderer;

		//delete the configuration
		config->Release();

		// close the last opened stream and release any session and processing module instances
		psm->Release();

		fclose(fp);
	}
	else if (mode == 2)
	{
		
		std::fstream fp;
		fp.open("data1.txt", std::fstream::in);
		
		HandRender *renderer = new HandRender(L"PROCEDURAL HAND PLAYBACK");


		//UtilRender renderc(L"Color123");
		// Create a SenseManager instance
		PXCSenseManager *sm = PXCSenseManager::CreateInstance();

		// Enable the video stream
		sm->EnableStream(PXCCapture::STREAM_TYPE_DEPTH, 0, 0, 0);

		// Initialization
		sm->Init();

		//// Stream data
		//while (sm->AcquireFrame(true) >= PXC_STATUS_NO_ERROR) {
		//	// Get the sample data
		//	PXCCapture::Sample *sample = sm->QuerySample();

		//	// process image sample->color
		//	renderer->RenderFrame(sample->depth);

		//	// Resume next frame processing
		//	sm->ReleaseFrame();
		//}

		//// Clean up
		//sm->Release();


		PXCHandData::JointData nodes[2][PXCHandData::NUMBER_OF_JOINTS] = {};
		// iterate through hands
		
		int numOfHands = 0;
		
		std::string handSide = "Unknown Hand", tmp;		


		while ( getline(fp, handSide))
		{
			// Get the sample data
			PXCCapture::Sample *sample = sm->QuerySample();

			// process image sample->color
			//for (int i = 0; i < numOfHands; i++)
			//{
				// get hand joints by time of appearence
			PXCHandData::IHand* handData;

			std::getline(fp, tmp);
			std::cout << handSide << std::endl << tmp << std::endl;

			PXCHandData::JointData jointData;
			// iterate through Joints and get joint data
			for (int j = 0; j < PXCHandData::NUMBER_OF_JOINTS; j++)
			{
				fp >> tmp >> tmp >> jointData.positionWorld.x >> tmp >> jointData.positionWorld.y >> tmp >> jointData.positionWorld.z >> jointData.positionImage.x >> jointData.positionImage.y;
				nodes[0][j] = jointData;
				nodes[0][j].confidence = 100;
				std::printf("     %s)\tX: %f Y: %f Z: %f %f %f\n", Definitions::JointToString((PXCHandData::JointType)j).c_str(), jointData.positionWorld.x, jointData.positionWorld.y, jointData.positionWorld.z, jointData.positionImage.x, jointData.positionImage.y);
			}
			fp >> tmp;

			renderer->DrawJoints(nodes);
			renderer->RenderFrame(sample->depth);

			sm->ReleaseFrame();
			Sleep(500);
			//}
		}
		system("pause");

		delete renderer;
		fp.close();
		//sm->Release();
		sm->Release();

	}

	return 0;
}