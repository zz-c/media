fmpeg命令行解封装.mp4生成.h264视频文件
ffmpeg -i test.mp4 -codec copy -bsf: h264_mp4toannexb -f h264 test.h264
ffplay播放
ffplay -i rtsp://127.0.0.1:8554 -x 960 -y 540
