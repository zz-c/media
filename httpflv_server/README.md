fmpeg命令行解封装.mp4生成.h264视频文件
ffmpeg -i test.mp4 -c:v copy  -c:a copy test.flv
ffplay播放
ffplay -i http://127.0.0.1:8080/test.flv -x 960 -y 540
