import os
import sys
import time
import socket
import threading
import subprocess

from PIL import ImageTk, Image, ImageGrab
from tkinter import messagebox, filedialog
from pynput import keyboard
from tkinter import ttk
import tkinter as tk
import mouse
import PIL

import win32gui, win32con, win32api #pywin32

import parser

class Interface(tk.Tk):
    def __init__(self):
        super().__init__()
        
         # Window
        self.resizable(False,False)
        self.title('Multibox')
        self.protocol("WM_DELETE_WINDOW", self.quit_program)
        self.iconphoto(True, ImageTk.PhotoImage(Image.open(r"assets/icon.jpg")))
        self.iconWoW = tk.PhotoImage(file='assets/iconWoW.png')
        self.iconKeybind = tk.PhotoImage(file='assets/iconKeybind.png')
        self.attributes('-topmost', True)
        
         # Variables
        parser.init_config('config.conf')
        self.PATH_WoW = parser.get_value('config.conf', 'PATH_WoW', '=')
        self.ACC_Info = parser.get_multiplevalues('config.conf', 'ACC_Infos')
        self.KEYBIND_Info = parser.get_multiplevalues('config.conf', 'KEYBIND_Infos')
        self.MOVEMENT_KEY = [win32con.VK_RIGHT, win32con.VK_UP, win32con.VK_DOWN, win32con.VK_LEFT]
        
        self.listCoord = []
        self.hwndACC = [] #Get windows handle
        self.script_running = False
        self.InMovement = [0 for x in range(20)]
        self.indexIMG = 0
                
        self.serverthread = server_thread()
        self.serverthread.start()
        
         # Tabs
        tabControl = ttk.Notebook(self)
        tab1 = ttk.Frame(tabControl)
        tab2 = ttk.Frame(tabControl)
        tabControl.pack(expand = 1, fill ="both")
        tabControl.add(tab1, text='Menu')
        tabControl.add(tab2, text='Options')
        
         # Widgets
        self.NBR_ACCOUNT = 0
        self.WoWDirButton = tk.Button(tab2, image=self.iconWoW, command=lambda: self.selectWoWDir())
        self.WoWDirEntry = tk.Entry(tab2, state='normal', width = 39)
        self.WoWDirEntry.insert(0,self.PATH_WoW)
        self.WoWDirEntry.configure(state='disabled')
        self.ModifyCredentials_Button = tk.Button(tab2, text='Modify credentials', command=lambda: self.open_credentials_tab(), padx=5, pady=5)
        self.ModifyKeyBindings_Button = tk.Button(tab2, text='Modify key bindings', command=lambda: self.open_keybindings_tab(), padx=5, pady=5)
        self.LaunchRepair_Button = tk.Button(tab1, text='Launch', command=lambda: self.launch_repair_clients(), padx=5, pady=5)
        self.ScriptOnOff_Label = tk.Label(tab1, text="OFF", foreground='red')
        self.NbrClient_Entry = tk.Entry(tab1, state='normal', justify=tk.CENTER, width = 7)
        self.NbrClient_Label = tk.Label(tab1, text="Number clients:")
        self.TankAutoFocus = tk.IntVar()
        self.TankAutoFocus_CheckBtn = tk.Checkbutton(tab1, text = "Tank: auto focus", variable = self.TankAutoFocus, command=lambda: self.sendCheckbox("1"+str(self.TankAutoFocus.get())), onvalue = 1, offvalue = 0, height=2, width = 15)
        self.TankAutoMove = tk.IntVar()
        self.TankAutoMove_CheckBtn = tk.Checkbutton(tab1, text = "Tank: auto move", variable = self.TankAutoMove, command=lambda: self.sendCheckbox("2"+str(self.TankAutoMove.get())), onvalue = 1, offvalue = 0, height=2, width = 15)
        
         # Players and informations related
        self.Group_Label = tk.Label(tab1, text="Players detected:")
        self.Name_Label = []; self.Class_Label = []; self.SpecialisationList = []
        self.Specialisation_Menu = []; self.OptionList = []
        for i in range(25):
            self.Name_Label.append(tk.Label(tab1, text="Null", foreground='grey'))
            self.Class_Label.append(tk.Label(tab1, text="Null", foreground='grey'))
            self.SpecialisationList.append(tk.StringVar(self))
            self.OptionList.append(['Null'])
            self.SpecialisationList[i].set(self.OptionList[i][0])
            self.Specialisation_Menu.append(tk.OptionMenu(tab1, self.SpecialisationList[i], *self.OptionList[i]))
            self.Specialisation_Menu[i].config(width = 11)
        
         # Config
        self.LaunchRepair_Button.config(width = 6)
        
         # Grid
        self.WoWDirButton.grid(row=0, column=0, sticky=tk.E, padx=2, pady=10)
        self.WoWDirEntry.grid(row=0, column=1, columnspan=6, padx=2)
        self.ModifyCredentials_Button.grid(row=1, column=0, columnspan=3)
        self.ModifyKeyBindings_Button.grid(row=1, column=4, columnspan=3)
        self.ScriptOnOff_Label.grid(row=0, column=5, sticky=tk.E)
        self.LaunchRepair_Button.grid(row=1, column=0, columnspan=2, pady=2)
        self.NbrClient_Label.grid(row=1, column=3, columnspan=2, sticky=tk.E)
        self.NbrClient_Entry.grid(row=1, column=5, sticky=tk.W)
        self.TankAutoFocus_CheckBtn.grid(row=2, column=0, columnspan=3)
        self.TankAutoMove_CheckBtn.grid(row=2, column=3, columnspan=3)
        
    def show_infoAccounts(self, nbr):
        y = 0
        self.Group_Label.grid(row=3, column=2, columnspan=3, pady=10)
        for i in range(nbr):
            if(i%2 == 0):
                self.Name_Label[i].grid(row=4+y, column=0)
                self.Class_Label[i].grid(row=4+y, column=1)
                self.Specialisation_Menu[i].grid(row=4+y, column=2)
            else:
                self.Name_Label[i].grid(row=4+y, column=3)
                self.Class_Label[i].grid(row=4+y, column=4)
                self.Specialisation_Menu[i].grid(row=4+y, column=5)
                y = y+1
        self.WoWDirEntry.configure(width=60)
        
    def sendCheckbox(self, msg):
        msg = "C"+msg
        self.serverthread.sendTankClients(bytes(msg, 'utf-8'))
        
    def quit_program(self):
        #Disconnect clients
        self.script_running = False
        listener.stop()
        for hwnd in self.hwndACC:
            if(win32gui.IsWindow(hwnd)):
                win32api.SendMessage(hwnd, win32con.WM_CLOSE, 0, 0)
        self.serverthread.running = False
        tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        tcp_socket.connect(('localhost', 50001))
        tcp_socket.close()
        
    def selectWoWDir(self):
        tmp = filedialog.askopenfile(title='Select your wow vanilla client', filetypes=[('WoW', ['exe'])], initialdir=self.PATH_WoW)
        if(tmp != None and tmp.name != self.PATH_WoW):
            parser.modify_config('config.conf', path_wow=tmp.name)
            self.PATH_WoW = tmp.name
            self.WoWDirEntry.configure(state='normal')
            self.WoWDirEntry.delete(0,tk.END)
            self.WoWDirEntry.insert(0,tmp.name)
            self.WoWDirEntry.configure(state='disabled')
        
    def open_credentials_tab(self):
        global credentialTab
        global credentials_Entry
        try:
            if(credentialTab.state() == "normal"): credentialTab.focus()
        except:
            credentialTab = tk.Toplevel(self)
            credentialTab.title('Credentials')
            credentialTab.resizable(False,False)
            credentialTab.attributes('-topmost', True)
            account_Label = []; username_Label = []
            password_Label = []; credentials_Entry = []
            validate_Button = tk.Button(credentialTab, text='Validate changes', command=lambda: self.modify_crendentials())
            y = 0
            for i in range(25):
                 # Widgets
                account_Label.append(tk.Label(credentialTab, text="Account "+str(i+1)))
                username_Label.append(tk.Label(credentialTab, text="Username:"))
                password_Label.append(tk.Label(credentialTab, text="Password:"))
                credentials_Entry.append([tk.Entry(credentialTab, width = 15), tk.Entry(credentialTab, width = 15)])
                credentials_Entry[i][0].insert(0, self.ACC_Info[i][0])
                credentials_Entry[i][1].insert(0, self.ACC_Info[i][1])
                # Grid
                col = 0
                if(i >= 20): col = 4
                elif(i >= 10): col = 2
                if(i == 20 or i == 10): y = 0
                account_Label[i].grid(row=y, column=col+1)
                username_Label[i].grid(row=y+1, column=col)
                password_Label[i].grid(row=y+2, column=col)
                credentials_Entry[i][0].grid(row=y+1, column=col+1)
                credentials_Entry[i][1].grid(row=y+2, column=col+1)
                y = y+3
            validate_Button.grid(row=28, column=5, padx=2)
        
    def modify_crendentials(self):
        for i in range(25):
            self.ACC_Info[i] = (credentials_Entry[i][0].get(), credentials_Entry[i][1].get())
        parser.modify_config('config.conf', acc_info=self.ACC_Info)
        credentialTab.destroy()
        
    #==================================================#
        
    def open_keybindings_tab(self):
        global keybindingsTab
        global keybind_Entry
        try:
            if(keybindingsTab.state() == "normal"): keybindingsTab.focus()
        except:
            keybindingsTab = tk.Toplevel(self)
            keybindingsTab.title('Credentials')
            keybindingsTab.resizable(False,False)
            keybindingsTab.attributes('-topmost', True)
            keybind_Label = []; keybind_Entry = []; keybind_Button = []
            
            keybind_Label.append(tk.Label(keybindingsTab, text="Use Hearthstone:"))
            keybind_Entry.append(tk.Entry(keybindingsTab, state='normal', width = 15))
            keybind_Entry[0].delete(0,tk.END)
            keybind_Entry[0].insert(0, self.KEYBIND_Info[0][1])
            keybind_Entry[0].configure(state='disabled')
            keybind_Button.append(tk.Button(keybindingsTab, image=self.iconKeybind, command=lambda: self.bindKey(0)))
            
            keybind_Label.append(tk.Label(keybindingsTab, text="Use Mount:"))
            keybind_Entry.append(tk.Entry(keybindingsTab, state='normal', width = 15))
            keybind_Entry[1].delete(0,tk.END)
            keybind_Entry[1].insert(0, self.KEYBIND_Info[1][1])
            keybind_Entry[1].configure(state='disabled')
            keybind_Button.append(tk.Button(keybindingsTab, image=self.iconKeybind, command=lambda: self.bindKey(1)))
            
            keybind_Label.append(tk.Label(keybindingsTab, text="Order to target:"))
            keybind_Entry.append(tk.Entry(keybindingsTab, state='normal', width = 15))
            keybind_Entry[2].delete(0,tk.END)
            keybind_Entry[2].insert(0, self.KEYBIND_Info[2][1])
            keybind_Entry[2].configure(state='disabled')
            keybind_Button.append(tk.Button(keybindingsTab, image=self.iconKeybind, command=lambda: self.bindKey(2)))
            
            for i in range(len(keybind_Entry)):
                keybind_Label[i].grid(row=i, column=0)
                keybind_Entry[i].grid(row=i, column=1)
                keybind_Button[i].grid(row=i, column=2)
        
    def bindKey(self, index):
        if(index == 0): keybindingsTab.bind("<Key>", self.key_pressed_Hearthstone)
        elif(index == 1): keybindingsTab.bind("<Key>", self.key_pressed_Mount)
        elif(index == 2): keybindingsTab.bind("<Key>", self.key_pressed_Target)
        
    def key_pressed_Hearthstone(self, event):
        keybindingsTab.unbind("<Key>")
        keybind_Entry[0].configure(state='normal')
        keybind_Entry[0].delete(0,tk.END)
        keybind_Entry[0].insert(0, event.keysym)
        keybind_Entry[0].configure(state='disabled')
        self.KEYBIND_Info[0] = (str(event.keycode), event.keysym)
        parser.modify_config('config.conf', keybind_info=self.KEYBIND_Info)
        
    def key_pressed_Mount(self, event):
        keybindingsTab.unbind("<Key>")
        keybind_Entry[1].configure(state='normal')
        keybind_Entry[1].delete(0,tk.END)
        keybind_Entry[1].insert(0, event.keysym)
        keybind_Entry[1].configure(state='disabled')
        self.KEYBIND_Info[1] = (str(event.keycode), event.keysym)
        parser.modify_config('config.conf', keybind_info=self.KEYBIND_Info)
        
    def key_pressed_Target(self, event):
        keybindingsTab.unbind("<Key>")
        keybind_Entry[2].configure(state='normal')
        keybind_Entry[2].delete(0,tk.END)
        keybind_Entry[2].insert(0, event.keysym)
        keybind_Entry[2].configure(state='disabled')
        self.KEYBIND_Info[2] = (str(event.keycode), event.keysym)
        parser.modify_config('config.conf', keybind_info=self.KEYBIND_Info)
        
    #==================================================#
        
    def send_client_txt(self, hwnd, txt):
        #Send text to window
        for c in txt:
            win32api.SendMessage(hwnd, win32con.WM_CHAR, ord(c), 0)
    
    def on_KeyPress(self, key):
        try:
            key_code = key.vk
        except AttributeError:
            key_code = key.value.vk
        for i in range(3):
            if(str(key_code) == self.KEYBIND_Info[i][0]):
                msg = "K"+str(i+1)
                self.serverthread.sendGroupClients(bytes(msg, 'utf-8'))
                break
        
    def adapt_listCoord(self, nbr_monitor):
        x_gap = 8
        if(nbr_monitor == 1):
            monitor = win32api.EnumDisplayMonitors()
            monitor_x0 = monitor[0][2][0]; monitor_y0 = monitor[0][2][1]
            monitor_width = monitor[0][2][2] - monitor_x0
            monitor_height = monitor[0][2][3] - monitor_y0
            if(self.NBR_ACCOUNT == 1):
                self.listCoord.append((monitor_x0, monitor_y0, monitor_width, monitor_height))
            elif(self.NBR_ACCOUNT == 2):
                self.listCoord.append((monitor_x0, monitor_y0, monitor_width//2, monitor_height))
                self.listCoord.append((monitor_x0+(monitor_width//2), monitor_y0, monitor_width//2, monitor_height))
            elif(self.NBR_ACCOUNT == 3):
                self.listCoord.append((monitor_x0, monitor_y0, monitor_width//2, monitor_height))
                self.listCoord.append((monitor_x0+(monitor_width//2), monitor_y0, monitor_width//2, monitor_height//2))
                self.listCoord.append((monitor_x0+(monitor_width//2), monitor_y0+(monitor_height//2), monitor_width//2, monitor_height//2))
            elif(self.NBR_ACCOUNT == 4):
                self.listCoord.append((monitor_x0, monitor_y0, monitor_width//2, monitor_height//2))
                self.listCoord.append((monitor_x0+(monitor_width//2), monitor_y0, monitor_width//2, monitor_height//2))
                self.listCoord.append((monitor_x0+(monitor_width//2), monitor_y0+(monitor_height//2), monitor_width//2, monitor_height//2))
                self.listCoord.append((monitor_x0, monitor_y0+(monitor_height//2), monitor_width//2, monitor_height//2))
            elif(self.NBR_ACCOUNT == 5):
                self.listCoord.append((monitor_x0, monitor_y0, monitor_width//2, monitor_height//2))
                self.listCoord.append((monitor_x0+(monitor_width//2), monitor_y0, monitor_width//2, monitor_height//2))
                self.listCoord.append((monitor_x0+((monitor_width*2)//3), monitor_y0+(monitor_height//2), monitor_width//3, monitor_height//2))
                self.listCoord.append((monitor_x0, monitor_y0+(monitor_height//2), monitor_width//3, monitor_height//2))
                self.listCoord.append((monitor_x0+(monitor_width//3), monitor_y0+(monitor_height//2), monitor_width//3, monitor_height//2))
        elif(nbr_monitor >= 2):
            monitor = win32api.EnumDisplayMonitors()
            monitor1_x0 = monitor[0][2][0]; monitor1_y0 = monitor[0][2][1]
            monitor1_x1 = monitor[0][2][2]; monitor1_y1 = monitor[0][2][3]
            monitor1_width = monitor1_x1 - monitor1_x0
            monitor1_height = monitor1_y1 - monitor1_y0
            monitor2_x0 = monitor[1][2][0]; monitor2_y0 = monitor[1][2][1]
            monitor2_x1 = monitor[1][2][2]; monitor2_y1 = monitor[1][2][3]
            monitor2_width = monitor[1][2][2] - monitor2_x0
            monitor2_height = monitor[1][2][3] - monitor2_y0
            resize_coef = [0.017, 1.017]
            if(self.NBR_ACCOUNT == 1):
                self.listCoord.append((monitor1_x0-x_gap, monitor1_y0, monitor1_width, monitor1_height))
            elif(self.NBR_ACCOUNT == 2):
                self.listCoord.append((monitor1_x0-x_gap, monitor1_y0, monitor1_width, monitor1_height))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0, monitor2_width, monitor2_height))
            elif(self.NBR_ACCOUNT == 3):
                self.listCoord.append((monitor1_x0-x_gap, monitor1_y0, monitor1_width, monitor1_height))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*1)))//2, monitor2_height))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//2), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*1)))//2, monitor2_height))
            elif(self.NBR_ACCOUNT == 4):
                self.listCoord.append((monitor1_x0-x_gap, monitor1_y0, monitor1_width, monitor1_height))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*1)))//2, monitor2_height))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//2), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*1)))//2, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//2), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*1)))//2, int(monitor2_height*resize_coef[1])//2))
            elif(self.NBR_ACCOUNT == 5):
                self.listCoord.append((monitor1_x0-x_gap, monitor1_y0, monitor1_width, monitor1_height))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*1)))//2, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//2), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*1)))//2, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//2), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*1)))//2, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*1)))//2, int(monitor2_height*resize_coef[1])//2))
            elif(self.NBR_ACCOUNT == 6):
                self.listCoord.append((monitor1_x0-x_gap, monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, monitor1_height))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*1)))//2, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//2), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*1)))//2, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//2), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*1)))//2, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*1)))//2, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap+(monitor1_width//2), monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, monitor1_height))
            elif(self.NBR_ACCOUNT == 7):
                self.listCoord.append((monitor1_x0-x_gap, monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, monitor1_height))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*1)))//2, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//2), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*1)))//2, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*2//3), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*1.5)))//3, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//3), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*1.5)))//3, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap+(monitor1_width//2), monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, monitor1_height))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*1.5)))//3, int(monitor2_height*resize_coef[1])//2))
            elif(self.NBR_ACCOUNT == 8):
                self.listCoord.append((monitor1_x0-x_gap, monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, monitor1_height))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*1.5)))//3, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//3), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*1.5)))//3, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*2//3), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*1.5)))//3, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*2//3), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*1.5)))//3, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap+(monitor1_width//2), monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, monitor1_height))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//3), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*1.5)))//3, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*1.5)))//3, int(monitor2_height*resize_coef[1])//2))
            elif(self.NBR_ACCOUNT == 9):
                self.listCoord.append((monitor1_x0-x_gap, monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, monitor1_height))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*1.5)))//3, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//3), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*1.5)))//3, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*2//3), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*1.5)))//3, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*3//4), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*2)))//4, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap+(monitor1_width//2), monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, monitor1_height))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*2//4), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*2)))//4, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//4), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*2)))//4, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*2)))//4, int(monitor2_height*resize_coef[1])//2))
            elif(self.NBR_ACCOUNT == 10):
                self.listCoord.append((monitor1_x0-x_gap, monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, monitor1_height))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*2)))//4, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//4), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*2)))//4, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*2//4), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*2)))//4, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*3//4), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*2)))//4, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap+(monitor1_width//2), monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, monitor1_height))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*3//4), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*2)))//4, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*2//4), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*2)))//4, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//4), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*2)))//4, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*2)))//4, int(monitor2_height*resize_coef[1])//2))
            elif(self.NBR_ACCOUNT == 11):
                self.listCoord.append((monitor1_x0-x_gap, monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, monitor1_height))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*2)))//4, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//4), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*2)))//4, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*2//4), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*2)))//4, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*3//4), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*2)))//4, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap+(monitor1_width//2), monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*3//4), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*2)))//4, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*2//4), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*2)))//4, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//4), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*2)))//4, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*2)))//4, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap+(monitor1_width//2), monitor1_y0+(monitor1_height//2), int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
            elif(self.NBR_ACCOUNT == 12):
                self.listCoord.append((monitor1_x0-x_gap, monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, monitor1_height))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*2)))//4, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//4), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*2)))//4, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*2//4), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*2)))//4, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*3//4), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*2)))//4, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap+(monitor1_width//2), monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*4//5), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*2.5)))//5, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*3//5), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*2.5)))//5, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*2//5), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*2.5)))//5, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//5), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*2.5)))//5, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap+(monitor1_width//2), monitor1_y0+(monitor1_height//2), int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*2.5)))//5, int(monitor2_height*resize_coef[1])//2))
            elif(self.NBR_ACCOUNT == 13):
                self.listCoord.append((monitor1_x0-x_gap, monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, monitor1_height))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*2.5)))//5, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//5), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*2.5)))//5, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*2//5), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*2.5)))//5, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*3//5), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*2.5)))//5, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap+(monitor1_width//2), monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*4//5), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*2.5)))//5, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*4//5), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*2.5)))//5, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*3//5), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*2.5)))//5, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*2//5), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*2.5)))//5, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap+(monitor1_width//2), monitor1_y0+(monitor1_height//2), int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//5), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*2.5)))//5, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*2.5)))//5, int(monitor2_height*resize_coef[1])//2))
            elif(self.NBR_ACCOUNT == 14):
                self.listCoord.append((monitor1_x0-x_gap, monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, monitor1_height))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*2.5)))//5, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//5), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*2.5)))//5, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*2//5), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*2.5)))//5, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*3//5), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*2.5)))//5, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap+(monitor1_width//2), monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*4//5), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*2.5)))//5, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*5//6), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*4//6), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*3//6), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap+(monitor1_width//2), monitor1_y0+(monitor1_height//2), int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*2//6), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//6), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
            elif(self.NBR_ACCOUNT == 15):
                self.listCoord.append((monitor1_x0-x_gap, monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, monitor1_height))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//6), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*2//6), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*3//6), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap+(monitor1_width//2), monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*4//6), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*5//6), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*5//6), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*4//6), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap+(monitor1_width//2), monitor1_y0+(monitor1_height//2), int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*3//6), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*2//6), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//6), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
            elif(self.NBR_ACCOUNT == 16):
                self.listCoord.append((monitor1_x0-x_gap, monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//6), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*2//6), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*3//6), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap+(monitor1_width//2), monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*4//6), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*5//6), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*5//6), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*4//6), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap+(monitor1_width//2), monitor1_y0+(monitor1_height//2), int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*3//6), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*2//6), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//6), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap, monitor1_y0+(monitor1_height//2), int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
            elif(self.NBR_ACCOUNT == 17):
                self.listCoord.append((monitor1_x0-x_gap, monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//6), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*2//6), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*3//6), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap+(monitor1_width//2), monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*4//6), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*5//6), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3)))//6, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*6//7), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3.5)))//7, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*5//7), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3.5)))//7, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap+(monitor1_width//2), monitor1_y0+(monitor1_height//2), int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*4//7), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3.5)))//7, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*3//7), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3.5)))//7, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*2//7), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3.5)))//7, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//7), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3.5)))//7, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap, monitor1_y0+(monitor1_height//2), int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3.5)))//7, int(monitor2_height*resize_coef[1])//2))
            elif(self.NBR_ACCOUNT == 18):
                self.listCoord.append((monitor1_x0-x_gap, monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3.5)))//7, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//7), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3.5)))//7, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*2//7), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3.5)))//7, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*3//7), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3.5)))//7, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap+(monitor1_width//2), monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*4//7), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3.5)))//7, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*5//7), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3.5)))//7, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*6//7), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3.5)))//7, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*6//7), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3.5)))//7, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap+(monitor1_width//2), monitor1_y0+(monitor1_height//2), int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*5//7), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3.5)))//7, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*4//7), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3.5)))//7, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*3//7), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3.5)))//7, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*2//7), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3.5)))//7, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap, monitor1_y0+(monitor1_height//2), int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//7), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3.5)))//7, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*3.5)))//7, int(monitor2_height*resize_coef[1])//2))
            elif(self.NBR_ACCOUNT == 19):
                self.listCoord.append((monitor1_x0-x_gap, monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3.5)))//7, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//7), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3.5)))//7, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*2//7), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3.5)))//7, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*3//7), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3.5)))//7, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap+(monitor1_width//2), monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*4//7), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3.5)))//7, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*5//7), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3.5)))//7, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*6//7), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*3.5)))//7, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*7//8), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*4)))//8, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap+(monitor1_width//2), monitor1_y0+(monitor1_height//2), int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*6//8), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*4)))//8, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*5//8), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*4)))//8, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*4//8), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*4)))//8, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*3//8), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*4)))//8, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap, monitor1_y0+(monitor1_height//2), int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*2//8), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*4)))//8, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//8), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*4)))//8, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*4)))//8, int(monitor2_height*resize_coef[1])//2))
            elif(self.NBR_ACCOUNT == 20):
                self.listCoord.append((monitor1_x0-x_gap, monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*4)))//8, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//8), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*4)))//8, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*2//8), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*4)))//8, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*3//8), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*4)))//8, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap+(monitor1_width//2), monitor1_y0, int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*4//8), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*4)))//8, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*5//8), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*4)))//8, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*6//8), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*4)))//8, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*7//8), monitor2_y0, int(monitor2_width*(1+(resize_coef[0]*4)))//8, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap+(monitor1_width//2), monitor1_y0+(monitor1_height//2), int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*7//8), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*4)))//8, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*6//8), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*4)))//8, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*5//8), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*4)))//8, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*4//8), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*4)))//8, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor1_x0-x_gap, monitor1_y0+(monitor1_height//2), int(monitor1_width*(1+resize_coef[0]))//2, int(monitor1_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*3//8), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*4)))//8, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width*2//8), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*4)))//8, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap+(monitor2_width//8), monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*4)))//8, int(monitor2_height*resize_coef[1])//2))
                self.listCoord.append((monitor2_x0-x_gap, monitor2_y0+(monitor2_height//2), int(monitor2_width*(1+(resize_coef[0]*4)))//8, int(monitor2_height*resize_coef[1])//2))
        
    def launch_repair_clients(self):
        #Launch or Repair clients
        if self.LaunchRepair_Button.config('text')[-1] == 'Launch':
            if(self.NbrClient_Entry.get() != ""): self.NBR_ACCOUNT = int(self.NbrClient_Entry.get())
            nbr_monitor = win32api.GetSystemMetrics(win32con.SM_CMONITORS)
            if(self.NbrClient_Entry.get() == ""): messagebox.showerror('Error', 'You must specify the number of clients !')
            elif(self.NBR_ACCOUNT <= 0): messagebox.showerror('Error', 'You can\'t have '+str(self.NBR_ACCOUNT)+' clients !')
            elif((self.NBR_ACCOUNT > 5 and nbr_monitor <= 1) or (self.NBR_ACCOUNT > 20)): #25 soon...
                messagebox.showerror('Error', 'Not enough monitors (' + str(nbr_monitor) + ' currently) for ' + str(self.NBR_ACCOUNT) + ' accounts')
            else:
                self.NbrClient_Entry.configure(state='disabled')
                self.LaunchRepair_Button.config(text='Repair')
                self.adapt_listCoord(nbr_monitor)
                for i in range(self.NBR_ACCOUNT):
                    p1 = subprocess.Popen(["./src/Bootstrap.exe", self.PATH_WoW])
                    p1.wait()
                    time.sleep(1)
                    hwnd = win32gui.FindWindow(None, "World of Warcraft")
                    win32gui.SetWindowText(hwnd, "WoW"+str(i+1))
                    self.hwndACC.append(hwnd)
                    if(i == 0 and self.NBR_ACCOUNT <= 5): win32gui.ShowWindow(hwnd, win32con.SW_MAXIMIZE)
                    else: win32gui.MoveWindow(hwnd, self.listCoord[i][0], self.listCoord[i][1], self.listCoord[i][2], self.listCoord[i][3], True)
                for i in range(self.NBR_ACCOUNT): #Enter username/password
                    self.send_client_txt(self.hwndACC[i], self.ACC_Info[i][0])
                    win32api.SendMessage(self.hwndACC[i], win32con.WM_KEYDOWN, win32con.VK_TAB, 0)
                    win32api.SendMessage(self.hwndACC[i], win32con.WM_KEYUP, win32con.VK_TAB, 0)
                    self.send_client_txt(self.hwndACC[i], self.ACC_Info[i][1])
                    time.sleep(0.1)
                    win32api.SendMessage(self.hwndACC[i], win32con.WM_KEYDOWN, win32con.VK_RETURN, 0)
                    win32api.SendMessage(self.hwndACC[i], win32con.WM_KEYUP, win32con.VK_RETURN, 0)
                self.show_infoAccounts(self.NBR_ACCOUNT)
        else: #Repair
            for i in range(self.NBR_ACCOUNT):
                if(not win32gui.IsWindow(self.hwndACC[i])):
                    p1 = subprocess.Popen(["./src/Bootstrap.exe", self.PATH_WoW])
                    p1.wait()
                    hwnd = win32gui.FindWindow(None, "World of Warcraft")
                    win32gui.SetWindowText(hwnd, "WoW"+str(i+1))
                    self.hwndACC[i] = hwnd
                    self.send_client_txt(self.hwndACC[i], self.ACC_Info[i][0])
                    win32api.SendMessage(self.hwndACC[i], win32con.WM_KEYDOWN, win32con.VK_TAB, 0)
                    win32api.SendMessage(self.hwndACC[i], win32con.WM_KEYUP, win32con.VK_TAB, 0)
                    self.send_client_txt(self.hwndACC[i], self.ACC_Info[i][1])
                    time.sleep(0.1)
                    win32api.SendMessage(self.hwndACC[i], win32con.WM_KEYDOWN, win32con.VK_RETURN, 0)
                    win32api.SendMessage(self.hwndACC[i], win32con.WM_KEYUP, win32con.VK_RETURN, 0)
                if(i == 0 and self.NBR_ACCOUNT == 5): win32gui.ShowWindow(self.hwndACC[i], win32con.SW_MAXIMIZE)
                else: win32gui.MoveWindow(self.hwndACC[i], self.listCoord[i][0], self.listCoord[i][1], self.listCoord[i][2], self.listCoord[i][3], True)
            
    def activateBot(self):
        self.MOVEMENT_KEY = [win32con.VK_RIGHT, win32con.VK_UP, win32con.VK_DOWN, win32con.VK_LEFT]
        if(self.script_running):
            self.script_running = False
            self.serverthread.sendAllClients(b"Bot: OFF")
            self.ScriptOnOff_Label.config(text='OFF')
            self.ScriptOnOff_Label.config(foreground='red')
        else:
            self.script_running = True
            self.serverthread.sendAllClients(b"Bot: ON")
            self.ScriptOnOff_Label.config(text='ON')
            self.ScriptOnOff_Label.config(foreground='green')
        

def rgb_hack(rgb):
    return "#%02x%02x%02x" % rgb 
    
def isATank(Class, Spec):
    if(Spec == "Protection" or Spec == "Feral"): return True
    else: return False
    
def isAMelee(Class, Spec):
    if(not isATank(Class, Spec) and (Class == "Warrior" or Class == "Rogue" or (Class == "Paladin" and Spec == "Retribution") or (Class == "Shaman" and Spec == "Enhancement"))):
        return True
    else: return False

class client_thread(threading.Thread):
    def __init__(self, index, conn, addr):
        super(client_thread, self).__init__()
        self.running = True
        self.conn = conn
        self.addr = addr
        self.index = index
        self.currentSpec = "Null"
        self.specChoice = 0
        self.Name = ""
        self.Class = ""
       
    def run(self):
        interface.after(500, self.checkSpecChange)
        while(self.running):
            try:
                data = self.conn.recv(128)
                if(data):
                    data = data.decode('utf-8')
                    ind = data.find("Class")
                    if(ind > -1):
                        self.Name = data[5:ind]
                        self.Class = data[ind+6::]
                        interface.Name_Label[self.index].config(text=self.Name)
                        interface.Class_Label[self.index].config(text=self.Class)
                        if(self.Class == "Druid"):
                            interface.OptionList[self.index] = ['Balance', 'Feral', 'Restoration']
                            color = "orange"
                        elif(self.Class == "Hunter"):
                            interface.OptionList[self.index] = ['Beast Mastery', 'Marksmanship', 'Survival']
                            color = "green"
                        elif(self.Class == "Mage"):
                            interface.OptionList[self.index] = ['Arcane', 'Fire', 'Frost']
                            color = rgb_hack((0, 210, 255))
                        elif(self.Class == "Paladin"):
                            interface.OptionList[self.index] = ['Holy', 'Protection', 'Retribution']
                            color = rgb_hack((255, 0, 122))
                        elif(self.Class == "Priest"):
                            interface.OptionList[self.index] = ['Discipline', 'Holy', 'Shadow']
                            color = rgb_hack((210, 210, 210))
                        elif(self.Class == "Rogue"):
                            interface.OptionList[self.index] = ['Assassination', 'Combat', 'Subtlety']
                            color = rgb_hack((255, 210, 0))
                        elif(self.Class == "Shaman"):
                            interface.OptionList[self.index] = ['Elemental', 'Enhancement', 'Restoration']
                            color = "blue"
                        elif(self.Class == "Warlock"):
                            interface.OptionList[self.index] = ['Affliction', 'Demonology', 'Destruction']
                            color = "purple"
                        elif(self.Class == "Warrior"):
                            interface.OptionList[self.index] = ['Arms', 'Fury', 'Protection']
                            color = "brown" 
                        elif(self.Class == "Null"):
                            interface.OptionList[self.index] = ['Null'] 
                            color = "grey"
                        interface.Specialisation_Menu[self.index]['menu'].delete(0, tk.END)
                        for option in interface.OptionList[self.index]:
                            interface.Specialisation_Menu[self.index]['menu'].add_command(label=option, command=tk._setit(interface.SpecialisationList[self.index], option))
                        if(self.Class != "Null"): interface.SpecialisationList[self.index].set(interface.OptionList[self.index][self.specChoice])
                        else: interface.SpecialisationList[self.index].set(interface.OptionList[self.index][0])
                        interface.Name_Label[self.index].config(foreground="black")
                        interface.Class_Label[self.index].config(foreground=color)
                    #print("Client " + self.addr[0] + ":" + str(self.addr[1]) + " - message: " + data)
            except Exception as e:
                interface.serverthread.clients[self.index] = 0
                self.running = False
                self.conn.close()
                if(win32gui.IsWindow(interface.hwndACC[self.index])):
                    win32api.SendMessage(interface.hwndACC[self.index], win32con.WM_CLOSE, 0, 0)
        print("[-] Client disconnected: " + self.addr[0] + ":" + str(self.addr[1]))
        
    def checkSpecChange(self):
        if(self.running):
            SpecTMP = interface.SpecialisationList[self.index].get()
            if(self.currentSpec != SpecTMP):
                if(isATank(self.Class, self.currentSpec) and not isATank(self.Class, SpecTMP)): #Was a Tank but changed
                    self.currentSpec = SpecTMP; msg = "null"
                    for i in range((self.index-(self.index%5)), (self.index-(self.index%5))+((interface.NBR_ACCOUNT-1)%5)+1): #Who is a Tank in group
                        if(isATank(interface.serverthread.clients_thread[i].Class, interface.serverthread.clients_thread[i].currentSpec)):
                            msg = ('Tank: '+interface.serverthread.clients_thread[i].Name)
                    interface.serverthread.sendGroupClients(bytes(msg, 'utf-8'), self.index)
                elif(isAMelee(self.Class, self.currentSpec) and not isAMelee(self.Class, SpecTMP)): #Was a Melee but changed
                    self.currentSpec = SpecTMP; msg = "null"
                    for i in range((self.index-(self.index%5)), (self.index-(self.index%5))+((interface.NBR_ACCOUNT-1)%5)+1): #Who is a Melee in group
                        if(isAMelee(interface.serverthread.clients_thread[i].Class, interface.serverthread.clients_thread[i].currentSpec)):
                            msg = ('Melee: '+interface.serverthread.clients_thread[i].Name)
                    interface.serverthread.sendGroupClients(bytes(msg, 'utf-8'), self.index)
                self.currentSpec = SpecTMP
                if(isATank(self.Class, self.currentSpec)): #Is now a Tank
                    msg = ('Tank: '+self.Name)
                    interface.serverthread.sendGroupClients(bytes(msg, 'utf-8'), self.index)
                elif(isAMelee(self.Class, self.currentSpec)): #Is now a Melee
                    msg = ('Melee: '+self.Name)
                    interface.serverthread.sendGroupClients(bytes(msg, 'utf-8'), self.index)
                for i in range(len(interface.OptionList[self.index])):
                    if(self.currentSpec == interface.OptionList[self.index][i]):
                        if(self.currentSpec != "Null"): self.specChoice = i
                        msg = ('Spec: '+str(i)+' ')
                        self.conn.send(bytes(msg, 'utf-8'))
            interface.after(500, self.checkSpecChange)
        
class server_thread(threading.Thread):
    def __init__(self):
        super(server_thread, self).__init__()
        self.running = True
        self.clients = []
        self.clients_thread = []

    def run(self):
        # Set up a TCP/IP server
        tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = ('localhost', 50001)
        tcp_socket.bind(server_address)
        tcp_socket.listen(25)
        while self.running:
            # blocking call, waits to accept a connection
            conn, addr = tcp_socket.accept()
            indextmp = -1
            for i in range(len(self.clients)):
                if(self.clients[i] == 0): indextmp = i
            if(indextmp == -1):
                self.clients.append((conn, addr))
                self.clients_thread.append(client_thread(len(self.clients)-1, conn, addr))
                self.clients_thread[len(self.clients_thread)-1].start()
            else:
                self.clients[indextmp] = (conn, addr)
                self.clients_thread[indextmp] = client_thread(indextmp, conn, addr)
                self.clients_thread[indextmp].start()
            print("[+] New client: " + addr[0] + ":" + str(addr[1]))
        for clientthread in self.clients_thread:
            clientthread.running = False
            clientthread.conn.close()
        print("server over...")
        interface.destroy()
        
    def sendAllClients(self, msg):
        for i in range(len(self.clients)):
            if(self.clients[i] != 0): self.clients[i][0].send(msg)
        
    def sendGroupClients(self, msg, index=-1):
        if(len(self.clients) > 0):
            if(index == -1):
                for i in range(interface.NBR_ACCOUNT//5):
                    if(win32gui.GetForegroundWindow() in interface.hwndACC[i*5:(i*5)+((interface.NBR_ACCOUNT-1)%5)+1]):
                        for y in range(i*5, (i*5)+((interface.NBR_ACCOUNT-1)%5)+1):
                            if(self.clients[y] != 0): self.clients[y][0].send(msg)
                            """win32api.SendMessage(self.hwndACC[y], win32con.WM_KEYDOWN, key_code, 0)
                            win32api.SendMessage(self.hwndACC[y], win32con.WM_KEYUP, key_code, 0)"""
                        return
            else:
                for y in range(index-(index%5), (index-(index%5))+((interface.NBR_ACCOUNT-1)%5)+1):
                    if(self.clients[y] != 0): self.clients[y][0].send(msg)
                return
                
    def sendTankClients(self, msg):
        if(len(self.clients) > 0):
            if(interface.NBR_ACCOUNT == 1):
                if(self.clients[0] != 0): self.clients[0][0].send(msg)
            else:
                for i in range(interface.NBR_ACCOUNT//5):
                    if(self.clients[i*5] != 0): self.clients[i*5][0].send(msg)
        
    #Main :
if __name__== "__main__" :
    interface = Interface()
    
    mouse.on_middle_click(interface.activateBot)

    listener = keyboard.Listener(on_press=interface.on_KeyPress)
    listener.start()

    interface.mainloop()
    
    print("\nYou can close this window...")