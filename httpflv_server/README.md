fmpeg�����н��װ.mp4����.h264��Ƶ�ļ�
ffmpeg -i test.mp4 -c:v copy  -c:a copy test.flv
ffplay����
ffplay -i http://127.0.0.1:8080/test.flv -x 960 -y 540
