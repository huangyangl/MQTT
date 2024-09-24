#include <cstdio>
#include <opencv2/opencv.hpp>  
#include <iostream> 
int main(int argc, char **argv){    
    if (argc != 2) {  
        std::cerr << "Usage: " << argv[0] << " <RTSP_URL>" << std::endl;  
        return -1;  
    }  
    // 创建一个VideoCapture对象来打开视频流  
    cv::VideoCapture cap(argv[1],cv::VideoCaptureAPIs::CAP_FFMPEG);  
    // 检查视频是否成功打开  
    if (!cap.isOpened()) {  
        std::cerr << "Error opening video stream or file" << std::endl;  
        return -1;  
    }
    //推流
	std::string rtsp_server_url = "rtsp://127.0.0.1:554/live/0";
	std::stringstream command;
	command << "ffmpeg ";
	// inputfile options
	command << "-y "  // overwrite output files
	<< "-an " // disable audio
	<< "-f rawvideo " // force format to rawvideo
	<< "-vcodec rawvideo "  // force video rawvideo ('copy' to copy stream)
	<< "-pix_fmt bgr24 "  // set pixel format to bgr24
	<< "-s 640x480 "  // set frame size (WxH or abbreviation)
	<< "-r 30 "; // set frame rate (Hz value, fraction or abbreviation)
	command << "-i - ";
	// outputfile options
	command 
	<< "-c:v libx264 "  // Hyper fast Audio and Video encoder
	<< "-pix_fmt yuv420p "  // set pixel format to yuv420p
	<< "-tune:v zerolatency "
	<< "-preset ultrafast " // set the libx264 encoding preset to ultrafast
	<< "-f rtsp " // force format to flv for rtmp, rtsp for rtsp
	<< rtsp_server_url;
	FILE *fp = nullptr;
    try
    {
        // 创建一个窗口来显示视频  
        cv::namedWindow("RTSP Stream", cv::WINDOW_AUTOSIZE);  
        cv::Mat frame;
        // 打开管道
        fp = popen(command.str().c_str(), "w");
        if (fp != nullptr) 
            std::cout << "fp open success!" << std::endl;
        else 
        {
            std::cout << "fp open fail!" << std::endl;
            pclose(fp);
        }
        while (true) {  
            // 读取一帧  
            if (!cap.read(frame)) {  
                std::cerr << "No frame" << std::endl;  
                continue;  
            }
            if(frame.empty()) continue;
            //推流
			fwrite(frame.data, sizeof(char), frame.total() * frame.elemSize(), fp); 
            // 显示帧  
            //cv::imshow("RTSP Stream", frame);  
            // 按 'q' 退出循环  
            if (cv::waitKey(1) == 'q') {  
                break;  
            }  
        }  
        // 释放对象并关闭所有窗口  
        pclose(fp);
        cap.release();
        cv::destroyAllWindows();
        return 0; 
    }
    catch(const std::exception& e)
    {
        std::cerr << "catch a err:" << std::endl; 
        std::cerr << e.what() << '\n';
        // 释放对象并关闭所有窗口
        pclose(fp);
        cap.release();
        cv::destroyAllWindows();
    }
    
}


extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}
 
int main() {
    AVFormatContext *format_ctx = NULL;
    AVStream *video_stream = NULL;
    AVCodec *codec;
    AVCodecContext *codec_ctx = NULL;
    int ret;
 
    // 1. 分配输出上下文并设置输出格式为RTSP
    avformat_alloc_output_context2(&format_ctx, NULL, "rtsp", "rtsp://your_rtsp_url");
    if (!format_ctx) {
        ret = AVERROR_UNKNOWN;
        goto end;
    }
 
    // 2. 查找编码器
    codec = avcodec_find_encoder(AV_CODEC_ID_H264); // 假设使用H264编码
    if (!codec) {
        ret = AVERROR_UNKNOWN;
        goto end;
    }
 
    // 3. 创建视频流
    video_stream = avformat_new_stream(format_ctx, codec);
    if (!video_stream) {
        ret = AVERROR_UNKNOWN;
        goto end;
    }
    codec_ctx = video_stream->codec;
    codec_ctx->codec_id = AV_CODEC_ID_H264;
    codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
    // 设置编码器选项，例如分辨率、帧率等
    codec_ctx->width = 640;
    codec_ctx->height = 480;
    codec_ctx->time_base = (AVRational){1, 25}; // 假设帧率为25
    // 设置输出编码器为H264
    // ... 设置其他编码器参数
 
    // 4. 打开编码器
    if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
        ret = AVERROR_UNKNOWN;
        goto end;
    }
 
    // 5. 写入文件头
    if (!(format_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&format_ctx->pb, "rtsp://your_rtsp_url", AVIO_FLAG_WRITE);
        if (ret < 0) {
            goto end;
        }
    }
    ret = avformat_write_header(format_ctx, NULL);
    if (ret < 0) {
        goto end;
    }
 
    // 6. 编码视频帧并发送至RTSP服务器
    // ... 编码和发送视频帧的代码
 
    // 7. 写入文件尾
end:
    av_write_trailer(format_ctx);
 
    // 关闭资源
    if (format_ctx && !(format_ctx->oformat->flags & AVFMT_NOFILE)) {
        avio_close(format_ctx->pb);
    }
    avformat_free_context(format_ctx);
 
    return ret < 0;
}


extern "C" {  
#include <libavformat/avformat.h>  
#include <libavutil/timestamp.h>  
}  
  
// 假设已经初始化了FFmpeg库，并且打开了输入和输出流  
// AVFormatContext* ifmt_ctx; // 输入流上下文  
// AVFormatContext* ofmt_ctx; // 输出流上下文  
  
// 读取并转发数据包的函数  
void forward_stream(AVFormatContext* ifmt_ctx, AVFormatContext* ofmt_ctx) {  
    AVPacket pkt;  
    int ret;  
  
    // 循环读取输入流中的数据包  
    while ((ret = av_read_frame(ifmt_ctx, &pkt)) >= 0) {  
        // 检查是否需要调整时间戳（这里作为示例，我们简单地将其加上一个固定的延迟）  
        // 注意：实际应用中，你可能需要根据流的具体情况和需要来调整时间戳  
        int64_t delay = AV_TIME_BASE; // 假设延迟为1秒（AV_TIME_BASE是FFmpeg中定义的时间基准，通常为1,000,000）  
        pkt.pts = av_rescale_q_rnd(pkt.pts + delay, ifmt_ctx->streams[pkt.stream_index]->time_base, ofmt_ctx->streams[pkt.stream_index]->time_base, AV_ROUND_PASS_MINMAX);  
        pkt.dts = av_rescale_q_rnd(pkt.dts + delay, ifmt_ctx->streams[pkt.stream_index]->time_base, ofmt_ctx->streams[pkt.stream_index]->time_base, AV_ROUND_PASS_MINMAX);  
  
        // 将数据包写入输出流  
        // 注意：这里假设输出流的stream_index与输入流的stream_index相匹配，实际情况可能需要更复杂的映射  
        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);  
        if (ret < 0) {  
            // 错误处理  
            fprintf(stderr, "Error muxing packet\n");  
            break;  
        }  
  
        // 释放数据包  
        av_packet_unref(&pkt);  
    }  
  
    // 检查是否因为到达文件末尾而退出循环  
    if (ret < 0 && ret != AVERROR_EOF) {  
        // 错误处理  
        fprintf(stderr, "Error reading frames from input file\n");  
    }  
  
    // 写入文件尾并关闭输出流  
    av_write_trailer(ofmt_ctx);  
}  
  

  //20240902
  extern "C" {  
#include <libavformat/avformat.h>  
#include <libavcodec/avcodec.h>  
#include <libswscale/swscale.h>  
#include <libavutil/imgutils.h>  
}  
  
int main() {  
    av_register_all();  
  
    AVFormatContext* pFormatCtx = nullptr;  
    if (avformat_alloc_output_context2(&pFormatCtx, nullptr, "rtsp", "rtsp://your_server_ip:port/stream") < 0) {  
        fprintf(stderr, "Could not create output context\n");  
        return -1;  
    }  
  
    // 设置输出格式参数（可选，根据需求配置）  
    // 例如，设置视频流参数  
    AVStream* video_stream = avformat_new_stream(pFormatCtx, nullptr);  
    if (!video_stream) {  
        fprintf(stderr, "Failed allocating output stream\n");  
        return -1;  
    }  
  
    AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);  // 假设使用H.264编码  
    if (!codec) {  
        fprintf(stderr, "Codec not found\n");  
        return -1;  
    }  
  
    AVCodecContext* c = avcodec_alloc_context3(codec);  
    if (!c) {  
        fprintf(stderr, "Could not allocate video codec context\n");  
        return -1;  
    }  
  
    // 填充AVCodecContext参数...  
    c->bit_rate = 400000;  
    // 其他参数设置...  
  
    if (avcodec_open2(c, codec, nullptr) < 0) {  
        fprintf(stderr, "Could not open codec\n");  
        return -1;  
    }  
  
    video_stream->codecpar->codec_id = c->codec_id;  
    avcodec_parameters_from_context(video_stream->codecpar, c);  
  
    // 初始化URL协议  
    if (!(pFormatCtx->oformat->flags & AVFMT_NOFILE)) {  
        if (avio_open(&pFormatCtx->pb, pFormatCtx->url, AVIO_FLAG_WRITE) < 0) {  
            fprintf(stderr, "Could not open output URL '%s'", pFormatCtx->url);  
            return -1;  
        }  
    }  
  
    // 写入文件头  
    if (avformat_write_header(pFormatCtx, nullptr) < 0) {  
        fprintf(stderr, "Error occurred when opening output URL\n");  
        return -1;  
    }  
  
    // 这里添加你的视频帧编码和发送逻辑  
  
    // 写入文件尾  
    av_write_trailer(pFormatCtx);  
  
    // 清理  
    avcodec_free_context(&c);  
    avformat_free_context(pFormatCtx);  
  
    return 0;  
}


extern "C" {  
#include <libavformat/avformat.h>  
#include <libavutil/timestamp.h>  
}  
#include <iostream>  
#include <thread>  
#include <mutex>  
#include <condition_variable>  
#include <queue>  
using namespace std;
// 读取并转发数据包 
void forward_stream(AVFormatContext* ifmt_ctx, AVFormatContext* ofmt_ctx) {  
    AVPacket *pkt = av_packet_alloc();
    // 循环读取输入流中的数据包  
    while (av_read_frame(ifmt_ctx, pkt) >= 0) {  
        // 检查是否需要调整时间戳（这里作为示例，我们简单地将其加上一个固定的延迟）  
        // 注意：实际应用中，你可能需要根据流的具体情况和需要来调整时间戳  
        //int64_t delay = AV_TIME_BASE; // 假设延迟为1秒（AV_TIME_BASE是FFmpeg中定义的时间基准，通常为1,000,000）  
        //pkt->pts = av_rescale_q_rnd(pkt->pts + delay, ifmt_ctx->streams[pkt->stream_index]->time_base, ofmt_ctx->streams[pkt->stream_index]->time_base, AV_ROUND_PASS_MINMAX);  
        //pkt->dts = av_rescale_q_rnd(pkt->dts + delay, ifmt_ctx->streams[pkt->stream_index]->time_base, ofmt_ctx->streams[pkt->stream_index]->time_base, AV_ROUND_PASS_MINMAX);  
        //pkt->dts = pkt->dts + delay; // some_delay是你想要添加的延迟  
        //pkt->pts = pkt->pts + delay; // 同上
        // 将数据包写入输出流  
        // 注意：这里假设输出流的stream_index与输入流的stream_index相匹配，实际情况可能需要更复杂的映射 
        if (av_interleaved_write_frame(ofmt_ctx, pkt) < 0) {  
            // 错误处理  
            fprintf(stderr, "Error muxing packet\n");  
            continue;  
        }  
        // 释放数据包  
        av_packet_unref(pkt);  
    }
    // 写入文件尾  
    av_write_trailer(ofmt_ctx);
    // 释放pkt
    av_packet_free(&pkt);
} 
  
int main(int argc, char **argv) {  
    char* _input_url = "rtsp://192.168.144.25:8554/main.264";
    char* _output_url = "rtmp://127.0.0.1:554/live/a8";
    if (argc >= 2) {
      for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-i" && i + 1 < argc) {
          _input_url = argv[i + 1];
          i++;
        }
        else if (std::string(argv[i]) == "-o" && i + 1 < argc) {
          _output_url = argv[i + 1];
          i++;
        }
        else if (std::string(argv[i]) == "-h") {
            std::cout << "\nUsage:\n"
            << "  " << argv[0]
            << " [-i <input_url>] [-o <output_url>]\n"
            << std::endl
            << "Description:\n"
            << "  -i <input_url>\n"
            << "     eg: -i rtsp://192.168.144.25:8554/main.264\n"
            << "  -o <output_url>\n"
            << "     eg: -o rtmp://127.0.0.1:554/live/test\n"
            << "     eg: -o rtsp://127.0.0.1:554/live/test\n"
            << "  -h\n"
            << "      Print help message.\n"
            << std::endl;
            return 0;
        }
        else {
            std::cout << "Error : unknown parameter " << argv[i] << std::endl<< "See " << argv[0] << " -h" << std::endl;
            return 0;
        }
      }
    }  
    const char* input_url = _input_url;
    const char* output_url = _output_url;
   // 初始化FFmpeg库
    av_register_all();
    AVFormatContext* ifmt_ctx = nullptr;
    AVFormatContext* ofmt_ctx = nullptr;
    try{
        // 打开输入流 
        if (avformat_open_input(&ifmt_ctx, input_url, nullptr, nullptr) != 0) {  
            std::cerr << "Failed to open input stream\n";  
            return 1;  
        }
        // 查找流信息 
        if (avformat_find_stream_info(ifmt_ctx, nullptr) < 0) {  
            std::cerr << "Failed to retrieve input stream information\n";  
            return 1;  
        }
        // 查找输出格式  
        //AVOutputFormat* out_fmt = av_guess_format("rtsp", NULL, NULL); 
        // 分配输出上下文
        std::string output_url_str(_output_url); 
        if(output_url_str.find("rtmp")!= std::string::npos)
            avformat_alloc_output_context2(&ofmt_ctx, nullptr, "flv", output_url);
        else if(output_url_str.find("rtsp")!= std::string::npos)
            avformat_alloc_output_context2(&ofmt_ctx, nullptr, "rtsp", output_url);
        else{
            std::cout << "output_url invalid " << std::endl<< "See " << argv[0] << " -h" << std::endl; 
            return -1; 
        } 
        if (!ofmt_ctx) {
            std::cerr << "Failed to allocate output context\n";
            return -1;  
        }
        // 复制流（音频流、视频流）
        for (unsigned int i = 0; i < ifmt_ctx->nb_streams; i++) {
            AVCodecParameters* codecpar = ifmt_ctx->streams[i]->codecpar;
            AVMediaType type = codecpar->codec_type;
            string typeStr = type == AVMEDIA_TYPE_VIDEO? "Video" : type == AVMEDIA_TYPE_AUDIO? "Audio" : "others";
            std::cerr << "i: " <<i <<";  input stream type: " << typeStr << endl;
            AVStream* in_stream = ifmt_ctx->streams[i];
            AVStream* out_stream = avformat_new_stream(ofmt_ctx, nullptr);//为ofmt_ctx创建一个流（音频、视频）
            if (!out_stream) {
                std::cerr << "Failed allocating output stream\n";
                return -1;
            }
            // 复制输入流的编解码器参数
            if (avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar) < 0) {
                std::cerr << "Failed to copy codec parameters\n";
                return -1;
            }
            // 设置时间基准
            out_stream->time_base = in_stream->time_base; 
        }
        // 查看输出流的编码器类型
        for (unsigned int i = 0; i < ofmt_ctx->nb_streams; i++) {
            AVCodecParameters* codecpar = ofmt_ctx->streams[i]->codecpar;
            AVMediaType type = codecpar->codec_type;
            string typeStr = type == AVMEDIA_TYPE_VIDEO? "Video" : type == AVMEDIA_TYPE_AUDIO? "Audio" : "others";
            std::cerr << "i: " <<i <<";  output stream type: " << typeStr << ";  codec_id: " << ofmt_ctx->streams[i]->codecpar->codec_id <<endl; 
        }

        // 打开输出URL
        if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
            if (avio_open(&ofmt_ctx->pb, output_url, AVIO_FLAG_WRITE) < 0) {
                std::cerr << "Could not open output URL\n";  
                return -1; 
            }  
        }
        // 写入文件头
        if (avformat_write_header(ofmt_ctx, nullptr) < 0) {  
            std::cerr << "Error occurred when opening output URL\n";
            return 1;  
        }
        // 转发  
        forward_stream(ifmt_ctx, ofmt_ctx);//这里有一个死循环
        // 输入流异常会导致forward_stream退出循环，执行下面的代码
        // 关闭文件、清理上下文
        avio_close(ofmt_ctx->pb);
        avformat_free_context(ofmt_ctx);
        avformat_close_input(&ifmt_ctx);
        return 0;
    }
    catch(const std::exception& e)
    {
        std::cerr << "catch an Error:"<< '\n';
        std::cerr << e.what() << '\n';
        // 关闭文件、清理上下文
        avio_close(ofmt_ctx->pb);
        avformat_free_context(ofmt_ctx);
        avformat_close_input(&ifmt_ctx);
        return -1;
    }
    return -1;

}


# 顶层CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(mqttclient VERSION 0.1)
# 寻找OpenCV库
find_package(mavsdk REQUIRED)
# 查找FFmpeg库
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED libavformat libavcodec libavutil libswscale)

# streaming
# 添加可执行文件
add_executable(streaming)
target_sources(streaming PRIVATE streaming.cpp)
# 添加头文件
target_include_directories(streaming PRIVATE /usr/include/opencv4/)
# 链接OpenCV库
target_link_libraries(streaming ${FFMPEG_LIBRARIES})