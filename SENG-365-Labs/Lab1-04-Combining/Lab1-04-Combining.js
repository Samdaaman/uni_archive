const http = require("http"), URL = require('url').URL;

http.createServer(function (request, response) {
    const parameters = new URL(request.url, 'http://localhost').searchParams;
    const items = [ 'milk', 'eggs', 'flour' ]
    response.writeHead(200, {'Content-Type' : 'text/plain'});
    const item_num = parameters.get('item_num');
    response.end('Here is your item: ' + item_num + ' : ' + items[item_num]);
}).listen(8081);

console.log('Server running at http://127.0.0.1:8081/')