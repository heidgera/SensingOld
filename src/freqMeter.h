/*
 *  freqMeter.h
 *  audioOutputExample
 *
 *  Created by Exhibits on 1/26/2554.
 *  Copyright 2011 Science Museum of Minnesota. All rights reserved.
 *
 */

#ifndef _freqMeter
#define _freqMeter

#include "ofMain.h"

class freqMeter : public ofInterObj {
public:
	vector<numberTemplate> numTemp;
	ofTrueTypeFont arialLabel;
	ofTrueTypeFont arialHeader;
	int num;
	float s,r,g,b;
	freqMeter():ofInterObj(){}
	void setup(int _x, int _y,double _w, double numDigs){
		arialLabel.loadFont("arial.ttf", 14);
		arialHeader.loadFont("arial.ttf", 20);
		num=numDigs;
		s=(_w)/(numDigs*40);
		cSetup(_x, _y, _w, 60*s);
		r=32,g=176,b=255;
		numTemp.clear();
		for (int i=0; i<numDigs; i++) {
			numTemp.push_back(numberTemplate(x+40*s*i,y,s,.25,.6,.9));
		}
	}
	~freqMeter(){
		numTemp.clear();
	}
	void draw(char * time){
		roundedShadow(x-30, y-50, w+75, h+80, 20, .75);
		roundedBox(x-20, y-40, w+55, h+60, 10, .7, .7, .7, .2);
		roundedBox(x-10, y-10, w+35, h+20, 10, .1, .1, .1, .2);
		for (unsigned int i=0; i<numTemp.size(); i++) {
			numTemp[i].draw(time[i]-'0');
		}
		ofSetColor(255*.25,255*.6,255*.9);
		arialLabel.drawString("hz",x+s*40*num-5, y+54*s);
		ofSetColor(0, 0, 0);
		arialHeader.drawString("Frequency",x, y-20);
	}
};

#endif