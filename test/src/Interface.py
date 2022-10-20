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
        self.attributes('-topmost', True)
        
         # Variables
        parser.init_config('config.conf')
        self.PATH_WoW = parser.get_value('config.conf', 'PATH_WoW', '=')
        self.ACC_Info = parser.get_multiplevalues('config.conf', 'ACC_Infos', '(', ',', ')')
        self.MOVEMENT_KEY = [win32con.VK_RIGHT, win32con.VK_UP, win32con.VK_DOWN, win32con.VK_LEFT]
        
        self.listCoord = []
        self.PIXEL_COORD = []
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
        tabControl.add(tab1, text='Menu')
        tabControl.add(tab2, text='Options')
        tabControl.pack(expand = 1, fill ="both")
        
         # Widgets
        OptionList = ['1', '5', '10', '15', '20']
        self.numberClientsList = tk.StringVar(self)
        self.numberClientsList.set(OptionList[0])
        self.NBR_ACCOUNT = int(self.numberClientsList.get())
        self.WoWDirButton = tk.Button(tab2, image=self.iconWoW, command=lambda: self.selectWoWDir())
        self.WoWDirEntry = tk.Entry(tab2, state='normal', width = 26)
        self.WoWDirEntry.insert(0,self.PATH_WoW)
        self.WoWDirEntry.configure(state='disabled')
        self.ModifyCredentials_Button = tk.Button(tab2, text='Modify credentials', command=lambda: self.open_credentials_tab(), padx=5, pady=5)
        self.LaunchRepair_Button = tk.Button(tab1, text='Launch', command=lambda: self.launch_repair_clients(), padx=5, pady=5)
        self.ScriptOnOff_Label = tk.Label(tab1, text="OFF", foreground='red')
        self.NbrClient_Menu = tk.OptionMenu(tab1, self.numberClientsList, *OptionList)
        self.NbrClient_Label = tk.Label(tab1, text="Number clients:")
        
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
        self.NbrClient_Menu.config(width = 2)
        
         # Grid
        self.WoWDirButton.grid(row=0, column=0, sticky=tk.E, padx=2, pady=10)
        self.WoWDirEntry.grid(row=0, column=1, columnspan=2, padx=2)
        self.ModifyCredentials_Button.grid(row=2, column=1)
        self.ScriptOnOff_Label.grid(row=0, column=5, sticky=tk.E)
        self.LaunchRepair_Button.grid(row=1, column=0, columnspan=2)
        self.NbrClient_Label.grid(row=1, column=3, columnspan=2)
        self.NbrClient_Menu.grid(row=1, column=5, sticky=tk.W)
        self.Group_Label.grid(row=2, column=2, columnspan=3, pady=10)
        
    def show_infoAccounts(self, nbr):
        y = 0
        for i in range(nbr):
            if(i%2 == 0):
                self.Name_Label[i].grid(row=3+y, column=0)
                self.Class_Label[i].grid(row=3+y, column=1)
                self.Specialisation_Menu[i].grid(row=3+y, column=2)
            else:
                self.Name_Label[i].grid(row=3+y, column=3)
                self.Class_Label[i].grid(row=3+y, column=4)
                self.Specialisation_Menu[i].grid(row=3+y, column=5)
                y = y+1
        
    def quit_program(self):
        #Disconnect clients
        self.script_running = False
        for hwnd in self.hwndACC:
            if(win32gui.IsWindow(hwnd)):
                win32api.SendMessage(hwnd, win32con.WM_CLOSE, 0, 0)
        self.serverthread.running = False
        tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        tcp_socket.connect(('localhost', 50001))
        self.destroy()
        
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
        
    def send_client_txt(self, hwnd, txt):
        #Send text to window
        for c in txt:
            win32api.SendMessage(hwnd, win32con.WM_CHAR, ord(c), 0)
    
    def on_KeyPress(self, key):
        if(key == keyboard.Key.page_up):
            for i in range(self.NBR_ACCOUNT//5):
                if(win32gui.GetForegroundWindow() in self.hwndACC[i*5:(i*5)+5]):
                    for y in range(i*5, (i*5)+5):
                        win32api.SendMessage(self.hwndACC[y], win32con.WM_KEYDOWN, win32con.VK_PRIOR, 0)
                        win32api.SendMessage(self.hwndACC[y], win32con.WM_KEYUP, win32con.VK_PRIOR, 0)
                    return
        
    def adapt_listCoord(self):
        screenWidth = 1920; screenHeight = 1080
        coefPixel = [0.3015, 0.5666]
        coefPixel2 = [0.278, 0.58]
        if(self.NBR_ACCOUNT == 1):
            NBR_ACCOUNT_SCREEN1 = 1
            wWidth1 = screenWidth
            wHeight1 = screenHeight
        else:
            NBR_ACCOUNT_SCREEN1 = (self.NBR_ACCOUNT//5)
            if(NBR_ACCOUNT_SCREEN1 == 1): wWidth1 = screenWidth
            elif(NBR_ACCOUNT_SCREEN1 == 3): wWidth1 = int((screenWidth//3)*(1+(0.012*2)))
            else: wWidth1 = int((screenWidth//2)*1.018)
            if(NBR_ACCOUNT_SCREEN1 <= 3): wHeight1 = screenHeight
            else: wHeight1 = (screenHeight//2)
            wWidth2 = int((screenWidth//((self.NBR_ACCOUNT - NBR_ACCOUNT_SCREEN1)//2))*(1+(0.018*NBR_ACCOUNT_SCREEN1)))
            wHeight2 = int((screenHeight//2)*1.04)
        self.listCoord = []
        tmp = 0; tmp2 = 0
        for i in range(NBR_ACCOUNT_SCREEN1):
            if(i > 0): tmp2 = tmp2 + 2
            if(i == 0):
                self.listCoord.append((-8, 0, wWidth1, wHeight1))
                self.PIXEL_COORD.append( (int((wWidth1-8)*coefPixel[0]), int(wHeight1*coefPixel[1])) )
            elif(i == 1):
                self.listCoord.append((wWidth1-23, 0, wWidth1, wHeight1))
                self.PIXEL_COORD.append( (int(self.PIXEL_COORD[0][0]+8+wWidth1-23), int(wHeight1*coefPixel[1])) )
            elif(NBR_ACCOUNT_SCREEN1 == 3):
                self.listCoord.append(((wWidth1*2)-int(18.5*2), 0, wWidth1, wHeight1))
                self.PIXEL_COORD.append(( int(self.PIXEL_COORD[0][0]+8+(wWidth1*2)-int(18.5*2)), int(wHeight1*coefPixel[1]) ))
            elif(i == 2):
                self.listCoord.append((wWidth1-23, wHeight1-37, wWidth1, wHeight1))
                self.PIXEL_COORD.append(( int(self.PIXEL_COORD[0][0]+8+wWidth1-23), int(self.PIXEL_COORD[0][1]+wHeight1-37) ))
            elif(i == 3):
                self.listCoord.append((-8, wHeight1-37, wWidth1, wHeight1))
                self.PIXEL_COORD.append(( int((wWidth1-8)*coefPixel[0]), int(self.PIXEL_COORD[0][1]+wHeight1-37) ))
            if(self.NBR_ACCOUNT > 1):
                for y in range(4):
                    if(tmp == 0):
                        if(tmp2 == 0):
                            self.listCoord.append((screenWidth+(wWidth2*tmp2)-8, 0, wWidth2, wHeight2))
                            self.PIXEL_COORD.append(( int(int((wWidth2-8)*coefPixel2[0])+screenWidth+(wWidth2*tmp2)), int(wHeight2*coefPixel2[1]) ))
                        else:
                            self.listCoord.append((screenWidth+(wWidth2*tmp2)-(19*tmp2), 0, wWidth2, wHeight2))
                            self.PIXEL_COORD.append(( int(int((wWidth2-8)*coefPixel2[0])+8+screenWidth+(wWidth2*tmp2)-(19*tmp2)), int(wHeight2*coefPixel2[1]) ))
                    elif(tmp == 1):
                        if(tmp2 == 0):
                            self.listCoord.append((screenWidth+(wWidth2*(tmp2+1))-23, 0, wWidth2, wHeight2))
                            self.PIXEL_COORD.append(( int(int((wWidth2-8)*coefPixel2[0])+8+screenWidth+(wWidth2*(tmp2+1))-23), int(wHeight2*coefPixel2[1]) ))
                        else:
                            self.listCoord.append((screenWidth+(wWidth2*(tmp2+1))-int(18.5*(tmp2+1)), 0, wWidth2, wHeight2))
                            self.PIXEL_COORD.append(( int(int((wWidth2-8)*coefPixel2[0])+8+screenWidth+(wWidth2*(tmp2+1))-int(18.5*(tmp2+1))), int(wHeight2*coefPixel2[1]) ))
                    elif(tmp == 2):
                        if(tmp2 == 0):
                            self.listCoord.append((screenWidth+(wWidth2*(tmp2+1))-23, wHeight2-37, wWidth2, wHeight2))
                            self.PIXEL_COORD.append(( int(int((wWidth2-8)*coefPixel2[0])+8+screenWidth+(wWidth2*(tmp2+1))-23), int(int(wHeight2*coefPixel2[1])+wHeight2-37) ))
                        else:
                            self.listCoord.append((screenWidth+(wWidth2*(tmp2+1))-int(18.5*(tmp2+1)), wHeight2-37, wWidth2, wHeight2))
                            self.PIXEL_COORD.append(( int(int((wWidth2-8)*coefPixel2[0])+8+screenWidth+(wWidth2*(tmp2+1))-int(18.5*(tmp2+1))), int(int(wHeight2*coefPixel2[1])+wHeight2-37) ))
                    else:
                        if(tmp2 == 0):
                            self.listCoord.append((screenWidth+(wWidth2*tmp2)-8, wHeight2-37, wWidth2, wHeight2))
                            self.PIXEL_COORD.append(( int(int((wWidth2-8)*coefPixel2[0])+screenWidth+(wWidth2*tmp2)), int(int(wHeight2*coefPixel2[1])+wHeight2-37) ))
                        else:
                            self.listCoord.append((screenWidth+(wWidth2*tmp2)-(19*tmp2), wHeight2-37, wWidth2, wHeight2))
                            self.PIXEL_COORD.append(( int(int((wWidth2-8)*coefPixel2[0])+8+screenWidth+(wWidth2*tmp2)-(19*tmp2)), int(int(wHeight2*coefPixel2[1])+wHeight2-37) ))
                    if(tmp >= 3): tmp = 0
                    else: tmp = tmp+1
        
    def launch_repair_clients(self):
        #Launch or Repair clients
        if self.LaunchRepair_Button.config('text')[-1] == 'Launch':
            self.LaunchRepair_Button.config(text='Repair')
            self.NBR_ACCOUNT = int(self.numberClientsList.get())
            self.adapt_listCoord()
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
            print("Stop")
        else:
            self.script_running = True
            self.serverthread.sendAllClients(b"Bot: ON")
            self.ScriptOnOff_Label.config(text='ON')
            self.ScriptOnOff_Label.config(foreground='green')
            print("Running")
        

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
                    for i in range((self.index-(self.index%5))*5, ((self.index-(self.index%5))*5)+5): #Who is a Tank in group
                        if(isATank(interface.serverthread.clients_thread[i].Class, interface.serverthread.clients_thread[i].currentSpec)):
                            msg = ('Tank: '+interface.serverthread.clients_thread[i].Name)
                    for i in range((self.index-(self.index%5))*5, ((self.index-(self.index%5))*5)+5):
                        interface.serverthread.clients[i][0].send(bytes(msg, 'utf-8'))
                elif(isAMelee(self.Class, self.currentSpec) and not isAMelee(self.Class, SpecTMP)): #Was a Melee but changed
                    self.currentSpec = SpecTMP; msg = "null"
                    for i in range((self.index-(self.index%5))*5, ((self.index-(self.index%5))*5)+5): #Who is a Melee in group
                        if(isAMelee(interface.serverthread.clients_thread[i].Class, interface.serverthread.clients_thread[i].currentSpec)):
                            msg = ('Melee: '+interface.serverthread.clients_thread[i].Name)
                    for i in range((self.index-(self.index%5))*5, ((self.index-(self.index%5))*5)+5):
                        interface.serverthread.clients[i][0].send(bytes(msg, 'utf-8'))
                self.currentSpec = SpecTMP
                if(isATank(self.Class, self.currentSpec)): #Is now a Tank
                    msg = ('Tank: '+self.Name)
                    for i in range((self.index-(self.index%5))*5, ((self.index-(self.index%5))*5)+5):
                        interface.serverthread.clients[i][0].send(bytes(msg, 'utf-8'))
                elif(isAMelee(self.Class, self.currentSpec)): #Is now a Melee
                    msg = ('Melee: '+self.Name)
                    for i in range((self.index-(self.index%5))*5, ((self.index-(self.index%5))*5)+5):
                        interface.serverthread.clients[i][0].send(bytes(msg, 'utf-8'))
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
        
    def sendAllClients(self, msg):
        for i in range(len(self.clients)):
            if(self.clients[i] != 0): self.clients[i][0].send(msg)
        
    #Main :
if __name__== "__main__" :
    interface = Interface()
    
    mouse.on_middle_click(interface.activateBot)

    listener = keyboard.Listener(on_press=interface.on_KeyPress)
    listener.start()

    interface.mainloop()