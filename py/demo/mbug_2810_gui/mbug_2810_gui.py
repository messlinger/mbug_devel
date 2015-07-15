
from time import sleep, time
import mbug

upd_interval=0.5
dev = None

#--------------------------------------------------------------------------------------
# GUI event callbacks

def pre_dev_select():
    devlist = mbug.list(2810)
    if devlist==[]:
        print_status('No device found')
    cb_dev['values'] = devlist
    
def on_dev_select(ev):
    global dev
    print_status("Opening device...")
    try:
        dev = mbug.open( cb_dev.get() )
        print_status("Ok")
        update()
    except:
        print_status("Error opening device")
        raise

#--------------------------------------------------------------------------------------
# Create GUI

import Tkinter as tk
import ttk

root = tk.Tk()
root.title("MBUG-2810-Thermometer")
root.resizable(width=0, height=0)
root.iconbitmap(bitmap='icon-4.ico')

frame_disp = tk.LabelFrame(root, borderwidth=2, relief=tk.GROOVE, text='Temperature')
_dC=u" \N{DEGREE SIGN}C"
_void_T=u"\N{EM DASH}.\N{EM DASH}"
t_temp = tk.Label(frame_disp, text=_void_T+_dC, font=("Helvetica", 30) , width=8)

frame_bot = tk.Frame(root)
l_dev = tk.Label(frame_bot, text="Device:")
cb_dev = ttk.Combobox(frame_bot, width=16, postcommand=pre_dev_select )
cb_dev.bind("<<ComboboxSelected>>", lambda ev: on_dev_select(ev) )
l_stat = tk.Label(frame_bot, text="Status:")
t_stat = tk.Label(frame_bot, text="")


frame_disp.grid(column=0, row=0, padx=6, pady=6,)
t_temp.grid(column=0, row=0, padx=6, pady=2)

frame_bot.grid(column=0, row=1, padx=6, pady=6)
l_dev.grid(column=0, row=0, padx=6, pady=2, sticky='E')
cb_dev.grid(column=1, row=0, padx=6, pady=2, sticky='W')
l_stat.grid(column=0, row=1, padx=6, pady=2, sticky='E')
t_stat.grid(column=1, row=1, padx=6, pady=2, sticky='W')


#-------------------------------------------------------------------------------------

def print_status(msg):
    t_stat.config(text=str(msg))
def print_temp(T):
    if T==None: t = _void_T 
    else: t = "% .2f"% float(T)        
    t_temp.config(text= t+_dC)

def update():
    if dev==None: return
    try:
        T = dev.read()
        print_temp(T)
        print_status('Ok')
        root.after(int(upd_interval*1000), update)
    except:
        print_status('Error reading device')
        raise

#--------------------------------------------------------------------------------------
def main():
    root.mainloop()
    if dev:
        dev.close()
    
if __name__ == '__main__':
    main()
