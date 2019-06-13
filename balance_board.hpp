//
//  balance_board.hpp
//
//  Created by Felix Dollack on 13.06.19.
//

#ifndef balance_board_hpp
#define balance_board_hpp

#include <stdio.h>
#include "ofxNetwork.h"
#include "ofxOsc.h"

struct BalanceData {
    std::string boardname;
    float bottom_left;
    float bottom_right;
    float top_left;
    float top_right;
    float sum;
    float virtual_x;
    float virtual_y;
};

class Balanceboard: public ofThread
{
public:
    BalanceData data;

    Balanceboard(int osc_port);
    void start();
    void stop();
    BalanceData getBalanceData();

protected:
    ofxOscReceiver *_receiver;
    ofxOscMessage _osc_msg;
    int _osc_port;
    BalanceData _buffer;

    void threadedFunction();
    void updateData();
};

#endif /* balance_board_hpp */
