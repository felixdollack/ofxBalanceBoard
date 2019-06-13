//
//  balance_board.cpp
//
//  Created by Felix Dollack on 13.06.19.
//

#include "balance_board.hpp"

Balanceboard::Balanceboard(int osc_port) {
    this->_osc_port = osc_port;
    this->_receiver = new ofxOscReceiver();
    this->_receiver->setup(this->_osc_port);

    std::memset(&this->_buffer, 0, sizeof(this->_buffer));
}

void Balanceboard::start() {
    this->_receiver->start();
    startThread();
}
void Balanceboard::stop() {
    std::unique_lock<std::mutex> lock(mutex);
    this->_receiver->stop();
    stopThread();
}

void Balanceboard::threadedFunction() {
    float val = 0.0f;
    int kk;

    while (this->isThreadRunning())
    {
        if (this->_receiver->hasWaitingMessages() == true)
        {
            this->_receiver->getNextMessage(this->_osc_msg);

            if (this->_osc_msg.getNumArgs() > 0)
            {
                std::unique_lock<std::mutex> lock(mutex);
                this->_buffer.boardname = this->_osc_msg.getAddress();

                for (kk=0; kk<this->_osc_msg.getNumArgs(); kk++)
                {
                    val = this->_osc_msg.getArgAsFloat(kk);

                    switch(kk)
                    {
                        case 0:
                            this->_buffer.bottom_left = val;
                            break;
                        case 1:
                            this->_buffer.bottom_right = val;
                            break;
                        case 2:
                            this->_buffer.top_left = val;
                            break;
                        case 3:
                            this->_buffer.top_right = val;
                            break;
                        case 4:
                            this->_buffer.sum = val;
                            break;
                        case 5:
                            this->_buffer.virtual_x = val;
                            break;
                        case 6:
                            this->_buffer.virtual_y = val;
                            break;
                        default:
                            break;
                    }
                }
            }
            this->_osc_msg.clear();
        }
    }
}

void Balanceboard::updateData() {
    // if we didn't lock here we would see
    // tearing as the thread would be updating
    // the pixels while we upload them to the texture
    std::unique_lock<std::mutex> lock(mutex);
    this->data = this->_buffer;
}

BalanceData Balanceboard::getBalanceData() {
    this->updateData();
    return this->data;
}
