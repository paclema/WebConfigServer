const express = require('express')
const bodyParser = require('body-parser')
const cors = require('cors')
const WebSocket = require('ws');
const fileUpload = require('express-fileupload');

const PORT = 3000;
const PORT_WEBSOCKETS = 81;



const app = express();

app.use(bodyParser.json());
app.use(cors());
app.use(express.urlencoded({
  extended: true
}))
// enable files upload
app.use(fileUpload({
  createParentPath: true
}));



app.get('/', function(req, res){
  res.send('Hello from express server')
});

app.post('/enroll', function(req, res){
  console.log(req.body);
  res.status(200).send({"message": "Data received"});
});

app.post('/save_config', function(req, res){
  console.log(req.body);
  // Emulate delay of 2s for response:
  setTimeout(() => {
    res.status(200).send({"message": "Configurations saved"}); 
  }, 2000);
  
});

app.post('/restart', function(req, res){
  console.log(req.body.restart);
  if (req.body.restart){
    res.status(200).send({"message": "Restarting device... "});
  }  else {
    res.status(400).send({"message": "Invalid request. Add restart parameter to the POST request" });
    }
});

app.post('/restore_config', function(req, res){
  console.log(req.body.filename);

  res.status(200).send({"message": "Configuration " + req.body.filename +" restored"});
});

app.post('/gpio', function(req, res){
  console.log("id: ", req.body.id," val: ", req.body.val);
  if (req.body.id && req.body.val){
    res.status(200).send({"message": "GPIO " + req.body.id + " changed to " + req.body.val});
  }  else {
    res.status(400).send({"message": "Invalid request. Add id and val parameters to the POST request" });
    }
});

app.listen(PORT, function(req, res){
  console.log("Server running on localhost:" + PORT);
});




// Websockets:

const wss = new WebSocket.Server({ port: PORT_WEBSOCKETS });
console.log('Websockets running on localhost:' + PORT_WEBSOCKETS);


wss.on('connection', ws => {
  console.log('Client connected');

  // setInterval(function (){
    //   // const data = '{"hello": "world"}'
    //   const data = '{"data": [{"TradeId": "1","TradeDate": "11/02/2016","BuySell": "Sell","Notional": "50000000","Coupon": "500","Currency": "EUR","Ticker": "LINDE","ShortName": "Linde AG","MaturityDate": "20/03/2023","Sector": "Basic Materials","Trader": "Yael Rich","Status": "Pending"}]}'
    //   console.log('Sending data %s', data);
    //   wss.emit('message', data)
    // }, 500);

  const id = setInterval(function () {
  // ws.send(JSON.stringify(process.memoryUsage()), function () {
  //     //
  //     // Ignore errors.
  //     //
  //   });
  ws.send('{"heap_free":' + Math.floor(Math.random() * (22000 - 18000) + 18000)+
        ', "heap_free2":' + Math.floor(Math.random() * (22000 - 18000) + 18000)+
        ', "heap_free3":' + Math.floor(Math.random() * (22000 - 18000) + 18000)+
        ', "heap_free4":' + Math.floor(Math.random() * (22000 - 18000) + 18000)+
        ', "heap_free5":' + Math.floor(Math.random() * (22000 - 18000) + 18000)+
        ', "heap_free6":' + Math.floor(Math.random() * (22000 - 18000) + 18000) + '}');
  // ws.send('{"heap_free2":' + Math.floor(Math.random() * (22000 - 18000) + 18000) + ' }');

  }, 100);

  ws.isAlive = true;

  // Socket pong:
  ws.on('pong', () => {
      ws.isAlive = true;
  });

  // New Socket message:
  ws.on('message', message => {
    //log the received message and send it back to the client
    console.log('received: %s', message);
    // ws.send(`Hello, you sent -> ${message}`);

    // console.log(typeof(message));
    var messageObject;
    try {
      messageObject = JSON.parse(message);
    } catch (e) {
      return console.error(e);
    }
    console.log(messageObject.hasOwnProperty('broadcast'));

    // To send to all the listeners if a client send: 'broadcast:message'
    // const broadcastRegex = /^broadcast\:/;
    // if (broadcastRegex.test(message)) {
      // message = message.replace(broadcastRegex, '');

    if (messageObject.hasOwnProperty('broadcast')) {

      //send back the message to the other clients
      wss.clients.forEach(client => {
        if (client != ws) {
          // client.send(`Hello, broadcast message -> ${message}`);
          ws.send(message);

        }
      });

    } else {
      // ws.send(`Hello, you sent -> ${message}`);
        ws.send(message);
    }
  });

  // Error Socket:
  ws.on('error', error => {
    console.log('received error: %s', error);
    ws.send(`There was an error -> ${error}`);
  });

  // Socket closed:
  ws.on('close', ws=> {
    console.log('Socket closed: %s', ws);
    clearInterval(id);
  });

});


// Handle broken WebSocket connections:
setInterval(() => {
    wss.clients.forEach( ws => {

        if (!ws.isAlive) return ws.terminate();

        ws.isAlive = false;
        ws.ping(null, false, true);
    });
}, 10000);



// File uploads:
app.post('/uploadFile', async (req, res) => {

  try {
    console.log(req.files);
      if(!req.files.ca_file) {
        res.status(500).send({
            status: false,
            message: 'No file uploaded'
        });
    } else {
        //Use the name of the input field (i.e. "ca_file") to retrieve the uploaded file
        let ca_file = req.files.ca_file;
        // console.log(ca_file);
        
        //Use the mv() method to place the file in upload directory (i.e. "uploads")
        ca_file.mv('./uploads/' + ca_file.name);

        //send response
        res.send({
            status: true,
            message: 'File is uploaded',
            data: {
                name: ca_file.name,
                mimetype: ca_file.mimetype,
                size: ca_file.size
            }
        });
    }
  } catch (err) {
      console.log(req);
      res.status(500).send(err);
  }
});