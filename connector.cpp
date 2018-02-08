#ifndef __connector_cpp__
#define __connector_cpp__

#include "connector.hpp"
#include <stdarg.h>
#include <iostream>

stringx package::packup( const char cmd, const stringx& content ) {
    int total = 10+content.length();
    char* pack = new char[total];
    pack[0] = 0xEA;
    pack[1] = 0xAE;
    pack[2] = cmd;
    pack[3] = (total>>0)&0x00FF;
    pack[4] = (total>>8)&0x00FF;
    pack[5] = (total>>16)&0x00FF;
    pack[6] = (total>>24)&0x00FF;
    memcpy( pack+7, content, content.length() );
    memset( pack+7+content.length(), 0xEE, 3 );

    stringx strx(pack, 10+content.length());
    delete[] pack;
    return strx;
}

stringx package::packup( const char cmd, ... ) {
    
}

char package::unpack(const stringx& pack, stringx& content ) {
    char ends[3];
    char begs[2];
    memset(ends, 0xee, 3 );
    begs[0] = 0xEA;
    begs[1] = 0xAE;
    if( pack.length() < 10 ) {
        throw "bad package with length";
    }
    if(memcmp( pack, begs, 2 )) {
        throw "bad package with begining";
    }
    if(memcmp( pack + pack.length() - 3, ends, 3 )) {
        throw "bad package with endding";
    }
    content = stringx(pack+7,pack.length()-10);
    return pack[2];
}

char package::gateunpack(const stringx& pack, stringx& content, int* id) {
    char ends[3];
    char begs[2];
    memset(ends, 0xee, 3 );
    begs[0] = 0xEA;
    begs[1] = 0xAE;
    if( pack.length() < 10 ) {
        throw "bad package with length";
    }
    if(memcmp( pack, begs, 2 )) {
        throw "bad package with begining";
    }
    if(memcmp( pack + pack.length() - 3, ends, 3 )) {
        throw "bad package with endding";
    }
    *id = 1;
    //std::cout<<"\n用户ID:"<<stringx( pack+8,pack.length()-10)<<std::endl;
    return pack[2];
}


#endif
