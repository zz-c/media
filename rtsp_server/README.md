fmpeg�����н��װ.mp4����.h264��Ƶ�ļ�
ffmpeg -i test.mp4 -codec copy -bsf: h264_mp4toannexb -f h264 test.h264
ffplay����
ffplay -i rtsp://127.0.0.1:8554 -x 960 -y 540
