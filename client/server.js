var net = require ('net') 

var server = net.createServer(function(c) { //'connection' listener
  console.log('server connected');
  c.setEncoding('utf8')    
  c.on('data', function (data) { 
    console.log(data)
    c.write("12", function () { 
			console.log(arguments)
		}); 
  })

  c.on('end', function() {
    console.log('server disconnected');
  });
});

server.listen(8124, function() { //'listening' listener
  console.log('server bound');
});
