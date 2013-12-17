
sysconfdir = /etc

.PHONY: all  lib  tool  install  python  uninstall  clean  realclean

all:  
	$(MAKE) -C  c/linux/  all 
	
lib:  
	$(MAKE) -C  c/linux/  lib 

tool:  
	$(MAKE) -C  c/linux/  tool
	
install:
	$(MAKE) -C  c/linux/  install
	$(MAKE) -C  py/  install
	cp  99-mbug.rules  $(sysconfdir)/udev/rules.d
	
python:
	$(MAKE) -C  py/  install
	cp  99-mbug.rules  $(sysconfdir)/udev/rules.d

uninstall:
	$(MAKE) -C  c/linux/  uninstall
	rm  $(sysconfdir)/udev/rules.d/99-mbug.rules  
	
clean:  
	$(MAKE) -C  c/linux/  clean
	
realclean:
	$(MAKE) -C  c/linux/  realclean

