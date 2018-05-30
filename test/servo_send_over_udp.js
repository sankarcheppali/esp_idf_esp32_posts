
const dgram = require('dgram');
var postns=require('../dudutest')
var dt=postns.mouthCues;
const udp = dgram.createSocket('udp4');
var srvoMap={"A":0,
"X":0,
"G":0,
"B":30,
"F":30,
"C":60,
"E":60,
"H":60,
"D":90}
function sendOverTcp(data, i) {
    let dtc = data
    let index = i
    return () => {
        //console.log("Code " + dtc[index].value + "," + Date.now())
        msg = srvoMap[dtc[index].value] + "\n" ;
        if (udp) udp.send(msg, 3020, "192.168.1.106")
        console.log(msg)
        if (dtc.length > (index + 1)) {
            timeout = (dtc[index + 1].start - dtc[index].start) * 1000
                //console.log("Timeout " + timeout)
            setTimeout(sendOverTcp(dtc, index + 1), timeout)
        }
    }
}

sendOverTcp(dt, 0)()
