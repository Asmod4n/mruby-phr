phr = Phr.new
assert("Phr#parse_request") do
  buffer = "POST / HTTP/1.1\r\nHost: www.google.com\r\nContent-Length: 5\r\nConnection: close\r\n\r\nhallo"
  offset = phr.parse_request buffer
  assert_equal(79, offset)
  assert_equal('POST', phr.method)
  assert_equal('/', phr.path)
  assert_equal(1, phr.minor_version)
  assert_equal([["host", "www.google.com"], ["content-length", "5"], ["connection", "close"]], phr.headers)
  assert_equal('hallo', buffer[offset..-1])
  phr.reset
end

assert("Phr#parse_response") do
  buffer = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\nConnection: close\r\n\r\nhallo"
  offset = phr.parse_response buffer
  assert_equal(57, offset)
  assert_equal(1, phr.minor_version)
  assert_equal(200, phr.status)
  assert_equal('OK', phr.msg)
  assert_equal([["content-length", "5"], ["connection", "close"]], phr.headers)
  assert_equal('hallo', buffer[offset..-1])
  phr.reset
end

assert("Phr#decode_chunked") do
  buffer = "b\r\nhello world\r\n0\r\n"
  phr.decode_chunked(buffer)
  assert_equal('hello world', buffer)
end
