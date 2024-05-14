
#include "sample_common.h"

#include <unistd.h>
#include <string>
#include <sstream>
#include <list>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include "json5pp.hpp"

// << string_format("%d", 202412);
template <typename... Args>
std::string string_format(const std::string &format, Args... args)
{
    size_t size = 1 + snprintf(nullptr, 0, format.c_str(), args...); // Extra space for \0
    // unique_ptr<char[]> buf(new char[size]);
    char bytes[size];
    snprintf(bytes, size, format.c_str(), args...);
    return std::string(bytes);
}

void string_write_file(std::string path, std::string txt)
{
    std::ofstream outfile(path);
    outfile << txt;
    outfile.flush();
    outfile.close();
}

std::string string_read_file(const std::string &path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

#include <nngpp/nngpp.h>
#include <nngpp/platform/platform.h>
#include <nngpp/protocol/pull0.h>
#include <nngpp/protocol/push0.h>
#include <nngpp/protocol/pair1.h>

// struct _sample_data_
// {
//     uint8_t audio_push_state = 0;
//     uint8_t audio_pull_state = 0;
//     uint8_t video_push_state = 0;
//     uint8_t video_pull_state = 0;
// } sample_data;

// extern "C" void set_sample_data(uint8_t audio_push_state, uint8_t audio_pull_state, uint8_t video_push_state, uint8_t video_pull_state)
// {
//     sample_data.audio_push_state = audio_push_state;
//     sample_data.audio_pull_state = audio_pull_state;
//     sample_data.video_push_state = video_push_state;
//     sample_data.video_pull_state = video_pull_state;
// }

// extern "C" void get_sample_data(uint8_t *audio_push_state, uint8_t *audio_pull_state, uint8_t *video_push_state, uint8_t *video_pull_state)
// {
//     *audio_push_state = sample_data.audio_push_state;
//     *audio_pull_state = sample_data.audio_pull_state;
//     *video_push_state = sample_data.video_push_state;
//     *video_pull_state = sample_data.video_pull_state;
// }

struct _sample_dls_
{
    bool is_init = true;
    // uint8_t state = 0, work = 0;
    std::string audio_push_addr = "tcp://0.0.0.0:1028";
    std::string audio_pull_addr = "tcp://0.0.0.0:1028";
    nng::socket *audio_push = NULL;//audio_push;// = nng::push::open();
    nng::socket *audio_pull = NULL;//audio_pull;// = nng::pull::open();

    std::string video_push_addr = "tcp://0.0.0.0:2810";
    std::string video_pull_addr = "tcp://0.0.0.0:2810";
    nng::socket *video_push = NULL;//video_push;// = nng::push::open();
    nng::socket *video_pull = NULL;//video_pull;// = nng::pull::open();
    
    // std::string system_addr = "tcp://0.0.0.0:2024";
    // nng::socket system_sock = nng::pair::open();
} sample_dls;

extern "C" void sample_dls_init(char *audio_push_addr, char *video_push_addr, char *audio_pull_addr, char *video_pull_addr)
{
  try
  {
    if (audio_push_addr) sample_dls.audio_push_addr = audio_push_addr;
    sample_dls.audio_push = new nng::socket();
    *sample_dls.audio_push = nng::push::open();
    nng::set_opt_send_timeout(*sample_dls.audio_push, 40);
    nng::set_opt_reconnect_time_min(*sample_dls.audio_push, 10);
    nng::set_opt_reconnect_time_max(*sample_dls.audio_push, 10);
    printf("audio_push_addr: %s\n", sample_dls.audio_push_addr.c_str());
    sample_dls.audio_push->dial(sample_dls.audio_push_addr.c_str(), nng::flag::nonblock);

    if (video_push_addr) sample_dls.video_push_addr = video_push_addr;
    sample_dls.video_push = new nng::socket();
    *sample_dls.video_push = nng::push::open();
    nng::set_opt_send_timeout(*sample_dls.video_push, 40);
    nng::set_opt_reconnect_time_min(*sample_dls.video_push, 10);
    nng::set_opt_reconnect_time_max(*sample_dls.video_push, 10);
    printf("video_push_addr: %s\n", sample_dls.video_push_addr.c_str());
    sample_dls.video_push->dial(sample_dls.video_push_addr.c_str(), nng::flag::nonblock);

    if (audio_pull_addr) sample_dls.audio_pull_addr = audio_pull_addr;
    sample_dls.audio_pull = new nng::socket();
    *sample_dls.audio_pull = nng::pull::open();
    nng::set_opt_recv_timeout(*sample_dls.audio_pull, 40);
    // nng::set_opt_recv_timeout(*sample_dls.audio_pull, 100); // only recv
    nng::set_opt_reconnect_time_min(*sample_dls.audio_pull, 10);
    nng::set_opt_reconnect_time_max(*sample_dls.audio_pull, 10);
    printf("audio_pull_addr: %s\n", sample_dls.audio_pull_addr.c_str());
    sample_dls.audio_pull->listen(sample_dls.audio_pull_addr.c_str());

    if (video_pull_addr) sample_dls.video_pull_addr = video_pull_addr;
    sample_dls.video_pull = new nng::socket();
    *sample_dls.video_pull = nng::pull::open();
    nng::set_opt_recv_timeout(*sample_dls.video_pull, 40);
    // nng::set_opt_recv_timeout(*sample_dls.video_pull, 100); // only recv
    nng::set_opt_reconnect_time_min(*sample_dls.video_pull, 10);
    nng::set_opt_reconnect_time_max(*sample_dls.video_pull, 10);
    printf("video_pull_addr: %s\n", sample_dls.video_pull_addr.c_str());
    sample_dls.video_pull->listen(sample_dls.video_pull_addr.c_str());

  }
  catch (const nng::exception &e)
  {
    printf("sample_dls_init %s: %s\n", e.who(), e.what());
  }
}

extern "C" void sample_dls_exit()
{
  if (sample_dls.audio_push)
  {
    auto bak = sample_dls.audio_push;
    sample_dls.audio_push = NULL;
    usleep(40*1000);
    delete bak;
  }

  if (sample_dls.video_push)
  {
    auto bak = sample_dls.video_push;
    sample_dls.video_push = NULL;
    usleep(40*1000);
    delete bak;
  }

  if (sample_dls.audio_pull)
  {
    auto bak = sample_dls.audio_pull;
    sample_dls.audio_pull = NULL;
    usleep(40*1000);
    delete bak;
  }

  if (sample_dls.video_pull)
  {
    auto bak = sample_dls.video_pull;
    sample_dls.video_pull = NULL;
    usleep(40*1000);
    delete bak;
  }

}

extern "C" void sample_dls_load()
{
  try
  {
    // printf("sample_data: %d %d %d %d\r\n", sample_data.audio_push_state, sample_data.audio_pull_state, sample_data.video_push_state, sample_data.video_pull_state);
    // uint8_t *buf = (uint8_t *)&sample_data;
    // uint32_t len = sizeof(sample_data);
    // sample_dls.system_sock.send(nng::view(buf, len));
    // print sample_data
    
    if (access("/tmp/sample_dls.json", F_OK) == 0)
    {
      sample_dls.is_init = false;
      printf("sample_dls.conf exist!\n");

      // echo '["tcp://0.0.0.0:1031","tcp://0.0.0.0:2813","tcp://0.0.0.0:1030","tcp://0.0.0.0:2812","work"]' > /tmp/sample_dls.json
      // echo '["tcp://0.0.0.0:1030","tcp://0.0.0.0:2812","tcp://0.0.0.0:1030","tcp://0.0.0.0:2812","stop"]' > /tmp/sample_dls.json
      // echo '["tcp://0.0.0.0:1030","tcp://0.0.0.0:2812","tcp://0.0.0.0:1030","tcp://0.0.0.0:2812","work"]' > /tmp/sample_dls.json
      // echo '["tcp://0.0.0.0:1030","tcp://0.0.0.0:2812","tcp://0.0.0.0:1030","tcp://0.0.0.0:2812","exit"]' > /tmp/sample_dls.json
      auto conf = string_read_file("/tmp/sample_dls.json");

      try
      {
        auto cfg = json5pp::parse(conf);
        auto audio_push_addr = cfg.as_array().at(0).as_string();
        auto video_push_addr = cfg.as_array().at(1).as_string();
        auto audio_pull_addr = cfg.as_array().at(2).as_string();
        auto video_pull_addr = cfg.as_array().at(3).as_string();
        auto user_data = cfg.as_array().at(4).as_string();

        printf("user_data: %s\n", user_data.c_str());
        sample_dls_exit();
        if (user_data == "stop")
        {
          puts("stop");
        }
        else if (user_data == "exit")
        {
          extern void sample_nng_signalHandler(int signum);
          sample_nng_signalHandler(0);
        }
        else
        {
          puts("load");
          sample_dls_init((char*)audio_push_addr.c_str(), (char*)video_push_addr.c_str(), (char*)audio_pull_addr.c_str(), (char*)video_pull_addr.c_str()); 
        }
      }
      catch (json5pp::syntax_error e)
      {
          printf("load_json_conf %s : %s", conf.c_str(), e.what());
      }
      
      system("rm /tmp/sample_dls.json");
    }
    else if (sample_dls.is_init)
    {
      sample_dls.is_init = false;
      sample_dls_init(NULL, NULL, NULL, NULL);
    }

    if (!sample_dls.is_init)
    {
        // 待优化，初始化过，之后每隔 30 分钟重建一次连接，音频长时间连接会出延迟。
        // 测试过 stop 和 start 后会恢复，一定要保证两端的同时发起通话，否则某一方可能会出现延迟。
    }

    system("echo 3 > /proc/sys/vm/drop_caches");

  }
  catch (const nng::exception &e)
  {
    printf("sample_dls_loop %s: %s\n", e.who(), e.what());
  }
}

// #define CALC_FPS(tips)                                                                                         \
//     {                                                                                                          \
//         static int fcnt = 0;                                                                                   \
//         fcnt++;                                                                                                \
//         static struct timespec ts1, ts2;                                                                       \
//         clock_gettime(CLOCK_MONOTONIC, &ts2);                                                                  \
//         if ((ts2.tv_sec * 1000 + ts2.tv_nsec / 1000000) - (ts1.tv_sec * 1000 + ts1.tv_nsec / 1000000) >= 1000) \
//         {                                                                                                      \
//             printf("%s => FPS:%d     \r\n", tips, fcnt);                                                  \
//             ts1 = ts2;                                                                                         \
//             fcnt = 0;                                                                                          \
//         }                                                                                                      \
//     }

extern "C" int sample_dls_audio_push(uint8_t *data, int size)
{
  try
  {
    if (sample_dls.audio_push)
    {
      sample_dls.audio_push->send(nng::view(data, size));
      // CALC_FPS("sample_dls_audio_push");
    }
    else
    {
      usleep(20*1000);
    }
  }
  catch (const nng::exception &e)
  {
    // printf("audio_push %s: %s\n", e.who(), e.what());
    return -1;
  }
  return 0;
}

extern "C" int sample_dls_audio_pull(uint8_t *data, int size)
{
  try
  {
    if (sample_dls.audio_pull)
    {
      size = sample_dls.audio_pull->recv(nng::view(data, size));
      // CALC_FPS("sample_dls_audio_pull");
    }
    else
    {
      usleep(20*1000);
    }
  }
  catch (const nng::exception &e)
  {
    // printf("audio_pull %s: %s\n", e.who(), e.what());
    return -1;
  }
  return size;
}

extern "C" int sample_dls_video_push(uint8_t *data, int size)
{
  try
  {
    if (sample_dls.video_push)
    {
      sample_dls.video_push->send(nng::view(data, size));
      // CALC_FPS("sample_dls_video_push");
    }
    else
    {
      usleep(20*1000);
    }
    // printf("sample_dls_video_push %d\n", size);
  }
  catch (const nng::exception &e)
  {
    // printf("video_push %s: %s\n", e.who(), e.what());
    return -1;
  }
  return 0;
}

extern "C" int sample_dls_video_pull(uint8_t *data, int size)
{
  try
  {
    if (sample_dls.video_pull)
    {
      size = sample_dls.video_pull->recv(nng::view(data, size));
      // CALC_FPS("sample_dls_video_pull");
    }
    else
    {
      usleep(20*1000);
    }
    // printf("sample_dls_video_pull %d\n", size);
  }
  catch (const nng::exception &e)
  {
    // printf("video_pull %s: %s\n", e.who(), e.what());
    return -1;
  }
  return size;
}

// ============================================================================================================================================

#ifdef DLS_VO

// #include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs/legacy/constants_c.h>
#include "opencv2/core/types_c.h"
#include <opencv2/core/core.hpp>
#include <opencv2/freetype.hpp>
#include <opencv2/highgui.hpp>

void overlayImage(const cv::Mat &background, const cv::Mat &foreground, cv::Mat &output, cv::Point2i location, double opacity)
{
  bool allow_mix = false; // By default no blending is allowed and black transparency is preserved
  if (opacity >= 0. && opacity <= 1.) allow_mix = true;

  if (background.data != background.data) background.copyTo(output);
  // start at the row indicated by location, or at row 0 if location.y is negative.
  for (int y = std::max(location.y, 0); y < background.rows; ++y)
  {
    int fY = y - location.y; // because of the translation

    // we are done of we have processed all rows of the foreground image.
    if (fY >= foreground.rows)
      break;

    // start at the column indicated by location, or at column 0 if location.x is negative.
    for (int x = std::max(location.x, 0); x < background.cols; ++x)
    {
      int fX = x - location.x; // because of the translation.

      // we are done with this row if the column is outside of the foreground image.
      if (fX >= foreground.cols)
        break;

      uchar *pixels_fore = foreground.data + (fY * foreground.step + fX * foreground.channels());
      uchar *pixels_back = background.data + (y * background.step + x * background.channels());
      uchar *pixels_output = output.data + (y * output.step + output.channels() * x);

      if (output.channels() == 3) {
        // allow mix color and black is RGB's alpha == 0
        double alpha = (allow_mix) ? ((pixels_fore[0] == 0 && pixels_fore[1] == 0 && pixels_fore[2] == 0) ? 0 : opacity) : 1.;
        if (alpha == 1.) {
            // printf("pos [%d %d] allow_mix %d alpha %f opacity %f\r\n", x, y, allow_mix, alpha, opacity);
            pixels_output[0] = pixels_fore[0];
            pixels_output[1] = pixels_fore[1];
            pixels_output[2] = pixels_fore[2];
        } else {
            double __alpha = (1. - alpha);
            // printf("pos [%d %d] allow_mix %d alpha %f opacity %f\r\n", x, y, allow_mix, alpha, opacity);
            pixels_output[0] = pixels_back[0] * __alpha + pixels_fore[0] * alpha;
            pixels_output[1] = pixels_back[1] * __alpha + pixels_fore[1] * alpha;
            pixels_output[2] = pixels_back[2] * __alpha + pixels_fore[2] * alpha;
        }
      } else if (output.channels() == 4) {
        uchar *pixel = foreground.data + (fY * foreground.step + fX * foreground.channels());
        // determine the opacity of the foregrond pixel, using its fourth (alpha) channel.
        double alpha = (allow_mix) ? (pixel[3] / 255.) * opacity : 1.; // use alpha in foreground RGBA
        // RGB need 3 pixel mix with alpha. and give up black color.
        for (int c = 0; c != output.channels(); ++c) {
            pixels_output[c] = pixels_back[c] * (1. - alpha) + pixels_fore[c] * alpha;
        }
      } else {
        puts("[image.draw_image] Only supports RGB or RGBA");
      }
    }
  }
}

extern "C"
{
  #include "vo.h"

  static int nv12_draw_point(unsigned char* yBuffer, unsigned char* uvBuffer, int w, int h, int x, int y, int yColor, int uColor, int vColor) {
      if (x < 0 || x >= w) return -1;
      if (y < 0 || y >= h) return -1;
      yBuffer[y*w+x] = yColor;
      uvBuffer[(y/2)*w+x/2*2] = uColor;
      uvBuffer[(y/2)*w+x/2*2+1] = vColor;
      return 0;
  }

  static int nv12_draw_rect(unsigned char* yBuffer, unsigned char* uvBuffer, int w, int h, int left, int top, int right, int bottom, int yColor, int uColor, int vColor) {
      int i;
      for (i = left; i <= right; i++) {
          nv12_draw_point(yBuffer, uvBuffer, w, h, i, top, yColor, uColor, vColor);
          nv12_draw_point(yBuffer, uvBuffer, w, h, i, bottom, yColor, uColor, vColor);
      }
      for (i = top; i <= bottom; i++) {
          nv12_draw_point(yBuffer, uvBuffer, w, h, left, i, yColor, uColor, vColor);
          nv12_draw_point(yBuffer, uvBuffer, w, h, right, i, yColor, uColor, vColor);
      }
      return 0;
  }

  void vo_loop()
  {
      // static int i = 1;
      // if (i++ > 180) i = 1;

      // {
      //     VIDEO_FRAME_INFO_S *pFrameInfo = vo_get(0);
      //     if (pFrameInfo)
      //     {
      //         unsigned int *addr = (unsigned int *)pFrameInfo->VFrame.mpVirAddr;
      //         uint8_t buffer[240 * 240 * 3 / 2] = { 0x00 };

      //         {
      //             FILE *fp = fopen("/home/res/test_nv21.yuv", "rb");
      //             if (fp)
      //             {
      //                 fread(buffer, 1, sizeof(buffer), fp);
      //                 fclose(fp);
      //             }
      //         }

      //         cv::Mat yuv_nv12((240 * 3) / 2, 240, CV_8UC1, buffer);
      //         cv::rectangle(yuv_nv12, cv::Point(20, 20), cv::Point(220, 220), cv::Scalar(0x5f), 20);
      //         cv::line(yuv_nv12, cv::Point(20, 20), cv::Point(220, 220), cv::Scalar(0x5f), 20);
      //         cv::circle(yuv_nv12, cv::Point(120, 120), 100, cv::Scalar(0x5f), 20);

      //         nv12_draw_rect(buffer, buffer + 240 * 240, 240, 240, 10, 20, 30, 40, 0xff, 0x80, 0x80);
      //         nv12_draw_rect(buffer, buffer + 240 * 240, 240, 240, 20 + i, 10 + i, 40 + i, 30 + i, 0xff, 0x80, 0x80);

      //         memcpy((void *)addr[0], buffer, 240 * 240 * 3 / 2);
      //         vo_set(pFrameInfo, 0);
      //     }
      // }
      
      // {
      //     VIDEO_FRAME_INFO_S *pFrameInfo = vo_get(9);
      //     if (pFrameInfo)
      //     {
      //         cv::Mat ui_bgra(240, 240, CV_8UC4, cv::Scalar(0, 0, 0, 0));
      //         cv::Mat logo = cv::imread("/home/res/logo.png", cv::IMREAD_UNCHANGED);
      //         cv::cvtColor(logo, logo, cv::COLOR_BGRA2RGBA);
      //         overlayImage(ui_bgra, logo, ui_bgra, cv::Point(0, 0), 0.99);

      //         cv::rectangle(ui_bgra, cv::Point(20, 20), cv::Point(220, 220), cv::Scalar(0, 0, 255, 128), 20);

      //         std::time_t t = std::time(0);
      //         std::tm *now = std::localtime(&t);
      //         char buf[64] = {0};
      //         std::strftime(buf, sizeof(buf), "%Y-%m-%d", now);
      //         cv::putText(ui_bgra, buf, cv::Point(10, 20), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0, 255), 2);
      //         std::strftime(buf, sizeof(buf), "%H:%M:%S", now);
      //         cv::putText(ui_bgra, buf, cv::Point(10, 40), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0, 255), 2);

      //         cv::Mat vo_bgra(240, 240, CV_8UC4, (unsigned char *)pFrameInfo->VFrame.mpVirAddr[0]);
      //         cv::cvtColor(ui_bgra, vo_bgra, cv::COLOR_BGRA2RGBA);
      //         vo_set(pFrameInfo, 9);
      //     }
      // }

// #define V851SE
#ifdef V851SE
      try
      {
          static int flag = 1;

          // 创建 jpg 图像
          static auto img = cv::Mat(DLS_H, DLS_W, CV_8UC3, cv::Scalar(0, 0, 0));
          static std::vector<uchar> buf;
          if (flag)
          {
              cv::randu(img, cv::Scalar(0, 0, 0), cv::Scalar(255, 255, 255));
              // if /home/res/logo.png 存在就使用 

              if (access("/home/res/logo.png", F_OK) != -1)
              {
                  cv::Mat logo = cv::imread("/home/res/logo.png", cv::IMREAD_UNCHANGED);
                  cv::cvtColor(logo, logo, cv::COLOR_BGRA2RGBA);
                  overlayImage(img, logo, img, cv::Point(0, 0), 0.99);
              }

              // 编码 jpg 图像，编码 90 质量
              cv::imencode(".jpg", img, buf, {cv::IMWRITE_JPEG_QUALITY, 50}); // 480 320 50 61k 90 118k
              
              flag = 0;
          }
          // printf("img s: %dx%d %d\r\n", img.cols, img.rows, img.channels());
          // printf("buf size: %d\r\n", buf.size());

          // 发送 jpg 图像
          sample_dls_video_push(buf.data(), buf.size());
      }
      catch(const std::exception& e)
      {
        printf("%s\r\n", e.what());
      }
#endif

// #define V851S
#ifdef V851S
      try
      {
          // 接收 jpg 图像
          static uint8_t buffer[JPEG_MAX_SIZE] = { 0x00 };
          int size = sample_dls_video_pull(buffer, sizeof(buffer));
          // printf("size: %d\r\n", size);
          if (size <= 0) return;

          cv::Mat img = cv::imdecode(cv::Mat(size, 1, CV_8UC1, buffer), cv::IMREAD_COLOR);
          if (img.empty()) return;

          // printf("img r: %dx%d %d\r\n", img.cols, img.rows, img.channels());

          // 保存图像到本地
          // cv::imwrite("/home/res/test.jpg", img, {cv::IMWRITE_JPEG_QUALITY, 90});

          // 显示 jpg 图像
          VIDEO_FRAME_INFO_S *pFrameInfo = vo_get(9);
          if (pFrameInfo)
          {
              cv::Mat ui_bgra(DLS_H, DLS_W, CV_8UC4, cv::Scalar(0, 0, 0, 0));
              // cv::cvtColor(img, img, cv::COLOR_BGRA2RGBA);
              // cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
              overlayImage(ui_bgra, img, ui_bgra, cv::Point(0, 0), 0.99);

              cv::rectangle(ui_bgra, cv::Point(20, 20), cv::Point(220, 220), cv::Scalar(0, 0, 255, 128), 20);

              std::time_t t = std::time(0);
              std::tm *now = std::localtime(&t);
              char buf[64] = {0};
              std::strftime(buf, sizeof(buf), "%Y-%m-%d", now);
              cv::putText(ui_bgra, buf, cv::Point(10, 20), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0, 255), 2);
              std::strftime(buf, sizeof(buf), "%H:%M:%S", now);
              cv::putText(ui_bgra, buf, cv::Point(10, 40), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0, 255), 2);

              cv::Mat vo_bgra(DLS_H, DLS_W, CV_8UC4, (unsigned char *)pFrameInfo->VFrame.mpVirAddr[0]);
              cv::cvtColor(ui_bgra, vo_bgra, cv::COLOR_BGRA2RGBA);
              vo_set(pFrameInfo, 9);
          }
      }
      catch(const std::exception& e)
      {
        printf("%s\r\n", e.what());
      }
#endif

  }
}

#endif

// echo "tcp://0.0.0.0:1028,tcp://0.0.0.0:2810,tcp://0.0.0.0:1028,tcp://0.0.0.0:2810,stop" > /tmp/sample_dls.cfg
// echo "tcp://0.0.0.0:1030,tcp://0.0.0.0:2812,tcp://0.0.0.0:1031,tcp://0.0.0.0:2813,work" > /tmp/sample_dls.cfg
int dnng_main(int argc, char **argv)
{
    system("cat /dev/zero > /dev/fb0");
    printf("Hello, main!\n");

    int DLS_VO = 0;
    int DLS_VI = 0;
    if (argc == 3)
    {
        DLS_VO = atoi(argv[1]);
        DLS_VI = atoi(argv[2]);
    }
    else
    {
        DLS_VO = 0;
        DLS_VI = 0;
    }
    printf("DLS_VO = %d, DLS_VI = %d\n", DLS_VO, DLS_VI);

    int sample_dls_audio_push(uint8_t *data, int size);
    int sample_dls_audio_pull(uint8_t *data, int size);
    set_sample_dls_audio_callback(sample_dls_audio_push, sample_dls_audio_pull);
    
    int sample_dls_video_push(uint8_t *data, int size);
    int sample_dls_video_pull(uint8_t *data, int size);
    set_sample_dls_video_callback(sample_dls_video_push, sample_dls_video_pull);

    sample_dls_load();

    sample_nng_load(DLS_VO, DLS_VI);

    while (!get_sample_nng_exit_flag())
    {
        usleep(1000 * 1000);
        sample_dls_load();
    }

    sample_nng_free(DLS_VO, DLS_VI);

    sample_dls_exit();

    return 0;
}

