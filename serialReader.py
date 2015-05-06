import serial
import sys
import numpy as np
import binascii,struct
## MPU-6050 Constants
# don't forget to normalize to 0 mean and 1 std. before scaling.
accelSens=16384. #LSB/(g)
gyroSens=131. #LSB/(deg/sec)
def rawRead(ser,filename,verbose=1):
    datas=[]
    while True:
        try:
            data=ser.read(1)
            datas.append(data)
            if verbose:
                sys.stdout.write(data)
        except KeyboardInterrupt:
            ser.close()
            print "Done!"
            break
    with open(filename,'w') as fp:
        try:
            for data in datas:
                fp.write(data)
            print "Wrote to {}".format(filename)
        finally:
            fp.close()

def parseRead(ser,filename):
    rxbuffer=[]
    dataRows=[]
    collectedData=[]
    while True:
        try:
            if len(rxbuffer)==2:
                print rxbuffer
                parsed=struct.unpack('h',struct.pack('2B',*rxbuffer))[0]
                collectedData.append(parsed)
                sys.stdout.write(str(parsed))
                rxbuffer=[]
                continue
            data=ser.read(1)
            if data==0x0A:
                dataRows.append(collectedData)
                collectedData=[]
                sys.stdout.write('\n')
                continue
            elif data==0x20:
                sys.stdout.write(' ')
                continue
            elif not data:
                continue
            rxbuffer.append(ord(data))
        except KeyboardInterrupt:
            dataRows.append(collectedData)
            collectedData=[]
            sys.stdout.write('\n')
            ser.close()
            break
    print "Done!"
    with open(filename,'wb') as fp:
        for line in dataRows:
            print line
            i=0
            for num in line:
                i+=1
                fp.write(str(num))
                if i!=len(line):
                    fp.write(' ')
            fp.write('\n');
        print "Wrote to {}".format(filename)
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
                dataline.append(num)
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
    parseRead(ser,filename)
    ser.close()

