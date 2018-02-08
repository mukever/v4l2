#ifndef __stringx_cpp__
#define __stringx_cpp__

/*
 * 作者: ezr
 * 创建: 2017/11/30
 * 最后编辑: 2017/11/30
 * 
 * stringx类的实现
 */

#include "stringx.hpp"

stringx::stringx():
data(nullptr),size(0){

}
stringx::stringx( const std::string& str ):
size(str.length()) {
    if( size > 0 ) {
        data = new char[size+1];
        memcpy( data, str.data(), size );
        data[size] = 0;
    } else {
        data = nullptr;
    }
}
stringx::stringx(  const char* str ){
    if( str == nullptr ) {
        data = nullptr;
        size = 0;
    } else {
        size = ::strlen(str);
        data = new char[size+1];
        memcpy( data, str, size );
        data[size] = 0;
    }
}
stringx::stringx(  const char* str,  size_t length ){
    if( str == nullptr or length == 0 ) {
        if( length > 0 )
            throw "invalid pointer";
        data = nullptr;
        size = 0;
    } else {
        size = length;
        data = new char[size+1];
        memcpy( data, str, length );
        data[size] = 0;
    }
}
stringx::stringx(  const char c ){
    data = new char[2];
    data[0] = c;
    data[1] = 0;
    size = 1;
}
stringx::stringx(  const stringx& another ){
    if( another.size == 0 ) {
        data = nullptr;
        size = 0;
    } else {
        size = another.size;
        data = new char[size+1];
        memcpy( data, another.data, size );
        data[size] = 0;
    }
}
stringx::~stringx(){
    if( data != nullptr ) {
        delete[] data;
        data = nullptr;
    }
    size = 0;
}

stringx& stringx::operator=(  const stringx& str ){
    if( data != nullptr ) {
        delete[] data;
    }
    if( str.size > 0 ) {
        size = str.size;
        data = new char[size+1];
        memcpy( data, str.data, size+1 );
    } else {
        data = nullptr;
        size = 0;
    }
}

bool stringx::operator==(  const stringx& str ) const{
    if( data == nullptr or str.data == nullptr or size != str.size ) {
        return false;
    } else {
        //从安全工程学来的等时长的字符串判断
        char ret = 0;
        for( size_t i = 0; i < size; i++ ) {
            ret |= data[i] ^ str.data[i];
        }
        return ret == 0;
    }
}
bool stringx::operator!=(  const stringx& str ) const{
    return !(str == *this);
}

stringx stringx::operator+(  const stringx& str ){

    if( str.size == 0 ) {
        return *this;
    }

    stringx rtvalue;
    rtvalue.size = size + str.size;
    if( rtvalue.size == 0 ) {
        return rtvalue;
    }
    rtvalue.data = new char[rtvalue.size+1];

    memcpy( rtvalue.data, data, size );
    memcpy( rtvalue.data+size, str.data, str.size );
    rtvalue.data[rtvalue.size] = 0;
    return rtvalue;
}
stringx& stringx::operator+=(  const stringx& str ){
    
    if( str.size == 0 ){
        return *this;
    }
    
    char *vdata = new char[size + str.size+1];
    memcpy( vdata, data, size );
    memcpy( vdata+size, str.data, str.size );
    vdata[size+str.size] = 0;
    if( data != nullptr )
        delete[] data;
    data = vdata;
    size += str.size;

    return *this;
}
stringx stringx::operator%(std::function<bool(char&,size_t,size_t)> func)const {
    stringx ns;
    //预选池
    char* signs = new char[size];
    size_t count = 0;
    for( size_t off = 0; off < size; off++ ) {
        signs[count] = data[off];
        if( func( signs[count], off, count ) ) {
            count += 1;
        }
    }
    ns = stringx( signs, count );
    delete[] signs;
    return ns;
}

char& stringx::operator[](  size_t pos ){
    if( pos >= size ){
        throw "out of bound";
    }
    return data[pos];
}

stringx::operator char*(){
    return data;
}

stringx::operator const char*()const{
    return data;
}

stringx::operator std::string()const {
    return std::string(data);
}

size_t stringx::length()const{
    return size;
}
size_t stringx::strlen()const{
    return ::strlen(data);
}

char* stringx::find(  const stringx& str, char* start )const{
    if( start == nullptr )
        start = data;
    if( start == nullptr or str.size == 0 or start < data or start >= size+data )
        return nullptr;
    
    for( char* ptr = start; ptr <= data+size-str.size; ptr++ ){
        char* target = ptr;
        for( size_t i = 0; i < str.size; i++ ) {
            if( target[i] != str.data[i] ) {
                target = nullptr;
                break;
            }
        }
        if( target != nullptr ) {
            return target;
        }
    }
    return nullptr;
}
char* stringx::find_one(  const stringx& chars,   char* start )const{
    if( start == nullptr )
        start = data;
    if( start == nullptr or chars.size == 0 or start < data or start >= size+data )
        return nullptr;

    for( size_t i = start-data; i < size; i++ ){
       for( size_t p = 0; p < chars.size; p++ ) {

           if( data[i] == chars.data[p] )
               return data+i;

       }
    }
    return nullptr;
}
stringx& stringx::replace(  const stringx& from,  const stringx& to,   char* start ){
    
    char* target = find( from, start );
    if( target == nullptr ) {
        return *this;
    }

    char *vdata = new char[size-from.size+to.size+1];
    vdata[size-from.size+to.size] = 0;
    memcpy( vdata, data, target-data );
    memcpy( target-data+vdata, to.data, to.size );
    memcpy( target-data+vdata+to.size, target+from.size, size-(target-data)-from.size );
    delete[] data;
    data = vdata;
    size = size-from.size+to.size;
    return *this;
}
stringx& stringx::replace_all(  const stringx& from,  const stringx& to ) {
    char* target = find( from );
    while( target != nullptr ){
        size_t off = target-data;
        replace( from, to, target );
        target = find( from, data+off+to.size );
    }
    return *this;
}
#endif