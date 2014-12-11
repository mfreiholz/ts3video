import asyncio
import socket

class MediaServerProtocol(asyncio.DatagramProtocol):
  """Handles incoming video- and audio frames
  
  Receives media information from one client and forwards it to one or
  more other connected clients.
  """

  def __init__(self):
    self.receivers = {}
    self.receivers["senderid"] = [("receiver-address", 5006), ("receiver-address",5006)]
  
  def connection_made(self, transport):
    self.transport = transport
    
  def error_received(exc):
    print("error received" % exc)
    
  def datagram_received(self, data, addr):
    print("Message from {0} {1}".format(addr, data))
    # Parse and modify datagram.
    # ...
    
    # Echo datagram.
    self.transport.sendto(data, addr)
    
    # Forward datagram to sibling clients.
    senderid = ""
    try:
      receivers = self.receivers[senderid]
      for rec in receivers:
        self.transport.sendto(data, rec)
    except KeyError:
      pass
    
#
# Main
#
    
loop = asyncio.get_event_loop()

print("Start UDP media server")
mediaServer = loop.create_datagram_endpoint(MediaServerProtocol, local_addr=("127.0.0.1", 5005))
transport, protocol = loop.run_until_complete(mediaServer)

print("Register test callbacks")
#loop.call_later(5, loop.stop)

print("Start event loop")
try:
  loop.run_forever()
except KeyboardInterrupt:
  pass
finally:
  transport.close()
  loop.close()
