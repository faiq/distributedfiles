var net = require ('net') 

var server = net.createServer(function(c) { //'connection' listener
  console.log('server connected')
  c.on('data', function (data) { 
    console.log(data)
    c.write("11", function () { 
			console.log(arguments)
		})
  })
})

server.listen(8124, function() { //'listening' listener
  console.log('server bound')
})
