ifconfig eth0 down
ifconfig wlan0 up

iwconfig wlan0 essid "joo"
wpa_supplicant -i wlan0 -c /etc/wpa_supplicant.conf &

ifconfig wlan0 192.168.43.110 netmask 255.255.255.0
route add default gw 192.168.43.1 dev wlan0

