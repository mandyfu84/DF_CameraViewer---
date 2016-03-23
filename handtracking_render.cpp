/*******************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2013-2014 Intel Corporation. All Rights Reserved.

*******************************************************************************/
#include <windows.h>
#include "handtracking_render.h"

#define MAX_NUM_OF_HANDS 2
#define TEXT_HEIGHT 18
#define JOINT_CIRCLE_RADIUS 1

#define COLOR_BLUE        RGB(51,51,255)
#define COLOR_GREEN      RGB(0,255,0)
#define COLOR_RED      RGB(255,0,0)
#define COLOR_YELLOW		RGB(255,255, 0)

void HandRender::NotifyAlerts(PXCHandData::AlertType label)
{
	switch(label)
	{
	case PXCHandData::ALERT_HAND_DETECTED:
		wprintf_s(L"Last Alert: Hand Detected\n");
		break;
	case PXCHandData::ALERT_HAND_NOT_DETECTED:
		wprintf_s(L"Last Alert: Hand Not Detected\n");
		leftGesture = L"";
		rightGesture = L"";
		break;
	}
}

void HandRender::DrawJoints(PXCHandData::JointData nodes[2][PXCHandData::NUMBER_OF_JOINTS]) 
{
	for (int i = 0; i < MAX_NUM_OF_HANDS; ++i) 
	{
		//Draw Joints
		for (int j = 0; j < PXCHandData::NUMBER_OF_JOINTS; ++j)  {
			if (nodes[i][j].confidence == 0) continue;

			int x=(int)nodes[i][j].positionImage.x ;
			int y=(int)nodes[i][j].positionImage.y ;
			
			
			//printf("x=%d y=%d   ", x,y);


			Node node={(int)x,(int)y};
			m_nodes.push_back(node);
		}

		MyHand    myhand_tmp;
		myhand_tmp.hand_nodes = m_nodes;
		myhand_list.push_back(myhand_tmp);
		if (myhand_list.size() > 10) myhand_list.pop_front();


		int wristX=(int)nodes[i][0].positionImage.x;
		int wristY=(int)nodes[i][0].positionImage.y;
		for (int j = 2 ;j < (PXCHandData::NUMBER_OF_JOINTS-1); j++) 			
		{
			if(nodes[i][j].confidence == 0) continue;

			if(j == 5 || j == 9 || j == 13 || j == 17)
			{
				continue;
			}

			if(j == 2 || j == 6 || j == 10 || j == 14 || j == 18)
			{
				int x0=(int)nodes[i][0].positionImage.x;
				int y0=(int)nodes[i][0].positionImage.y;
				int x1=(int)nodes[i][j].positionImage.x;
				int y1=(int)nodes[i][j].positionImage.y;

				if (x0 > 0 && y0 > 0  && x1 > 0 && y1 > 0 ) {
					Line line={(int)x0,(int)y0,(int)x1,(int)y1};
					m_lines.push_back(line);
				}
			}

			int x0=(int)nodes[i][j].positionImage.x;
			int y0=(int)nodes[i][j].positionImage.y;
			int x1=(int)nodes[i][j+1].positionImage.x;
			int y1=(int)nodes[i][j+1].positionImage.y;

			if (x0 > 0  && y0 > 0  && x1 > 0 && y1 > 0 ) {
				Line line={(int)x0,(int)y0,(int)x1,(int)y1};
				m_lines.push_back(line);
			}
		}

	}
}

void HandRender::NotifyGestures(PXCHandData::BodySideType handSide, pxcCHAR* gesture){

	if(handSide == PXCHandData::BodySideType::BODY_SIDE_LEFT)
	{
		rightGesture = gesture;
	}else if(handSide == PXCHandData::BodySideType::BODY_SIDE_RIGHT){
		leftGesture = gesture;
	}

}


void HandRender::DrawMore(HDC hdc, double sx, double sy) {

	//Draw Bones
	HPEN red=CreatePen(PS_SOLID, 3, COLOR_BLUE);
	SelectObject(hdc, red);
	for (std::list<Line>::iterator itrl=m_lines.begin(); itrl!=m_lines.end(); itrl++) {
		MoveToEx(hdc, (int)(itrl->x0*sx), (int)(itrl->y0*sy), NULL); 
		LineTo(hdc, (int)(itrl->x1*sx), (int)(itrl->y1*sy)); 
	}
	DeleteObject(red);


	/*Draw track point*/
	HPEN yellow = CreatePen(PS_SOLID, 3, COLOR_YELLOW);
	SelectObject(hdc, yellow);

	for (std::list<MyHand>::iterator itrl1 = myhand_list.begin(); itrl1 != myhand_list.end(); itrl1++) {
		for (std::list<Node>::iterator itrl = (*itrl1).hand_nodes.begin(); itrl != (*itrl1).hand_nodes.end(); itrl++) {
			int x = (int)(itrl->x*sx);
			int y = (int)(itrl->y*sy);
			int rx = (int)(JOINT_CIRCLE_RADIUS*sx);
			int ry = (int)(JOINT_CIRCLE_RADIUS*sy);
			MoveToEx(hdc, x, y, NULL);
			Arc(hdc, x - rx, y - ry, x + rx, y + ry, x + rx, y + ry, x + rx, y + ry);
		}
	}
	DeleteObject(yellow);

	//Draw Joints
	HPEN color=CreatePen(PS_SOLID, 4, COLOR_RED);
	SelectObject(hdc, color);
	for (std::list<Node>::iterator itrl=m_nodes.begin(); itrl!=m_nodes.end(); itrl++) {
		int x=(int)(itrl->x*sx);
		int y=(int)(itrl->y*sy);
		int rx=(int)(JOINT_CIRCLE_RADIUS*sx);
		int ry=(int)(JOINT_CIRCLE_RADIUS*sy);
		MoveToEx(hdc, x, y, NULL);
		Arc(hdc,x-rx,y-ry,x+rx,y+ry,x+rx,y+ry,x+rx,y+ry);	
	}
	DeleteObject(color);


	/*Draw track line*/
	//HPEN green = CreatePen(PS_SOLID, 2, COLOR_GREEN);
	//SelectObject(hdc, green);
	////for (std::list<MyHand>::iterator itrl1 = myhand_list.begin(); itrl1 != myhand_list.end();) {
	//
	//std::list<MyHand>::iterator itrl1 = myhand_list.begin();
	//while(itrl1 != myhand_list.end()){
	//	std::list<Node>::iterator itrl2 = (itrl1++)->hand_nodes.begin();
	//	for (std::list<Node>::iterator itrl = (*itrl1).hand_nodes.begin(); itrl != (*itrl1).hand_nodes.end(); itrl++) {
	//			MoveToEx(hdc, (int)(itrl->x*sx), (int)(itrl->y*sy), NULL);
	//			LineTo(hdc, (int)(itrl2->x*sx), (int)(itrl2->y*sy));
	//			itrl2++;
	//	}
	//}
	//DeleteObject(green);



	//Clear Lists
	m_lines.clear();
	m_nodes.clear();

	//Display Gestures and HandSide
	HFONT hfont;
	LOGFONT logFont;
	memset(&logFont, 0, sizeof(logFont));
	logFont.lfHeight = TEXT_HEIGHT;
	logFont.lfWeight = FW_BOLD;
	hfont = CreateFontIndirect(&logFont);
	SetTextColor(hdc, COLOR_GREEN);
	SelectObject(hdc, hfont);
	int locX=1, locY=1;

	SelectObject(hdc, hfont);

	int x=locX*sx;
	int y=((locY*sy));

	if(leftGesture.compare(L"")!=0)
	{
		TextOutW(hdc, x, y, (LPCWSTR)leftHandSideText.c_str(), wcslen(leftHandSideText.c_str()));
		TextOutW(hdc, x + wcslen(leftHandSideText.c_str())*10, y, (LPCWSTR)leftGesture.c_str(), wcslen(leftGesture.c_str()));
	}

	if(rightGesture.compare(L"")!=0)
	{
		y=((locY*sy)+TEXT_HEIGHT);
		TextOutW(hdc, x, y, (LPCWSTR)rightHandSideText.c_str(), wcslen(rightHandSideText.c_str()));
		TextOutW(hdc, x + wcslen(rightHandSideText.c_str())*10, y, (LPCWSTR)rightGesture.c_str(), wcslen(rightGesture.c_str()));
	}

	DeleteObject(hfont);

}
