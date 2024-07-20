build/disk.img:
	qemu-img create -f raw build/disk.img 1G
	@echo -en 'g\nn\n\n\n+256M\nn\n\n\n\n\np\nw\n' | sudo fdisk /var/lib/lfs/lfs.img

