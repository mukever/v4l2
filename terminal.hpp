#ifndef __termianl__
#define __terminal__

/**
 * 作者: 王雨泽
 * 创建时间: 2017/11/28
 * 最后修改: 2017/12/01
 * 
 * 功能:提供terminal端的通信抽象层,让其他模块不需要考虑通信及同步问题
 */
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <pthread.h>
#include <functional>
#include "connector.hpp"
#include "stringx.hpp"
 
 

//线程安全的终端通信接口
class terminal {

    protected:
        static terminal* instance;  //terminal只能运行为单例!!!

        static void* echo_callback( void* p );
        static void* callback( void* p );
        void init( int& sockud, int& sockue, int& socktc ); //配置socket的属性
        void echo( const stringx& pack );   //处理controller的echo回应
        void discover( const stringx& pack, const in_addr addr );   //处理controller的有效回应

    public:
        //进还是出
        enum eformat {
            in,
            out
        };

    protected:
        bool                        ctlonline;  //控制器在线
        in_addr                     ctladdr;    //controller地址
        int                         secho;      //心跳标志
        
        stringx                     guid;       //终端的全局唯一身份标识
        pthread_t                   tid_ec;     //终端回调及回声线程
        //int                       tid_fn;     //终端前桥线程 计划使用主线程做钱桥线程所以此处暂时不用
        
        int                         sreq;       //认证请求标志
        time_t                      treq;       //上次请求的时间戳
        stringx                     dreq;       //认证请求数据

        std::function<bool(bool)>   fgate;      //闸机控制回调函数
        std::function<bool()>       ffield;     //在场验证回调函数
        time_t                      pre_time;
        int                         userid;     //当前用户id 判断是否连续开门

    public:
        terminal(const char* _guid );
        ~terminal();

        /**
         * 创建一个认证请求
         * 发送与否取决于当前情景
         * 
         * 参数:
         *  eformat 图像格式
         *  width   图像宽度
         *  height  图像高度
         *  facea   人脸角度
         *  data    图像数据
         * 返回值:
         *  告诉前端此请求是否会被发送
         */
        bool request( eformat eformat, short width, short height, char facea, const stringx& data );

        /**
         * 绑定一个在场验证回调函数
         */
        void setfield( std::function<bool()> func );

        /**
         * 绑定一个闸机控制回调函数
         */
        void setgate( std::function<bool(bool)> func );


};

#endif