# -*- coding: utf-8 -*-
"""
Created on Thu Mar 29 10:50:36 2018

@author: MAZELLA_SICSIC
"""
import os
import time
import serial
import datetime
import tkinter
from tkinter import *
from tkinter.messagebox import *
from tkinter.ttk import *
import serial.tools.list_ports
 
ser = serial.Serial()
ser.baudrate = 9600
ser.timeout=0
ser.write_timeout=0

conn=0
serBuffer=""
usersFilePath= "users.csv"
tmpUsersFilePath= "tmpusers.csv"
transactions=[]
waitNewBadge =0
userData=["","","",""]
transac=[]

def alert():
    showinfo("Info ERIA", "Action à programmer")
    
def alertNoConn():
    showinfo("Info ERIA", "Veuillez connecter un lecteur")
    
def errUserAdd():
    showinfo("Info ERIA", "Merci de remplir tous les champs")
    
def about():
    showinfo("Info ERIA", "Système de paiement ERIA, développé par Robin Sicsic et Thomas Mazella (promotion 2020)")
    
def openUsersFile():
    os.startfile('users.csv')
    
def openCountsFile():
    os.startfile('counts.csv')
    
def connectPort():
    global conn
    global ser
    port_get=portSelect.get()
    if (conn == 0):
        if port_get=="" :
            showwarning("Info ERIA", "Veuillez séléctionner un port valide ou brancher un lecteur ERIA.")
        else :
            comInfoText.set('Connecté au port '+port_get)
            btnText.set('Déconnexion')
            ser.port = port_get
            ser.open()
            portSelect.config(state=DISABLED)
            conn=1
            time.sleep(2.5)
            ser.write(("42\n").encode())
            readSerial()
    else :
        ser.write(("43\n").encode())
        ser.close()
        portSelect.config(state="readonly")
        btnText.set('Connexion')
        comInfoText.set('Déconnecté')
        conn=0
        comInText.set('Aucune donnée')
        
def readSerial():
    global serBuffer
    global transactions
    global waitNewBadge
    clk = datetime.datetime.now()
    clok=""
    for i in range(0,19):
        clok=clok+str(clk)[i]
    clock.set(clok)
    while conn==1:
        c = ser.read()
        if len(c) == 0:
            break
        if c == b'\r':
            c = b''
        if c == b'\n':
            #serBuffer += "\n"
            print(serBuffer)
            if '&' in serBuffer:
                index = serBuffer.index("&")
                value = serBuffer[index+1:len(serBuffer)]
                identifier = serBuffer[0:index]
                pay(identifier, value, clok)
            else:
                wallet=getWallet(serBuffer)
                if wallet==-1:
                    if waitNewBadge==0:
                        ser.write(("16\n").encode()) #BADGE INEXISTANT
                        print("16\n")
                    else:
                        addUser(serBuffer,userData[1],userData[2],userData[3])
                        ser.write(("21\n").encode()) #BADGE AJOUTE
                        print("21\n")
                        waitNewBadge=0
                else:
                    if waitNewBadge==0:
                        ser.write(("14&"+str(conditionnementSolde(wallet))+"\n").encode())
                        print("14&"+str(conditionnementSolde(wallet))+"\n")
                        comInText.set(serBuffer+" possède "+ str(conditionnementSolde(wallet)) + "€")
                    else:
                        ser.write(("22\n").encode()) #BADGE DEJA ENREGISTRE
                        print("22\n")
                        stopWaitCard()
                        waitNewBadge=0
            #transactions += [serBuffer,wallet]
            serBuffer = ""
        else:
            serBuffer += c.decode()
    fenetre.after(10, readSerial)
    
def arron100(flo):
    flo=flo*100
    ent=int(flo)
    flo2=ent/100
    return(flo2)

def getCentieme(flo):
    flo2=flo*10
    ent2=int(flo2)
    return(10*(10*flo-ent2))
    
def pay(identifier, value, timestamp):
    global transac
    solde=getWallet(identifier)
    if solde == -1:
        ser.write(("13\n").encode()) #BADGE INCONNU
        print("13")
    else:
        if float(value)<=float(solde):
            updateWallet(identifier,arron100(float(solde)-float(value)))
            ser.write(("11\n").encode()) #TRANSACTION OK
            lName, fName = getName(identifier)
            transaction = timestamp+ "    "+ fName+" "+lName +" a payé "+ str(conditionnementSolde(float(value))) + "€"
            tr.set(transaction+"\n"+tr.get())
            transac.append(transaction)
            comInText.set(transaction)
            print("11")
        else:
            ser.write(("12\n").encode()) #SOLDE INSUFFISANT
            print("12")
 
def getWallet(recieved):
    usersFile = open(usersFilePath, "r+")
    wallet=0
    line=" "
    found=0  
    while(line!=""):
        line = usersFile.readline()
        line = line.strip('\n')
        lineArr = line.split(',')
        if(lineArr[0]==recieved):
            wallet=lineArr[1]
            found=1
            break
    if(found==1):
        return(wallet)
    else:
        return(-1)
    usersFile.close()
    
def getName(recieved):
    usersFile = open(usersFilePath, "r+")
    line=" "
    found=0  
    while(line!=""):
        line = usersFile.readline()
        line = line.strip('\n')
        lineArr = line.split(',')
        if(lineArr[0]==recieved):
            firstName=lineArr[3]
            lastName=lineArr[2]
            found=1
            break
    if(found==1):
        return(lastName,firstName)
    else:
        return(-1,-1)
    usersFile.close()
        
def updateWallet(identifier, newWallet):
    tmpUsersFile = open(tmpUsersFilePath, "w")
    usersFile = open(usersFilePath, "r")
    line=" "
    while(line!=""):
        line = usersFile.readline()
        lineN = line.strip('\n')
        lineArr = lineN.split(',')
        if(lineArr[0]==identifier):
            lastName,firstName=getName(identifier)
            tmpUsersFile.write(identifier+","+str(newWallet)+","+lastName+","+firstName)
        else:
            tmpUsersFile.write(line)
    usersFile.close()
    tmpUsersFile.close()
    os.remove(usersFilePath)
    os.rename(tmpUsersFilePath, usersFilePath)

def waitCardWindow():
    global waitNewBadge
    global win
    win = Toplevel()
    win.wm_attributes("-topmost", 1)
    win.title('Info ERIA')
    win.resizable(False, False)
    win.iconbitmap('img\carte.ico')
    message = "Passez le badge à enregistrer sur le lecteur"
    Label(win, text=message).pack()
    ser.write(("15\n").encode())
    waitNewBadge=1
    Button(win, text='Annuler', command=cancelWaitCard).pack()
            
def stopWaitCard():
    global waitNewBadge
    global win
    waitNewBadge=0
    usWindow.destroy()
    win.destroy()
    
def cancelWaitCard():
    ser.write(("20\n").encode())
    stopWaitCard()
    
def addUser(badgeID,amount,lastname,firstname):
    usersFile = open(usersFilePath, "a")
    usersFile.write("\n"+badgeID+","+amount+","+lastname+","+firstname)
    usersFile.close()
    stopWaitCard()
    
    
def addUserVar(amount,firstname,lastname):
    global userData
    if(amount!="" and firstname!="" and lastname!=""):
        userData=["",amount,lastname,firstname]
        waitCardWindow()
    else:
        errUserAdd()
    
def conditionnementSolde(solde):
    sol=float(solde)
    soldeCond= solde
    if(sol<10):
        soldeCond="0"+str(soldeCond)
    if(getCentieme(float(solde))==0):
        soldeCond=str(soldeCond)+"0"
    return(soldeCond)
    
def addUserWindow():
    global usWindow
    global userData
    global conn
    if conn==1:
        usWindow= Tk()
        usWindow.wm_attributes("-topmost", 1)
        usWindow.title("Ajouter un membre")
        usWindow.iconbitmap('img\carte.ico')
        usWindow.geometry("300x200")
        usWindow.resizable(False, False)
        Label(usWindow, text='Nom ').pack()
        userLastName = StringVar() 
        lastNameField = Entry(usWindow, textvariable=userLastName, width=20)
        lastNameField.pack()
        
        Label(usWindow, text='Prénom ').pack()
        userFirstName = StringVar() 
        firstNameField = Entry(usWindow, textvariable=userFirstName, width=20)
        firstNameField.pack()
        
        Label(usWindow, text='Montant enregistré ').pack()
        amountEntry = Combobox(usWindow, width=12, values=montants,state="readonly")
        amountEntry.pack()
        
        Button(usWindow, text="Ajouter l'utilisateur", command= lambda: addUserVar(amountEntry.get(),firstNameField.get(),lastNameField.get())).pack()
    else:
        alertNoConn()
    

#countsFile = open('counts.csv', "a")
ports=[]

montants=["5.0","10.0","15.0","20.0","25.0","30.0","50.0"]

for i in range(len(list(serial.tools.list_ports.comports()))):
    ports.append(list(serial.tools.list_ports.comports())[i][0])        
        
# FENETRE
fenetre= Tk()
fenetre.title("ERIA")
fenetre.iconbitmap('img\carte.ico')
fenetre.geometry("600x400")
fenetre.resizable(False, False)

#CADRE DE CONTENU
connecCadre = LabelFrame(fenetre, text="Connexion")
connecCadre.grid(sticky="n")
connecCadre.pack(fill="x",pady=0)


#MENU DEROULANT PORTS SERIE
Label(connecCadre, text='Port du lecteur ').pack(side=LEFT)
portSelect = Combobox(connecCadre, width=12, values=ports,state="readonly")
portSelect.pack(side=LEFT)

#BOUTON CONNEXION
btnText=StringVar()
btnText.set('Connexion')
Button(connecCadre, textvariable=btnText, command=connectPort).pack(side=LEFT)

#ETAT DE CONNEXION
comInfoText=StringVar()
comInfoText.set('Déconnecté')
comInfo = Label(connecCadre, textvariable=comInfoText, font=("Helvetica", 16)).pack()

#MESSAGE ATTENTION
warning = Label(fenetre, text="Attention : cliquez sur \"Déconnexion\" avant de déconnecter le lecteur ERIA.", foreground="red") #Si le lecteur se débranche inopinément, le rebrancher avant de cliquer sur \"Déconnexion\"
warning.pack()


#CADRE D'AJOUT DE MEMBRE OU SOLDE
userCadre = LabelFrame(fenetre, text="Ajout de membre ou de solde")
userCadre.pack(fill="x",pady=0)

Button(userCadre, text="Ajouter un membre", command= addUserWindow).pack()


        
#DONNEES PORT SERIE
serialFrame = LabelFrame(fenetre, text="Dernières transactions", height=200)
serialFrame.pack_propagate(False)
serialFrame.pack(fill="x",pady=0)
comInText=StringVar()
comInText.set('Aucune donnée')
comIn = Label(serialFrame, textvariable=comInText, background="grey", foreground="white").pack()

tr=StringVar()
Label(serialFrame, textvariable=tr).pack(side = LEFT)

#DONNEES BAS DE FENETRE
clock=StringVar()

Label(fenetre, textvariable=clock, font=('Helvetica',16)).pack(side=RIGHT)

#MENU
menubar = Menu(fenetre)

menu1 = Menu(menubar, tearoff=0)
menu1.add_command(label="Voir la base des utilisateurs", command=openUsersFile)
menu1.add_command(label="Voir le fichier de compte", command=openCountsFile)
menubar.add_cascade(label="Fichier", menu=menu1)

menu2 = Menu(menubar, tearoff=0)
menu2.add_command(label="Choix du port de communication", command=alert)
menubar.add_cascade(label="Paramètres", menu=menu2)

menu3 = Menu(menubar, tearoff=0)
menubar.add_command(label="À propos", command=about)

fenetre.config(menu=menubar)

fenetre.mainloop()