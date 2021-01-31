
import threading

class server_thread_class:
   def __init__(self, thread_name = "thread"):
      self.var = 0
      self.event = threading.Event()
      self.name = thread_name

   def start(self, function, *arg_list):
      if(len(arg_list) > 0):
         self.var = threading.Thread(target=function, args=(self,arg_list))
      else:
         self.var = threading.Thread(target=function, args=(self,))
      self.var.start()

   def is_active(self):
      if(self.var != 0):
         ret_val = self.var.is_alive() and (not self.event.is_set())
      else:
         ret_val = 0
      return (ret_val)

   def stop_thread(self):
      if(self.var.is_alive()): # verify thread is actually active
         self.kill_thread() # send signal for thread to abort
         self.var.join() # wait for thread termination
         self.event.clear()

   def kill_thread(self):
      print('Stopping Thread: ', self.name)
      self.event.set()

   def pause(self, delay_val):
      self.event.wait(delay_val)



