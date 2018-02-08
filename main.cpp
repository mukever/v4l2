#include <stdint.h>
#include "gtk/gtk.h"
#include "window.h"
#include "video.h"
#include "iostream"
#include "terminal.hpp"
#include "ysqfd.h"
#include <sys/timeb.h>

using namespace OAID;
using namespace std;
#define WINDOW_XPOS   0
#define WINDOW_YPOS   0
#define RGB_WINDOW    "RGB"
#define  WINDOW_ICON 0
static time_t pre_time;
static uint8_t *grey8rawdata = NULL;
static uint8_t *grey8mem_init(int iw, int ih)
{
    grey8rawdata = (uint8_t *)malloc(iw * ih);
    return grey8rawdata;
}
static void grey8mem_release(void)
{
    if (grey8rawdata) {
        free(grey8rawdata);
        grey8rawdata = NULL;
    }
}

void greydata_from_frame(captureCamera *frame)
{
    int w, h;

    w = frame->pixfmt->width;
    h = frame->pixfmt->height;
    yuvproc::convert_yuyv_to_grey8(frame->base, grey8rawdata, w, h);
}
char*   log_Time(void)
{
    struct  tm      *ptm;
    struct  timeb   stTimeb;
    static  char    szTime[19];

    ftime(&stTimeb);
    ptm = localtime(&stTimeb.time);
    sprintf(szTime, "%02d-%02d %02d:%02d:%02d.%03d",
            ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, stTimeb.millitm);
    szTime[18] = 0;
    return szTime;
}
//闸机控制回调函数(这个到时候鑫哥写底层)
bool gate( bool gate ) {
    time_t  tt = time(NULL);
    if( gate && tt-pre_time >5) {
        pre_time = tt;
        std::cout << "打开闸机" << std::endl;
        system("echo 1>");

    } else {
        std::cout << "关闭闸机" << std::endl;
    }
    return true;//返回false应当触发一个fault log
}
int main(int argc, char **argv)
{


    //建立一个terminal实例
    static terminal mt = terminal("0123456789abcdef12");
    pre_time = time(NULL);
    mt.setgate(gate);
    int camfd;
    int camid = 0;
    int status = 1;
    fcvImage *image_orig;
    captureCamera *frame;
    int vidiw, vidih;

    video *_video = new video();
    window *_window = new window();
    ysqfd *_ysqfd = new ysqfd();
    fcvimage *_fcvimage = new fcvimage();

    camfd = _video->create_vidcapture(camid);
    if (camfd < 0) {
        assert_failure();
        return camfd;
    }

    _video->query_vidimgsize(camid, &vidiw, &vidih);
    grey8mem_init(vidiw, vidih);
    //_ysqfd->algr_ysqfd_init(vidiw, vidih);

    _window->named_window(RGB_WINDOW, WINDOW_NORMAL);

    /* Get the Screen Resolution */
    /*
    GdkScreen* screen;

    gint width, height;
    screen = gdk_screen_get_default();
    width = gdk_screen_get_width(screen);
    height = gdk_screen_get_height(screen);
    printf("screen width: %d, height: %d\n", width, height);

     */

    //_window->resize_window(RGB_WINDOW, width, height);
    _window->resize_window(RGB_WINDOW, 640, 480);
    _window->move_window(RGB_WINDOW, WINDOW_XPOS, WINDOW_YPOS);


    short w, h;
    int flag;
    do {
        _video->capturevid(camid);
        frame = _video->capturevid(camid);
        greydata_from_frame(frame);
        image_orig = _fcvimage->vimage_from_frame(frame);
        w = frame->pixfmt->width;
        h = frame->pixfmt->height;

       // _ysqfd->ysqfd_process(grey8rawdata,image_orig,&flag);

        if(flag){

            std::cout<<"开启线程发送数据"<<std::endl;
            //rawdata =(char*) gdk_pixbuf_get_pixels(image_orig);
            //mt.request(terminal::out, w, h, 1, stringx((char*)rawdata,640*480*3) );
          mt.request(terminal::out, w, h, 1, stringx((char *)frame->base,640*480*2 ));
        }

        _window->imageshow(RGB_WINDOW, image_orig);

        _window->waitkey(1);

    }while(status);

    _video->destroy_vidcapture(camid);
    grey8mem_release();
    _window->destroy_window(RGB_WINDOW);
    //_ysqfd->algr_ysqfd_exit();
    return 0;
}