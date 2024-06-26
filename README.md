# mruby-phr
A small http parser for mruby based on https://github.com/h2o/picohttpparser

Examples
=======
```ruby
buffer = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\nConnection: close\r\n\r\nhallo"

phr = Phr.new
offset = phr.parse_response buffer
puts phr.minor_version
puts phr.status
puts phr.msg
puts phr.headers
body = buffer[offset..-1]
puts body
phr.reset

buffer = "POST / HTTP/1.1\r\nHost: www.google.com\r\nContent-Length: 5\r\nConnection: close\r\n\r\nhallo"
offset = phr.parse_request buffer
puts phr.method
puts phr.path
puts phr.minor_version
puts phr.headers
body = buffer[offset..-1]
puts body
phr.reset

buffer = "b\r\nhello world\r\n0\r\n"
phr.decode_chunked(buffer)
puts buffer
phr.reset
```

Return values
-------------
Phr has three response values
:parser_error when a request was malformed
:incomplete when the supplied buffer doesnt hold a complete request
Or a Integer value describing where the body starts.