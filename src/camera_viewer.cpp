#include <stdio.h>
#include <pxcsession.h>
#include <pxcsensemanager.h>
#include <pxccapture.h>
#include "util_render.h"


int main()
{
	
	UtilRender renderc(L"Color123");
	
	// Create a SenseManager instance
	PXCSenseManager *sm = PXCSenseManager::CreateInstance();

	// Enable the video stream
	sm->EnableStream(PXCCapture::STREAM_TYPE_COLOR, 0, 0, 0);

	// Initialization
	sm->Init();

	// Stream data
	while (sm->AcquireFrame(true) >= PXC_STATUS_NO_ERROR) {
		// Get the sample data
		PXCCapture::Sample *sample = sm->QuerySample();

		// process image sample->color
		renderc.RenderFrame(sample->color);

		// Resume next frame processing
		sm->ReleaseFrame();
	}

	// Clean up
	sm->Release();

	return 0;
}

