//
// Created by lxy on 6/16/17.
//

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
#include <cstddef>
#include <cerrno>
#include "minicap_adaptor.h"

bool
read_pic_un(uint32_t &pic_len, char* const buf)
{
    struct sockaddr_un addr;
    int ret = 0;
    static int data_socket = 0;
    static bool init = false;

    if(init)
        goto do_read;

    data_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (data_socket == -1) {
        std::cerr << "socket " << strerror(errno) << std::endl;
        return false;
    }
    /*
            * For portability clear the whole structure, since some
            * implementations have additional (nonstandard) fields in
            * the structure.
            */

    memset(&addr, 0, sizeof(struct sockaddr_un));

    /* Connect socket to socket address */

    addr.sun_family = AF_UNIX;
    strncpy(&addr.sun_path[1], sock_name.c_str(), sock_name.length());

    ret = connect (data_socket, (const struct sockaddr *) &addr,
                   offsetof(struct sockaddr_un, sun_path) + sock_name.length() + 1);


    if (ret == -1) {
        std::cerr << "connect: " << strerror(errno) << std::endl;
        return false;
    }
    if (header_len != read(data_socket, buf, header_len)) {
        std::cerr << "Broken header" << std::endl;
        return false;
    }
    init = true;

do_read:
    //todo while
    ret = read(data_socket, &pic_len, length_len);
    if(ret != length_len) {
        std::cerr << "Broken length " << ret << std::endl;
        return false;
    }
    auto to_read = pic_len;
    while (to_read) {
        ret = read(data_socket, buf + pic_len - to_read, to_read);
        if(ret == -1) {
            std::cerr << "Broken pic" << std::endl;
            return false;
        }
        to_read -= ret;
    }
    return true;
}

