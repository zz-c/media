fmpeg�����н��װ.mp4����.m3u8��Ƶ�ļ�
ffmpeg -i test.mp4 -c:v libx264 -c:a copy -f hls -hls_time 5 -hls_list_size 0 test/index.m3u8
ffplay����
ffplay -i http://127.0.0.1:8080/index.m3u8 -x 960 -y 540
