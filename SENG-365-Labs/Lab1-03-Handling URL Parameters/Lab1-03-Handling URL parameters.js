const http = require("http"), URL = require('url').URL;

http.createServer(function (request, response) {
    const parameters = new URL(request.url, 'http://localhost').searchParams.toString();
    response.writeHead(200, {'Content-Type' : 'text/plain'});
    response.end('Here is your data: ' + parameters);
}).listen(8081);

console.log('Server running at http://127.0.0.1:8081/')