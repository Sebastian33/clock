import socket
import time

CENTURY = 3155673600
DAYS_IN_MONTH = [31,28,31,30,31,30,31,31,30,31,30,31]

def sec2date(s):
    if s>=CENTURY:
        leap=1
        years0=100
        s-=CENTURY
    else:
        leap=0
        years0=0
    
    seconds=s%60
    s=s//60
    minutes=s%60
    s=s//60
    hours=s%24
    s=s//24
    
    days=s%365
    years=s//365
    
    leap+=years//4
    leap-=years//100
    leap+=years//400

    if leap!=0 and ((years%400==0) or (years%4==0 and years%100!=0)):
        leap-=1

    while days<leap:
        years-=1
        days+=365
        if (years%400==0) or (years%4==0 and years%100!=0):
            days+=1
        
    days=days-leap+1

    month=0
    while days>DAYS_IN_MONTH[month]:
        days-=DAYS_IN_MONTH[month]
        if month==1 and (years0+years>0) and ((years%400==0) or (years%4==0 and years%100!=0)):
            if days>1:
                days-=1
            else:
                days=29
                month=1
                break
        month+=1

    years+=years0
    return (years, month, days, hours, minutes, seconds)

def sec2date2(s):
    years=1900
    
    seconds=s%60
    s=s//60
    minutes=s%60
    s=s//60
    hours=s%24
    s=s//24
    
    while True:
        d=365
        if (years%400==0) or (years%4==0 and years%100!=0):
            d+=1
        if s>=d:
            s-=d
        else:
            break
        years+=1

    s+=1
    #print(years, s, hours, minutes, seconds)
    return (years, s, hours, minutes, seconds)

def date2sec(years, months, days, hours, minutes, seconds):
    seconds+=60*minutes+hours*3600

    for m in range(months):
        days+=DAYS_IN_MONTH[m]

    if years<100:
        leap=years//4 
        leap-=years//100
        leap+=years//400

        if years!=0 and ((years%400==0) or (years%4==0 and years%100!=0)):
            if months<=1:
                leap-=1

    else:
        years-=100
        days+=36524
        leap=years//4+1 
        leap-=years//100
        leap+=years//400

        if (years%400==0) or (years%4==0 and years%100!=0):
            if months<=1:
                leap-=1

    days+=years*365+leap-1
    seconds+=days*3600*24
    return seconds

def test():
    year=1900
    month=1
    day=1

    while year<4000:
        print(year, month, day)
        s=date2sec(year-1900, month-1, day, 0, 0, 0)
        date=sec2date(s)
        if date[0]!=year-1900 or date[1]!=month-1 or date[2]!=day or date[3]!=0 or date[4]!=0 or date[5]!=0:
            print("nope")
            return

        day+=1
        if month==2 and day==29 and ((year%400==0) or (year%4==0 and year%100!=0)):
            continue
        elif DAYS_IN_MONTH[month-1]<day:
            day=1
            month+=1

        if month>12:
            month=1
            year+=1
    print("done")

def ntpRequest():
    client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    client.settimeout(5.0)
    #addr=('time.google.com',123)
    addr=('time.windows.com',123)

    li=0
    vn=4
    mode=3
    message=((li<<5)|(vn<<3)|mode).to_bytes(1,'big')
    message+=(16).to_bytes(1,'big')
    message+=(10).to_bytes(1,'big')
    message+=(0).to_bytes(1,'big')
    message+=bytes(8)
    message+=b'REFR'
    message+=date2sec(2024, 4, 25, 11, 37, 2).to_bytes(4, 'big')+bytes(4)
    dt=time.localtime()
    print(dt)
    message+=date2sec(dt.tm_year, dt.tm_mon, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec).to_bytes(4, 'big')+bytes(4)
    message+=bytes(16)

    #print(message)
    
    client.sendto(message, addr)
    rsp = client.recvfrom(1024)
    rsp=rsp[0]
    print(rsp)
    #receive ts
    #s=rsp[35]+(rsp[34]<<8)+(rsp[33]<<16)+(rsp[32]<<24)
    #print(sec2date(s))
    #transmit ts
    s=rsp[43]+(rsp[42]<<8)+(rsp[41]<<16)+(rsp[40]<<24)
    print(sec2date(s))
