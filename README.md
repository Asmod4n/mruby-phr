# mruby-phr

Example
=======
```ruby
buffer = "HTTP/1.1 200 OK\r\nConnection: close\r\n\r\n"

phr = Phr.new
phr.parse_response buffer
puts phr.minor_version
puts phr.status
puts phr.msg
puts phr.headers
```

Please be aware: the header names are being modified and when the buffer goes out of Scope your App will crash.
