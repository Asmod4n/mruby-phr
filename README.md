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

Please be aware: the buffer is being modified, meaning, headers will be downcase, and when the buffer goes out of Scope your App will crash because of a use after free error.
Picohttpparser only holds references to the buffer it is passed and when that buffer gets garbage collected while the phr Object is still alive bad things happen.
