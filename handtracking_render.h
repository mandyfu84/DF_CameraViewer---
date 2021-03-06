/*******************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2013-2014 Intel Corporation. All Rights Reserved.

*******************************************************************************/
#pragma once
#include <list>
#include <map>
#include "pxcsensemanager.h"
#include <pxchandmodule.h>
#include "util_render.h"
#include <vector>

class HandRender: public UtilRender {
public:
	HandRender(pxcCHAR *title=0):UtilRender(title) {
		
		leftHandSideText = L"Left Hand";
		rightHandSideText = L"Right Hand";
		leftGesture = L"";
		rightGesture = L"";

	};

	void HandRender::DrawJoints(PXCHandData::JointData nodes[2][PXCHandData::NUMBER_OF_JOINTS]);
	void NotifyGestures(PXCHandData::BodySideType, pxcCHAR*);
	void NotifyAlerts(PXCHandData::AlertType);
	//void DrawPlayBack();


protected:

	virtual void DrawMore(HDC hdc, double scale_x, double scale_y);

	struct Line {
        int x0, y0;
        int x1, y1;
    };

    struct Node {
        int x, y;
    };

    std::list<Line>     m_lines;
	std::list<Node>     m_nodes;

	/*Add*/
	struct MyHand {
		std::list<Line>     hand_lines;
		std::list<Node>     hand_nodes;
	};
	std::list<MyHand>     myhand_list;


	std::wstring leftGesture;
	std::wstring rightGesture;

	std::wstring leftHandSideText;
	std::wstring rightHandSideText;

};

