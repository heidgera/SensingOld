#include "testApp.h"

void fret(double x, double y, double w, double h,double divs, double pos, bool up)
{
	roundedShadow(x-9, y-9, w+18, h+18, 20, .75);
	roundedBox(x-4, y-4, w+8, h+8, 10, .7, .7, .7);
	roundedBox(x, y, w, h, 10, .2, .2, .2);
	for (int i=1; i<divs; i++) {
		if(!up) roundedBox(x+w/divs*i, y, 4, h, 2, .5, .5, .5);
		else roundedBox(x, y+h/divs*i, w, 4, 2, .5, .5, .5);
	}
	ofSetColor(255, 64, 64);
	if(!up) ofCircle(pos, y+h/2, 10);
	else ofCircle(x+w/2,pos, 10);
}

void frets(double x, double y, double s, double divs, double xpos, double ypos)
{
	fret(x, y+s+20, s, 60, divs, xpos, false);
	fret(x-80, y, 60, s, divs, ypos, true);
	ofSetColor(255, 0, 0,64);
	ofLine(xpos, y+s+50, xpos, ypos);
	ofLine(x-50, ypos, xpos, ypos);
}
	

//--------------------------------------------------------------
void testApp::setup(){
	MODE=0;
	complete=false;
	yOffset=100;
	numOctaves=8;
	maxVal=968;
	minVal=100;
	setMin=100;
	maxVal-=setMin;
	minVal-=setMin;
	octave=(maxVal-minVal)/numOctaves;
	
	background.loadImage("background.jpg");
	
	overlayCnt=0;
	
	height=ofGetHeight();
	width=ofGetWidth();
	sqWid=(height-2*yOffset);
	
	pixOctave=sqWid/numOctaves;
	
	net.update(numOctaves, sqWid,sqWid, (width-sqWid)/2, yOffset);
	
	ofHideCursor();
	ofBackground(0,0,0);

	xpos=net.vertex(0, 0).x;
	ypos=net.vertex(0, 0).x;
	
	//---------------- Sinewave synth -------------
	
	// 2 output channels,
	// 0 input channels
	// 22050 samples per second
	// 256 samples per buffer
	// 4 num buffers (latency)

	sampleRate 			= 44100;
	phase				= 0;
	phaseAdder			= 0.0f;
	phaseAdderTarget 	= 0.0f;
	phase2				= 0;
	phaseAdder2			= 0.0f;
	phaseAdderTarget2 	= 0.0f;
	volume				= 0.1f;
	bNoise 				= false;
	lAudio = new float[256];
	rAudio = new float[256];
	ofSoundStreamSetup(2,0,this, sampleRate,256, 4);

	ofSetFrameRate(60);
	
	//--------------------------------------------
	
	serial.setup("/dev/tty.usbserial-A800fgTY",115200);
	
	arialHeader.loadFont("arial.ttf", 40);
	arialLabel.loadFont("arial.ttf", 20);
	
	xMeter.setup((width-sqWid)/2-280, height/2, 150, 4);
	yMeter.setup((width+sqWid)/2+50, height/2, 150, 4);
}


//--------------------------------------------------------------
void testApp::update(){
	
	//int base=(xpos>106.) ? 50 : 50*xpos/106.;
	//targetFrequency = base*pow((double)2, (double)(xpos/106.))/2;
	//phaseAdderTarget = (targetFrequency / (float) sampleRate) * TWO_PI;
	
	serial.writeByte('a');
	
	//int nRead  = 0;
	//bool full = true;
	
	while (serial.available()) {
		byteRead=serial.readByte();
		switch (MODE) {
			case POSDATA:
			case VOLDATA:
				data[3-waitForData--]=byteRead;
				cout << data[(3-(waitForData+1))] << " data[" << (3-(waitForData+1))<<endl;
				break;
			case 0:
				//cout << " Changing mode to "<< byteRead <<endl;
				if (byteRead==POSDATA||byteRead==VOLDATA) {
					MODE=byteRead;
					waitForData=3;
				}
				break;
			default:
				break;
		}
		if(waitForData==0) {
			switch (data[0]) {
				case XPOS:{
					float curPos=xpos=data[1]+(data[2]<<8)-setMin;
					int base=(xpos>octave) ? 50 : 50.*xpos/octave;
					targetFrequency = base*pow(2., (curPos/octave))/2;
					phaseAdderTarget = (targetFrequency / (float) sampleRate) * TWO_PI;
				}
					break;
				case YPOS:{
					float curPos=ypos=data[1]+(data[2]<<8)-setMin;
					int base=(ypos>octave) ? 50 : 50.*ypos/octave;
					targetFrequency2 = base*pow(2., (curPos/octave))/2;
					phaseAdderTarget2 = (targetFrequency2 / (float) sampleRate) * TWO_PI;
				}
					break;
				case VOL:{
					cout << data[0] << (int)data [1] << (int)data[2]<<endl;
					int curVol=data[1]+(data[2]<<8);
					volume=((double)curVol)/1024.f;
				}
					break;

				default:
					break;
			}
			MODE=0;
			complete=false;
			data[0]=data[1]=data[2]=0;
		}

	}
}

static int primes[]={2 , 3 , 5 , 7 , 11 , 13 , 17 , 19 , 23 , 29 , 31 , 37 , 41 , 43 , 47};

void nearest_fraction(int * x, int * y, int size, int num)
{
	//int tempX=x, tempY=y;
	size/=num;
	*x+=size/2;
	*y+=size/2;
	*x/=size;
	*y/=size;
	for (int i=0; i<13; i++) {
		while (((*x)%primes[i]==0&&(*y)%primes[i]==0)&&*x&&*y) {
			(*x)/=primes[i];
			(*y)/=primes[i];
		}
	}
}
	
//--------------------------------------------------------------
void testApp::draw(){
	if(width!=ofGetWidth()||height!=ofGetHeight()){
		height=ofGetHeight();
		width=ofGetWidth();
		sqWid=(height-2*yOffset);
		net.update(numOctaves,sqWid,sqWid, (width-sqWid)/2, yOffset);
		octave=(maxVal-minVal)/numOctaves;
		pixOctave=sqWid/numOctaves;
	}
	
	if(count++>360) count=0;;
	
	
	ofSetColor(175,175,175);
	background.draw(0, 0,width,height);
	
	int xint=ofMap(xpos, minVal, maxVal, (width-sqWid)/2, (width+sqWid)/2,true);
	int yint=ofMap(maxVal-ypos, minVal, maxVal, yOffset, yOffset+sqWid,true);
	
	roundedShadow((width-sqWid)/2-15, yOffset-10, sqWid+25, sqWid+25, 20, .75);
	ofSetColor(255, 255, 255);
	ofRect((width-sqWid)/2-5, yOffset, sqWid+5, sqWid+5);
	
	net.draw();
	
	/************************** Reporting data ********************
	ofSetColor(0xFFFFFF);
	char reportString[255];
	sprintf(reportString, "nread = %i", nBytesRead);
	if (!bNoise) sprintf(reportString, "%s (%fhz)", reportString, targetFrequency);

	ofDrawBitmapString(reportString,80,380);
	
	//arialHeader.drawString(reportString,20, height-50);
	 
	**************************************************************/
	
	overlays(xint,yint);
	
	char freq[32];
	sprintf(freq, "%04i",int(targetFrequency2));
	xMeter.draw(freq);
	sprintf(freq, "%04i",int(targetFrequency));
	yMeter.draw(freq);
	
	ofSetLineWidth(6);
	ofEnableSmoothing();
	for (unsigned int i=1; i<lines.size(); i++) {
		ofSetColor(65, 134, 200,255-i);
		ofLine(lines[i-1].x, lines[i-1].y, lines[i].x, lines[i].y);
	}
	ofDisableSmoothing();
	
	frets((width-sqWid)/2, yOffset, sqWid, numOctaves, xint, yint);
	ofSetColor(65, 134, 200);
	ofCircle(xint, yint, 7);
	
	lines.push_front(ofPoint(xint,yint));
	if (lines.size()>255) {
		lines.pop_back();
	}
}

void testApp::overlays(int xint, int yint)
{
	if(overlayCnt==0){
		char reportString[255];
		sprintf(reportString, "Try to Trace the Lines!");
		int w= arialHeader.stringWidth(reportString);
		int h= arialHeader.stringHeight(reportString);
		ofRectangle box=arialHeader.getStringBoundingBox(reportString, (width-w)/2, 75);
		roundedBox(box.x-5, box.y-5, box.width+10, box.height+10, 10, .6, .6, .6, .2);
		roundedBox(box.x-3, box.y-3, box.width+6, box.height+6, 10, .2, .2, .2, .2);
		ofSetColor(255, 255, 255);
		arialHeader.drawString(reportString,(width-w)/2, 75);
		ofSetLineWidth(3);
		ofSetColor(255, 64, 64);
		for (int i=0; i< net.numdivs; i++) {
			ofLine(net.vertex(i, i),net.vertex(i+1, i));
			ofLine(net.vertex(i+1, i),net.vertex(i+1, i+1));
		}
	}
	if(overlayCnt==1){
		char reportString[255];
		sprintf(reportString, "Constant Ratios");
		int w= arialHeader.stringWidth(reportString);
		int h= arialHeader.stringHeight(reportString);
		ofRectangle box=arialHeader.getStringBoundingBox(reportString, (width-w)/2, 75);
		roundedBox(box.x-5, box.y-5, box.width+10, box.height+10, 10, .6, .6, .6, .2);
		roundedBox(box.x-3, box.y-3, box.width+6, box.height+6, 10, .2, .2, .2, .2);
		ofSetColor(255, 255, 255);
		arialHeader.drawString(reportString,(width-w)/2, 75);
		ofSetLineWidth(3);
		ofSetColor(255, 64, 64);
		ofLine(net.vertex(0,0),net.vertex(numOctaves/2, numOctaves));
		ofLine(net.vertex(0,0),net.vertex(numOctaves, numOctaves/2));
		//arialLabel.drawString("2:1", net.vertex(2,4).x, net.vertex(2,4).y);
		ofPushMatrix();
		ofTranslate(net.vertex(2,4).x-10, net.vertex(2,4).y,0);
		ofRotate(-63, 0,0,1);
		ofSetColor(178, 64, 64);
		arialLabel.drawString("2:1 ratio", 0,0);
		ofPopMatrix();
		ofPushMatrix();
		ofTranslate(net.vertex(4,2).x-15, net.vertex(4,2).y,0);
		ofRotate(-27, 0,0,1);
		ofSetColor(178, 64, 64);
		arialLabel.drawString("1:2 ratio", 0,0);
		ofPopMatrix();
	}
	
	if(overlayCnt==2){
		char reportString[255];
		sprintf(reportString, "Constant Differences");
		int w= arialHeader.stringWidth(reportString);
		int h= arialHeader.stringHeight(reportString);
		ofRectangle box=arialHeader.getStringBoundingBox(reportString, (width-w)/2, 75);
		roundedBox(box.x-5, box.y-5, box.width+10, box.height+10, 10, .6, .6, .6, .2);
		roundedBox(box.x-3, box.y-3, box.width+6, box.height+6, 10, .2, .2, .2, .2);
		ofSetColor(255, 255, 255);
		arialHeader.drawString(reportString,(width-w)/2, 75);
		ofSetLineWidth(3);
		ofSetColor(255, 64, 64);
		ofLine(net.vertex(0,0),net.vertex(numOctaves, numOctaves));
		ofLine(net.vertex(0,1),net.vertex(numOctaves-1, numOctaves));
		//arialLabel.drawString("2:1", net.vertex(2,4).x, net.vertex(2,4).y);
		ofPushMatrix();
		ofTranslate(net.vertex(3,4).x-10, net.vertex(3,4).y,0);
		ofRotate(-45, 0,0,1);
		ofSetColor(178, 64, 64);
		arialLabel.drawString("Octave Difference", 0,0);
		ofPopMatrix();
		ofPushMatrix();
		ofTranslate(net.vertex(3,3).x-15, net.vertex(3,3).y,0);
		ofRotate(-45, 0,0,1);
		ofSetColor(178, 64, 64);
		arialLabel.drawString("Unison", 0,0);
		ofPopMatrix();
	}
	
	if(overlayCnt==3){
		char reportString[255];
		sprintf(reportString, "Can you trace the circle?");
		int w= arialHeader.stringWidth(reportString);
		int h= arialHeader.stringHeight(reportString);
		ofRectangle box=arialHeader.getStringBoundingBox(reportString, (width-w)/2, 75);
		roundedBox(box.x-5, box.y-5, box.width+10, box.height+10, 10, .6, .6, .6, .2);
		roundedBox(box.x-3, box.y-3, box.width+6, box.height+6, 10, .2, .2, .2, .2);
		ofSetColor(255, 255, 255);
		arialHeader.drawString(reportString,(width-w)/2, 75);
		ofSetLineWidth(3);
		ofSetColor(255, 64, 64);
		ofNoFill();
		ofRect(net.vertex(1, 1),pixOctave*(numOctaves-2),-pixOctave*(numOctaves-2));
		ofSetCircleResolution(60);
		ofCircle(net.vertex(numOctaves/2, numOctaves/2), pixOctave*(numOctaves-2)/2);
		ofSetCircleResolution(20);
		ofFill();
	}
	if(overlayCnt==4){
		int xm=xint-(width-sqWid)/2,ym=sqWid-(yint-100);
		nearest_fraction(&xm, &ym,sqWid,numOctaves);
		
		char report[255];
		sprintf(report, "%i:%i ratio",xm,ym);
		ofRectangle box=arialLabel.getStringBoundingBox(report, xint+10, yint-10);
		roundedBox(box.x-5, box.y-5, box.width+10, box.height+10, 10, .6, .6, .6, .2);
		roundedBox(box.x-3, box.y-3, box.width+6, box.height+6, 10, .2, .2, .2, .2);
		ofSetColor(255,255,255);
		arialLabel.drawString(report, xint+10,yint-10);
		ofSetColor(255, 64, 64);
		ofSetLineWidth(3);
		
		char reportString[255];
		sprintf(reportString, "Ratios");
		int w= arialHeader.stringWidth(reportString);
		int h= arialHeader.stringHeight(reportString);
		ofRectangle box2=arialHeader.getStringBoundingBox(reportString, (width-w)/2, 75);
		roundedBox(box2.x-5, box2.y-5, box2.width+10, box2.height+10, 10, .6, .6, .6, .2);
		roundedBox(box2.x-3, box2.y-3, box2.width+6, box2.height+6, 10, .2, .2, .2, .2);
		ofSetColor(255, 255, 255);
		arialHeader.drawString(reportString,(width-w)/2, 75);
		
		/*double xp=xm,yp=ym;
		 
		 if(xp<8&&yp<8){
		 if(xp>yp) yp*=(8/xp), xp=8;
		 else if(yp>xp) xp*=(8/yp), yp=8;
		 else if(yp=xp) xp=yp=8;
		 }
		 ofLine(net.vertex(0, 0),net.vertex(xp, yp));*/
	}
}


//--------------------------------------------------------------
void testApp::keyPressed  (int key){
	if (key == '-'){
		volume -= 0.05;
		volume = MAX(volume, 0);
	} else if (key == '+'){
		volume += 0.05;
		volume = MIN(volume, 1);
	}
	else if(key=='q'){
		serial.close();
		
	}
	else if(key=='n'){
		numOctaves--;
		net.update(numOctaves,sqWid,sqWid, (width-sqWid)/2, yOffset);
		octave=(maxVal-minVal)/numOctaves;
		pixOctave=sqWid/numOctaves;
	}
	else if(key=='m'){
		numOctaves++;
		net.update(numOctaves,sqWid,sqWid, (width-sqWid)/2, yOffset);
		octave=(maxVal-minVal)/numOctaves;
		pixOctave=sqWid/numOctaves;
	}
	else if(key==' '){
		if(++overlayCnt>=5) overlayCnt=0;
	}
}

//--------------------------------------------------------------
void testApp::keyReleased  (int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	/*int width = ofGetWidth();
	pan = (float)x / (float)width;
	float height = (float)ofGetHeight();
	float heightPct = ((height-y) / height);
	targetFrequency = 2000.0f * heightPct;
	phaseAdderTarget = (targetFrequency / (float) sampleRate) * TWO_PI;
	phaseAdderTarget2 = (targetFrequency / (float) sampleRate) * TWO_PI;*/
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	int width = ofGetWidth();
	pan = (float)x / (float)width;
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	bNoise = true;
}


//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	bNoise = false;
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}
//--------------------------------------------------------------
void testApp::audioRequested 	(float * output, int bufferSize, int nChannels){
	pan = 0.5f;
	float leftScale = 1 - pan;
	float rightScale = pan;

	// sin (n) seems to have trouble when n is very large, so we
	// keep phase in the range of 0-TWO_PI like this:
	while (phase > TWO_PI){
		phase -= TWO_PI;
	}
	while (phase2 > TWO_PI){
		phase2 -= TWO_PI;
	}

	if ( bNoise == true){
		// ---------------------- noise --------------
		for (int i = 0; i < bufferSize; i++){
			lAudio[i] = output[i*nChannels    ] = ofRandomf() * volume * leftScale;
			rAudio[i] = output[i*nChannels + 1] = ofRandomf() * volume * rightScale;
		}
	} else {
		phaseAdder = 0.95f * phaseAdder + 0.05f * phaseAdderTarget;
		phaseAdder2 = 0.95f * phaseAdder2 + 0.05f * phaseAdderTarget2;
		for (int i = 0; i < bufferSize; i++){
			phase += phaseAdder;
			phase2 += phaseAdder2;
			float sample = sin(phase);
			float sample2 = sin(phase2);
			//float sample2=(sample>.75)?1:(sample<-.75)?-1:0;
			lAudio[i] = output[i*nChannels    ] = sample * volume * leftScale;
			rAudio[i] = output[i*nChannels + 1] = sample2 * volume * rightScale;
		}
	}

}

void testApp::exit(){
	serial.close();
	//ofSoundStreamClose();
}

