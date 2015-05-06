var serialport = require('serialport');
var _ = require('lodash');
var SerialPort = serialport.SerialPort;

// list serial ports:
serialport.list(function (err, ports) {
  ports.forEach(function(port) {
    console.log(port.comName, port.pnpId, port.manufacturer);
  });
});

var sp = new SerialPort("/dev/tty.uart-A1FF4A7AB7480E34", {
  baudrate: 9600
});

sp.on('open', function (error) {
  console.log('open', arguments);
});

sp.on('data', function(data) {
  console.log('data received: ' + data);
});
