#!/usr/bin/env zsh

export DISPLAY=:0.0
make && xrandr --output HDMI1 --auto --rotate left --primary
rc=$?
if [ $rc -eq 0 ]; then
	echo "awesome.restart()" | awesome-client
	# ./nuclear-compositor &
	trap 'killall nuclear*' INT
	"$@"
fi

xrandr --output HDMI1 --off
echo Closing
