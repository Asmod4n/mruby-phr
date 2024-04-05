assert("Phr#parse_request") do
  phr = Phr.new
  buffer = "POST / HTTP/1.1\r\nHost: www.google.com\r\nContent-Length: 5\r\nConnection: close\r\n\r\nhallo"
  offset = phr.parse_request buffer
  assert_equal(79, offset)
  assert_equal('POST', phr.method)
  assert_equal('/', phr.path)
  assert_equal(1, phr.minor_version)
  assert_equal([["Host", "www.google.com"], ["Content-Length", "5"], ["Connection", "close"]], phr.headers)
  assert_equal('hallo', buffer[offset..-1])
end

assert("Phr#parse_response") do
  phr = Phr.new
  buffer = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\nConnection: close\r\n\r\nhallo"
  offset = phr.parse_response buffer
  assert_equal(57, offset)
  assert_equal(1, phr.minor_version)
  assert_equal(200, phr.status)
  assert_equal('OK', phr.msg)
  assert_equal([["Content-Length", "5"], ["Connection", "close"]], phr.headers)
  assert_equal('hallo', buffer[offset..-1])
end

assert("Phr#decode_chunked") do
  phr = Phr.new
  buffer = "b\r\nhello world\r\n0\r\n"
  phr.decode_chunked(buffer)
  assert_equal('hello world', buffer)
end
