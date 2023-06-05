fmpeg命令行解封装.mp4生成.m3u8视频文件
ffmpeg -i test.mp4 -c:v libx264 -c:a copy -f hls -hls_time 5 -hls_list_size 0 test/index.m3u8
ffplay播放
ffplay -i http://127.0.0.1:8080/index.m3u8 -x 960 -y 540
