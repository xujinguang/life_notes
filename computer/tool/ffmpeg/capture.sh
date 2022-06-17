#!/bin/bash
#ffmpeg -y -f x11grab -r 30 -s 1920x1080 -i :0.0+0,0 -vcodec libx264 -strict -2 -b:v 16M output.mov
time_prefix=`date '+%Y%m%d%H%M%S'`
filename=$(basename -- "$2")
extension="${filename##*.}"
filename="${filename%.*}"
if [ $# -eq 0 ]
then
ffmpeg -y\
	-f alsa -ac 2 -sample_rate 44100 -i sysdefault:CARD=Microphone\
	-f x11grab -framerate 30 -video_size 1920x1080 -i :1 -c:v libx264 -pix_fmt yuv420p -qp 0 -preset ultrafast \
	screen_video_${time_prefix}.mkv
exit 0
fi
case $1 in
	"audio") # 捕捉音频
		ffmpeg -y -f alsa -ac 2 -sample_rate 44100 -i sysdefault:CARD=Microphone audio_${time_prefix}.mp3
	;;
	"ca") # 提前视频的音频
		ffmpeg -i $2 -vn -y -acodec copy ${filename}_${time_prefix}.m4a
	;;
	"af") # 给视频添加字幕 srt格式
		ffmpeg -i $2 -vf subtitles=$3  ${filename}_${time_prefix}.mp4
	;;
	"cut") #裁减视频
		ffmpeg -ss 00:00:00 -t $3 -i $2 -vcodec copy -acodec copy ${filename}_${time_prefix}.mp4
	;;
	*)
		echo "invalid param $1"
	;;
esac

