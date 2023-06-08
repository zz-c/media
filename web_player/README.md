# webPlayer

### 项目介绍
* 如何在浏览器播放视频流
* 实现在浏览器播放hls视频流，http-flv视频流

### 实现浏览器播放视频流
~~~

// 网页播放hls协议的视频流
hlsplayer

// 网页播放http-flv协议的视频流
httpflvplayer

~~~

### 准备一个基于ZLMediaKit编译的流媒体服务器程序（用来测试）

* [ZLMediaKit开源地址](https://gitee.com/xia-chu/ZLMediaKit)

~~~

// rtsp推流（文件推流）
ffmpeg -re -i test.mp4 -rtsp_transport tcp -c copy -f rtsp rtsp://127.0.0.1:554/live/test

// rtsp推流（文件循环推流）
ffmpeg -re -stream_loop  -1  -i test.mp4 -rtsp_transport tcp -c copy -f rtsp rtsp://127.0.0.1:554/live/test

// rtmp推流（文件推流）
ffmpeg -re -i test.mp4 -vcodec h264_nvenc  -acodec aac -f flv  rtmp://192.168.1.3:1935/live/test

// rtmp推流（文件循环推流）
ffmpeg -re -stream_loop  -1 -i test.mp4 -vcodec h264  -acodec aac -f flv  rtmp://127.0.0.1:1935/live/test

// ZLMediaKit支持多种流媒体协议的转换，协议转换后的播放地址

//rtsp播放
rtsp://127.0.0.1:554/live/test

//rtmp播放
rtmp://127.0.0.1:1935/live/test

//hls播放
http://127.0.0.1:80/live/test/hls.m3u8

//http-flv播放
http://127.0.0.1:80/live/test.live.flv

//http-ts播放
http://127.0.0.1:80/live/test.live.ts





~~~



