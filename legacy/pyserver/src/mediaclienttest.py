import asyncio

class MediaClientProtocol(asyncio.DatagramProtocol):

  def __init__(self):
    pass
    
  def connection_made(self, transport):
    self.transport = transport;
    
  def connection_lost(self, exc):
    print("connection lost")
    
  def datagram_received(self, data, addr):
    print("Message from {0} {1}".format(addr, data))
    
  def error_received(self, exc):
    print("error received:", exc)
    
  def sayHello(self):
    print("sayHello()")
    self.transport.sendto(b'Hey di ho!')
    asyncio.get_event_loop().call_later(1, self.sayHello)
    
#
# MAIN
#

loop = asyncio.get_event_loop()

print("Start UDP client")
clientFuture = loop.create_datagram_endpoint(MediaClientProtocol, remote_addr = ("127.0.0.1", 5005))
transport, protocol = loop.run_until_complete(clientFuture)

print("Schedule some tasks...")
loop.call_soon(protocol.sayHello)

print("Run event loop")
try:
  loop.run_forever()
except:
  pass
finally:
  transport.close()
  loop.close()