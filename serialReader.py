import serial
import sys
import numpy as np
import binascii,struct
import angleGui
def rawRead(ser,filename=None,grouping=(4,8)):
    datas=[]
    bytesReceived=0
    while True:
        try:
            data=np.array(ser.read(1))
            data=struct.unpack('B',data)[0]
            if not data:
                continue

            sys.stdout.write("%02X"%data)
            bytesReceived+=1
            if np.mod(bytesReceived,grouping[0]*grouping[1])==0:
                sys.stdout.write('\n')
            elif np.mod(bytesReceived,grouping[0])==0:
                sys.stdout.write(' ')
            if filename:
                datas.append(data)
        except KeyboardInterrupt:
            ser.close()
            sys.stdout.write('\n')
            sys.stdout.write('Done!\n')
            break
    if not filename:
        return
    with open(filename,'w') as fp:
        try:
            for data in datas:
                fp.write(data)
            print "Wrote to {}".format(filename)
        finally:
            fp.close()

def parseRead(ser,filename,formats):
    """
    ser - serial stream
    filename - output csv file
    formats - ex: ['h','h','h','d'] is three shorts and a double per row

    """
    rxbuffer=[]
    dataRows=[]
    collectedData=[]
    currFormat=0
    format_sizes=map(struct.calcsize,formats)
    print formats,format_sizes
    while True:
        try:
            if len(rxbuffer)==format_sizes[currFormat]:
                bytestring=struct.pack('{}B'.format(format_sizes[currFormat]),*rxbuffer)
                parsed=struct.unpack(formats[currFormat],bytestring)[0]
                collectedData.append(parsed)
                sys.stdout.write(str(parsed))
                sys.stdout.write(' ')
                rxbuffer=[]
                currFormat=(currFormat+1)%len(formats)
                continue
            if len(collectedData)==len(formats):
                dataRows.append(collectedData)
                collectedData=[]
                rxbuffer=[]
                sys.stdout.write('\n')
            data=ser.read(1)
            if not data:
                continue
            data=struct.unpack('B',data)[0]
            rxbuffer.append(data)
        except KeyboardInterrupt:
            dataRows.append(collectedData)
            collectedData=[]
            sys.stdout.write('\n')
            ser.close()
            break
    print "Done!"
    if not filename:
        return
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

def complementEstimates(rawgyro,rawaccel,coeffs=[0.98,0.02],dt=1e-6):
    gyro = rawgyro
    accel = rawaccel
    coeffs=np.array(coeffs)/sum(coeffs)
    estimates=[0]
    for x1,x2 in zip(gyro,accel):
        estimates.append(coeffs[0]*(estimates[-1]+x1*dt)+coeffs[1]*x2)
    return estimates

if __name__=="__main__":
    import argparse
    parser=argparse.ArgumentParser()
    parser.add_argument('formats',type=str)
    parser.add_argument('-o','--outfile',type=str,default=None)
    parser.add_argument('-d','--device',type=int,default=10)
    parser.add_argument('-r','--rate',type=int,default=9600)
    parsedArgs=parser.parse_args()
    config = {
            'device':parsedArgs.device-1,
            'rate':parsedArgs.rate,
            }
    ser = serial.Serial(config['device'], config['rate'], timeout=1)
    filename=parsedArgs.outfile

    formats=parsedArgs.formats
    if formats=='raw':
        rawRead(ser,filename)
    elif formats=="gui":
        angleGui.initializeGui(ser)
    else:
        formats=formats.split(',')
        parseRead(ser,filename,formats)
    ser.close()

