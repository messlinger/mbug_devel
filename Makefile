
sysconfdir = /etc

.PHONY: all  lib  tool  install  python  uninstall  clean  realclean

all:  
	$(MAKE) -C  c/linux/  all 
	
lib:  
	$(MAKE) -C  c/linux/  lib 

tool:  
	$(MAKE) -C  c/linux/  tool
	
install: udev_rule
	$(MAKE) -C  c/linux/  install
	$(MAKE) -C  py/  install
	
python: udev_rule
	$(MAKE) -C  py/  install

udev_rule:
	echo  SUBSYSTEM==\"usb\", ATTRS{idVendor}==\"04d8\", ATTRS{idProduct}==\"fbc3\", MODE=\"0666\" \
	  > $(sysconfdir)/udev/rules.d/99-mbug.rules
	udevadm control --reload-rules

uninstall:
	$(MAKE) -C  c/linux/  uninstall
	rm  $(sysconfdir)/udev/rules.d/99-mbug.rules  
	udevadm control --reload-rules
	
clean:  
	$(MAKE) -C  c/linux/  clean
	
realclean:
	$(MAKE) -C  c/linux/  realclean

