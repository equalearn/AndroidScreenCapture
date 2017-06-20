//
// Created by lxy on 6/16/17.
//

#ifndef _minicap_adaptor_H_
#define _minicap_adaptor_H_

const uint32_t header_len = 24;
const uint32_t length_len = 4;
const uint32_t buffer_size = 1024*1024;
//const uint32_t check_factor = 10;    //check @ times per timestamp cycle
const uint32_t check_interval = 5;   //ms, so [max valid framerate] = 1000 / @
const std::string sock_name = "minicap";

#endif //_minicap_adaptor_H_
