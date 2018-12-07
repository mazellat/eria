# -*- coding: utf-8 -*-
"""
Created on Thu Apr 12 11:41:42 2018

@author: MAZELLA_SICSIC
"""


def getWallet(recieved):
    usersFile = open('users.csv', "r+")
    wallet=0
    line=" "
    found=0
    
    while(line!=""):
        line = usersFile.readline()
        lineArr = line.split(',')
        if(lineArr[0]==recieved):
            wallet=lineArr[1]
            found=1
            break
    
    if(found==1):
        return(wallet)
    else:
        print("Key ID not found")
    usersFile.close()
    
print(getWallet("A2392F736D"))