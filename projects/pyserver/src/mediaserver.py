import socket


class AsynMediaServer(asyncore.dispatcher):

  def __init__(self, host, port):
    asyncore.dispatcher.__init__(self)
    self.create_socket(socket.AF_INET, socket.SOCK_DGRAM)
    self.bind((host, port))
    
  def handle_connect(self):
    print "handle connect"
    
  def handle_close(self):
    print "handle close"
    
  def handle_expt(self):
    print "handle expt"
    asyncore.dispatcher.handle_expt(self)
    
  #def handle_error(self):
  #  print "handle error"
    
  def handle_accept(self):
    print "handle accept"
    pair = accept()
    if pair is not None:
      sock, addr = pair
      print "Incoming donnection from %s" % repr(addr)

  def handle_read(self):
    data, addr = self.recvfrom(8192);
    print "handle read {1} {0}".format(data, addr)
    
  def handle_write(self):
    print "handle write"

#######################################################################
# Main
#######################################################################

server = AsynMediaServer("", 5005)
asyncore.loop()