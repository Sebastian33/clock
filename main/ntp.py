CENTURY = 3155673600

def sec2date(s):
    if s>=CENTURY:
        leap=1
        years0=2000
        s-=CENTURY
    else:
        leap=0
        years0=1900
    
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

    while days<leap:
        leap-=days
        years-=1
        days=364
        if (years%400==0) or (years%4==0 and years%100!=0):
            days+=1

    days=days-leap+1
    years+=years0
    print(years, days, hours, minutes, seconds)

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
        if s>d:
            s-=d:
        else:
            break
        years+=1

    print(years, s, hours, minutes, seconds)
