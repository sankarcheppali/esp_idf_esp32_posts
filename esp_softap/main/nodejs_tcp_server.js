const net = require('net');
const server = net.createServer((socket) => {
  console.log('\n\n\---client connected---');
  socket.on('data',(data)=>{
      console.log("Msg from client :"+data.toString());
  });
  socket.end('Hello TCP Client\n'); // writes the given data and sends FIN packet
}).on('error', (err) => {
  console.log("err:"+err);
  throw err;
});


server.listen({
  host: '0.0.0.0',
  port: 3010,
  exclusive: true
},() => {
  console.log('TCP server started');
});