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
    this->needSmoothing = false;
    this->smoothingParam = 0.0f;
    std::memset(&this->_buffer, 0, sizeof(this->_buffer));
}

bool Balanceboard::isSmoothing() {
    return this->needSmoothing;
}

void Balanceboard::setSmoothing(float value) {
    if (value > 0.0f) {
        this->needSmoothing = true;
        this->smoothingParam = value;
    } else {
        this->needSmoothing = false;
        this->smoothingParam = 0.0f;
    }
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
    int kk, board_id = -1;

    while (this->isThreadRunning())
    {
        if (this->_receiver->hasWaitingMessages() == true)
        {
            this->_receiver->getNextMessage(this->_osc_msg);

            if (this->_osc_msg.getNumArgs() > 0)
            {
                std::unique_lock<std::mutex> lock(mutex);
                const std::string boardname = this->_osc_msg.getAddress();
                // get board ID (osculator id's range from 1 to 8)
                board_id = std::stoi(&boardname[5])-1;
                for (kk=0; kk<this->_osc_msg.getNumArgs(); kk++)
                {
                    val = this->_osc_msg.getArgAsFloat(kk);
                    if (val < 0.03f) {
                        if (kk != 4) {
                            // ignore very small values
                            continue;
                        }
                    }

                    switch(kk)
                    {
                        case 4:
                            this->_buffer.sum[board_id] = val;
                            this->_buffer.maxWeight[board_id] = std::max(this->_buffer.maxWeight[board_id], val);
                            // value that works well as lower threshold to say no user is present
                            if (val < 0.03f) {
                                this->_buffer.sum[board_id] = 0.02f; // this is smaller than the presence threshold
                                this->_buffer.maxWeight[board_id] = 0.03f;
                            }
                            break;
                        case 5:
                            if (this->needSmoothing) {
                                this->_buffer.virtual_x[board_id] = (1-this->smoothingParam) * val + this->smoothingParam * this->_buffer.virtual_x[board_id];
                            } else {
                                this->_buffer.virtual_x[board_id] = val;
                            }
                            this->_buffer.norm_x[board_id] = (this->_buffer.virtual_x[board_id]-0.5) / this->_buffer.maxWeight[board_id];
                            break;
                        case 6:
                            if (this->needSmoothing) {
                                this->_buffer.virtual_y[board_id] = (1-this->smoothingParam) * val + this->smoothingParam * this->_buffer.virtual_y[board_id];
                            } else {
                                this->_buffer.virtual_y[board_id] = val;
                            }
                            this->_buffer.norm_y[board_id] = (this->_buffer.virtual_y[board_id]-0.5) / this->_buffer.maxWeight[board_id];
                        default:
                            break;
                    }
                }
                board_id = -1;
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
