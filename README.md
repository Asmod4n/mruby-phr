# mruby-phr

Example
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
```

Please be aware: the header names are being modified and when the buffer goes out of Scope your App will crash.
