#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    //ofSetLogLevel(OF_LOG_VERBOSE);
    
    mReceivePort = 32000;
    
    mSenderPort = 12000;
    mSenderIP = "10.42.0.1";
    
    mReceiver.setup(mReceivePort);
    mSender.setup(mSenderIP, mSenderPort);
    
    stream.setup(2, 0,  44100, 256, 4); //256
    
    
    mLabelsNames.push_back("sky");
    mLabelsNames.push_back("building");
    mLabelsNames.push_back("pole");
    mLabelsNames.push_back("roadMarking");
    mLabelsNames.push_back("road");
    mLabelsNames.push_back("pavement");
    mLabelsNames.push_back("tree");
    mLabelsNames.push_back("signSymbol");
    mLabelsNames.push_back("fence");
    mLabelsNames.push_back("vehicle");
    mLabelsNames.push_back("pedestrian");
    mLabelsNames.push_back("bike");
    mLabelsNames.push_back("void");
    
    colorClass.push_back(ofColor(128, 128, 128));
    colorClass.push_back(ofColor(128, 0, 0));
    colorClass.push_back(ofColor(192, 192, 128));
    colorClass.push_back(ofColor(255, 69, 0));
    colorClass.push_back(ofColor(128, 64, 128)); //road
    colorClass.push_back(ofColor(60, 40, 222));
    colorClass.push_back(ofColor(128, 128, 0));
    colorClass.push_back(ofColor(192, 128, 128));
    colorClass.push_back(ofColor(64, 64, 128));
    colorClass.push_back(ofColor(64, 0, 128));
    colorClass.push_back(ofColor(64, 64, 0));
    colorClass.push_back(ofColor(0, 128, 192));
    colorClass.push_back(ofColor(50, 50, 50));
    
    {
        
        std::string cities []  = {"Mexico"};
        
        for(int j = 0; j < 1; j++){
            
            for(int i = 0; i < mLabelsNames.size(); i++){
                ofDirectory dir;
                dir.allowExt("mp3");
                dir.allowExt("wav");
                std::string dirloc = cities[j]+"/"+mLabelsNames.at(i);
                std::cout<< dirloc <<std::endl;
                dir.listDir(dirloc);
                playersDirectory.push_back( dir );
            }
        }
            
        
        mPredictionClass.resize( mLabelsNames.size() );
        playersClasses.resize( mLabelsNames.size() );
        activeClass.resize( mLabelsNames.size() );
        predictionClass.resize( mLabelsNames.size() );
        activeSoundClass.resize( mLabelsNames.size() );
        sendActivationOSC.resize(mLabelsNames.size() );

        for(int i = 0; i < mLabelsNames.size(); i++){
            playersClasses.at(i).connectTo(mixer);
            
            playersVolumeClasses.push_back(1);
            volumeGroup.add(playersVolumeClasses.back().set(mLabelsNames.at(i), 0.015, 0.0, 1.0));
        }
        
    }
    
   //stream.setInput(input);
    stream.setOutput(output);
    
   inputVol.connectTo(input).connectTo(mixer).connectTo(output);
    
    //input.connectTo(mixer).connectTo(output);
    
    
    //inputVol.connectTo(mixer).connectTo(mDelayFilter).connectTo(output);
    
   // inputVol.connectTo(mixer).connectTo(mDelayFilter).connectTo(output);
    
    gui.setup(volumeGroup);
    gui.setPosition(1200, 20);
    gui.add(volumeInput.setup("input Volumen", 0.5, 0.0, 1.0));
    gui.add(mFilterCutoff.setup("Cutoff", 0.5, 0.1, 1.0));
    gui.add(mDelayFeedback.setup("DelayFeedBack", 0.0, 0.0, 0.99));
    gui.add(mDelayFeedback.setup("maxOut", 0.0, -0.5, 10.80));
    
    

    ofAddListener(volumeGroup.parameterChangedE(), this, &ofApp::updateVolumeClass);
    ofAddListener(volumeGroup.parameterChangedE(), this, &ofApp::updateVolumeInput);
    mDelayFeedback.addListener(this, &ofApp::updateDelay);
    
    ofBackground(0);
    current_msg_string = 0;
    ofSetWindowTitle("Panoptic OSC Maker");
    mSendActivation = false;
    
    
    mTouchActivation.resize(4);
    for(int i = 0; i < 4; i++){
        mTouchActivation.at(i) = false;
    }

}

//--------------------------------------------------------------
void ofApp::updateVolumeClass(ofAbstractParameter& p){
    for (int i = 0; i < playersVolumeClasses.size(); i++) {
        mixer.setChannelVolume(i, playersVolumeClasses[i]);
    }
}

void ofApp::updateVolumeInput(ofAbstractParameter& p){
   // inputVol.setChannelVolume(0, volumeInput);
   //std::cout<<"change input "<<std::endl;
    
    inputVol.setChannelVolume(0, volumeInput);
}

void ofApp::updateDelay( float & val ){
    mDelayFilter.setFeedback(val);
    std::cout<<"change dealy"<<std::endl;
}

//--------------------------------------------------------------
void ofApp::update(){
    
    // hide old messages
    for(int i = 0; i < NUM_MSG_STRINGS; i++){
        if(timers[i] < ofGetElapsedTimef()){
            msg_strings[i] = "";
        }
    }

    while(mReceiver.hasWaitingMessages()){
        // get the next message
        ofxOscMessage m;
        mReceiver.getNextMessage(m);
        string msg_string;
        

        if( m.getAddress() == "/vol" ){
            mVolOSC = m.getArgAsFloat(0);
        }
        
        for(int i = 0; i < mTouchActivation.size(); i++){
            if( m.getAddress() == "/touch"+to_string(i) ){
                mTouchActivation.at(i) = true;
                cout<<"got touch "<<i<<std::endl;
            }
        }
        
        for(int i = 0; i < mLabelsNames.size(); i++){
            if( m.getAddress() == "/"+mLabelsNames.at(i) ){
                float inVal = m.getArgAsFloat(0);
                int  offVal = m.getArgAsInt(1);
                
                mPredictionClass.at(i) = inVal;
                
                cout<<"Got msg "<<inVal<<" "<<offVal<<std::endl;
                if( mPredictionClass.at(i) > 0.0 &&  offVal == 1){
                    
                    if(!playersClasses.at(i).isPlaying()){
                       
                        int randFile  = ofRandom(0, playersDirectory.at(i).size()-1);
                        ofDirectory dir = playersDirectory.at(i);
                        std::string path = dir.getPath(randFile);
                        playersClasses.at(i).load( path  );
                        
                        cout<<"Starting to play class: "<<mLabelsNames.at(i)<<std::endl;
                        
                        cout<<dir.getPath(randFile)<<" Duration: "<< playersClasses.at(i).getDurationMS()<<std::endl;
                        playersClasses.at(i).setLoop(false);
                        playersClasses.at(i).play();
                        activeSoundClass.at(i) = path;
    
                        
                        float max = 1.0;
                        float logMap = (log(inVal*2.0 + 1.0 + maxOut + mVolOSC))/log(1.0 + max);
                        float outMapVol = ofClamp(logMap, 0.0, 1.0);
                        playersClasses.at(i).setVolume(outMapVol);
                        
                        playersVolumeClasses.at(i) = outMapVol;
                        mixer.setChannelVolume(i, outMapVol);
                        
                        activeClass.at(i) = true;
                        
                        mSendActivation = true;
                        sendActivationOSC.at(i) = true;
                        
                    }else{
                        
                        float max = 1.0 + maxOut;
                        float logMap = (log(inVal*2.0 + 1.0 + maxOut + mVolOSC))/log(1.0 + max);
                        float outMapVol = ofClamp(logMap, 0.0, 1.0);
                        playersClasses.at(i).setVolume(outMapVol);
                        
                        playersVolumeClasses.at(i) = outMapVol;
                        mixer.setChannelVolume(i, outMapVol);
                        
                        cout<<"Cannot Play: "<<mLabelsNames.at(i)<<std::endl;
                        cout<<"Changed Volumen to: "<<outMapVol<<std::endl;
                    }
                   
                }
                //stop playing.
                if(offVal == 0){
                    cout<<"Off: "<<mLabelsNames.at(i)<<std::endl;
                    activeClass.at(i) = false;
                    playersClasses.at(i).stop();
                    playersClasses.at(i).unload();
                    playersVolumeClasses.at(i) = 0.0;
                    mixer.setChannelVolume(i, 0.0);
                    mSendActivation = true;
                    sendActivationOSC.at(i) = false;
                }
            }
        }
        
        // add to the list of strings to display
        msg_strings[current_msg_string] = msg_string;
        timers[current_msg_string] = ofGetElapsedTimef() + 5.0f;
        current_msg_string = (current_msg_string + 1) % NUM_MSG_STRINGS;
        // clear the next line
        msg_strings[current_msg_string] = "";
    }
    
    //send a osc when the class music si done playing
    /*
    if(mSendActivation){
        for(int i = 0; i < sendActivationOSC.size(); i++){
            ofxOscMessage m;
            m.setAddress("/"+ mLabelsNames.at(i));
            m.addIntArg( (sendActivationOSC.at(i) == true) ? 1 : 0 );
            mSender.sendMessage(m, false);
        }
        mSendActivation = false;
    }
    */

}

//--------------------------------------------------------------
void ofApp::draw(){
    
    //draw recent unrecognized messages
    for(int i = 0; i < NUM_MSG_STRINGS; i++){
        ofDrawBitmapStringHighlight(msg_strings[i], 10, 30 + 15 * i);
    }
    
    string buf = "listening for osc messages on port : " + ofToString(mReceivePort);
    ofDrawBitmapStringHighlight(buf, 10, 10);
    
    buf = "sending osc messages on : " + ofToString(mSenderPort) +" - "+mSenderIP;
    ofDrawBitmapStringHighlight(buf, 10, 30);
    
    
    for(int i = 0; i < activeClass.size(); i++){
        bool active = activeClass.at(i);
        float prediction = predictionClass.at(i);
        ofColor color   = colorClass.at(i);
        std::string label = mLabelsNames.at(i);
        
        int maxTime = playersClasses.at(i).getDurationMS();
        int currentTime = playersClasses.at(i).getPositionMS();
        
        if(maxTime == currentTime){
            activeClass.at(i) = false;
            playersClasses.at(i).unload();
            
            //send OSC
            sendActivationOSC.at(i) = false;
            mSendActivation = true;
        }
        
        if(active){
            ofSetColor(255, 255, 255);
        }else{
            ofSetColor(130, 130, 100);
        }
        ofDrawBitmapString(" "+std::to_string(prediction)+"  "+label,  70, 70 + i*30);
        ofDrawBitmapString(" "+std::to_string(currentTime)+"/"+std::to_string(maxTime), 270, 70 + i*30);
        ofDrawBitmapString(" "+activeSoundClass.at(i), 400, 70 + i*30);
        
        
        ofSetColor(color);
        ofDrawRectangle(30, 55 + i*30, 20, 20);
        
    }
    
    ofSetColor(255, 255, 255);
    for(int i = 0; i < mTouchActivation.size(); i++){
        bool acTouch = mTouchActivation.at(i);
        ofDrawBitmapString(to_string(i)+": "+to_string(acTouch), 1200, 400 + i*20);
        
    }
    
    gui.draw();

    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
