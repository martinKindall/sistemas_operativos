
sudo mknod /dev/syncwrite0 c 65 0
sudo mknod /dev/syncwrite1 c 65 1

sudo insmod syncwrite.ko

sudo chmod a+rw /dev/syncwrite0
sudo chmod a+rw /dev/syncwrite1

# sudo rmmod syncwrite