import serial
import sys
import numpy as np
## MPU-6050 Constants
# don't forget to normalize to 0 mean and 1 std. before scaling.
accelSens=16384. #LSB/(g)
gyroSens=131. #LSB/(deg/sec)
def rawRead(ser,filename,verbose=0):
    datas=[]
    while True:
        try:
            data=ser.read(1)
            datas.append(data)
            if verbose:
                sys.stdout.write( data )
        except KeyboardInterrupt:
            ser.close()
            print "Done!"
            break
    with open(filename,'wb') as fp:
        try:
            for data in datas:
                fp.write(data)
            print "Wrote to {}".format(filename)
        finally:
            fp.close()

def parseRead(ser,filename,verbose=0):
    datas=[[]]
    i=0
    while True:
        try:
            data=ser.read(1)
            if not data:
                continue
            data=ord(data)
            if len(datas[-1])==6:
                printline=""
                for n in datas[-1]:
                    printline+="{:8}".format(n)
                if verbose:
                    sys.stdout.write(printline)
                    sys.stdout.write("\n")
                datas.append([])
                continue
            if i==0:
                datas[-1].append(0)
            datas[-1][-1]+=data<<i
            i=(i+1)%4
        except KeyboardInterrupt:
            break
        ser.close()
        print "Done!"
        with open(filename,'w') as fp:
            try:
                for line in datas:
                    fp.write("{}\n".format(line))
                print "Wrote to {}".format(filename)
            finally:
                fp.close()

def readCSV(filename):
    fp=open(filename)
    lines=[]
    longest=0
    for line in fp:
        lines.append(line)
        longest=max(longest,len(line.split(' ')))
    data=[]
    for line in lines:
        line=line.strip('\n')
        if not line:
            continue
        line=line.split(' ')
        dataline=[]
        for num in line:
            try:
                dataline.append(int(num))
            except:
                dataline.append(0)
        if longest>len(dataline):
            dataline.extend([0 for x in xrange(longest-len(dataline))])
        data.append(dataline)
    fp.close()
    return data

def complementEstimates(rawgyro,rawaccel,coeffs=[0.98,0.02],dt=1e-3):
    gyro = rawgyro/gyroSens
    accel = rawaccel/accelSens
    coeffs=array(coeffs)/sum(coeffs)
    estimates=[0]
    for x1,x2 in zip(gyro,accel):
        estimates.append(coeffs[0]*(estimates[-1]+x1*dt)+coeffs[1]*x2)
    return estimates

if __name__=="__main__":
    import argparse
    parser=argparse.ArgumentParser()
    parser.add_argument('filename',type=str)
    parser.add_argument('-d','--device',type=int,default=9)
    parser.add_argument('-r','--rate',type=int,default=9600)
    parser.add_argument('-v','--verbose',type=int,default=0)
    parsedArgs=parser.parse_args()
    config = {
            'device':parsedArgs.device-1,
            'rate':parsedArgs.rate,
            }
    ser = serial.Serial(config['device'], config['rate'], timeout=0)
    filename=parsedArgs.filename
    rawRead(ser,filename,verbose=parsedArgs.verbose)
    ser.close()

