const net = require('net');
const client = net.createConnection(3000,"192.168.1.117", () => {
  //'connect' listener
  console.log('connected to server!');
  client.write('Hello TCP world!\r\n');
});
client.on('data', (data) => {
  console.log(data.toString());
  client.end();
});
client.on('end', () => {
  console.log('disconnected from server');
});