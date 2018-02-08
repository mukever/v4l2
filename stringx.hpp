#ifndef __stringx__
#define __stringx__

/*
 * 作者: ezr
 * 创建: 2017/11/26
 * 最后编辑: 2017/11/26
 * 
 * !!!从AP剥离的stringx模块!!!
 * stringx服务于二进制安全需求的串存储策略
 */

#include <string>
#include <string.h>
#include <functional>

//二进制安全的字符串
class stringx{

    protected:
        char *data;     //数据指针指向nullptr或一片长度为size+1的连续空间
        size_t size;    //指示data中有效数据的长度
    
    public:
        stringx();
        stringx( const std::string& str );
        stringx(  const char* str );
        stringx(  const char* str,  size_t length );
        stringx(  const char c );
        stringx(  const stringx& another );
        ~stringx();

        stringx& operator=(  const stringx& str );

        //全等测试将data视为内存块而非字符串
        //空数据与任何数据块不等
        //长度不同则不等
        bool operator==(  const stringx& str )const;
        bool operator!=(  const stringx& str )const;

        stringx operator+(  const stringx& str );
        stringx& operator+=(  const stringx& str );

        /*
         * 映射方法
         * 创建新的字符串,func返回true的字符会被插入新的字符串
         * func参数:
         *  char& ref 新字符串中当前字符的引用
         *  size_t off 原字符串中当前字符的偏移量
         *  size_t count 新字符串当前总长度
         */
        stringx operator%(std::function<bool(char&,size_t,size_t)> func)const;


        char& operator[](  size_t pos );
        operator char*();
        operator const char*()const;
        operator std::string()const;

        size_t length()const; //返回所有有效数据所占空间大小
        size_t strlen()const; //返回从data首地址开始地一个字符串的有效字符长度

        char* find(  const stringx& str,   char* start = nullptr )const;
        char* find_one(  const stringx& chars,   char* start = nullptr )const;
        stringx& replace(  const stringx& from,  const stringx& to,   char* start = nullptr );
        stringx& replace_all(  const stringx& from,  const stringx& to );
};
#endif