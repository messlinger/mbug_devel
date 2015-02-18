import mbug_dev, time

d = mbug_dev.mbug_1220()

def Ccal(f):
    C0=34
    k=658e3
    return 1.*k/f - C0 

try:
	while(1):
		f,l = d.read()
		print "f=", f, "C=", Ccal(f), "level=", l
finally:
	d.close()
