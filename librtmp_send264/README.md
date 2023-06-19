https://blog.csdn.net/leixiaohua1020/article/details/42104893
https://blog.csdn.net/leixiaohua1020
需x86编译

sps(序列参数集)、pps(图像参数集合)
在H264码流中,都是以"0x00 0x00 0x01"或者"0x00 0x00 0x00 0x01"为开始码的,找到开始码之后,
使用开始码之后的第一个字节的低5位判断是否为7(sps)或者8(pps), 及data[4] & 0x1f == 7 || data[4] & 0x1f == 8.
然后对获取的nal去掉开始码之后进行base64编码,得到的信息就可以用于sdp.sps和pps需要用逗号分隔开来。


