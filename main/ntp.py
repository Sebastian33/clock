CENTURY = 3155673600
DAYS_IN_MONTH = [31,28,31,30,31,30,31,31,30,31,30,31]
LEAP_BEFORE_1900 = 460

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

    if leap!=0 and ((years%400==0) or (years%4==0 and years%100!=0)):
        leap-=1

    while days<leap:
        years-=1
        days+=365
        if (years%400==0) or (years%4==0 and years%100!=0):
            days+=1
        
    days=days-leap+1
    years+=years0

    month=0
    while days>DAYS_IN_MONTH[month]:
        days-=DAYS_IN_MONTH[month]
        if month==1 and ((years%400==0) or (years%4==0 and years%100!=0)):
            if days>1:
                days-=1
            else:
                days=29
                month=1
                break
        month+=1
        
    month+=1
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

    for m in range(months-1):
        days+=DAYS_IN_MONTH[m]

    leap=years//4 
    leap-=years//100
    leap+=years//400
    leap-=LEAP_BEFORE_1900

    if (years%400==0) or (years%4==0 and years%100!=0):
        leap-=1
        if months>2:
            days+=1

    years-=1900
    days+=years*365+leap-1
    seconds+=days*3600*24
    return seconds

def test():
    year=1900
    month=1
    day=1

    while year<2500:
        print(year, month, day)
        s=date2sec(year, month, day, 0, 0, 0)
        date=sec2date(s)
        if date[0]!=year or date[1]!=month or date[2]!=day or date[3]!=0 or date[4]!=0 or date[5]!=0:
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
