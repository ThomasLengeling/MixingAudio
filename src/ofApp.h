#pragma once

#include "ofMain.h"
#include "ofxGui.h"

#include "ofxBasicSoundPlayer.h"
#include "ofxSoundObjects.h"
#include "ofxOsc.h"

#include "NoiseGenerator.h"
#include "LowPassFilter.h"
#include "DigitalDelay.h"

#define NUM_MSG_STRINGS 30

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

    
        std::vector<std::string> mLabelsNames;
        std::vector<float>       mPredictionClass;
    
        ofxSoundInput  input;
        ofxSoundOutput output;
        ofSoundStream  stream;
    
        ofxSoundMixer mixer;
        ofxSoundMixer inputVol;
    
        ofTrueTypeFont font;
        ofxOscReceiver receiver;
    
        //Filters
        NoiseGenerator  mNoise;
        LowPassFilter   mFilter;
        DigitalDelay    mDelayFilter;
        float           mVolOSC;
    

        int current_msg_string;
        string msg_strings[NUM_MSG_STRINGS];
        float timers[NUM_MSG_STRINGS];
    
        vector< ofxBasicSoundPlayer > playersClasses;
        vector< ofParameter<double> > playersVolumeClasses;
    
        vector< ofDirectory >         playersDirectory;
        vector< bool >                activeClass;
        vector< bool >                sendActivationOSC;
    
        vector< string >              activeSoundClass;
    
        vector< float >               predictionClass;
        vector< ofColor >             colorClass;
    
        vector< bool >                mTouchActivation;
    
        //GUI
        ofParameterGroup volumeGroup;
        ofxFloatSlider   volumeInput;
        ofxFloatSlider   mFilterCutoff;
        ofxFloatSlider   mDelayFeedback;
        ofxFloatSlider   maxOut;
    
        ofxPanel gui;
    
        void updateVolumeClass(ofAbstractParameter& p);
        void updateVolumeInput(ofAbstractParameter& p);
    
        void updateDelay( float & val );
    
        //OSC
        int mReceivePort;
        int mSenderPort;
        std::string mSenderIP;
    
        ofxOscReceiver mReceiver;
        ofxOscSender   mSender;
    
        bool          mSendActivation;
    
    
};
