
import pymongo

class server_device_class:
   name = "noname"
   id = 0
   ip_addr = 0
   linked = 0
   connected = 0
   def __init__(self, **device_info):
      if "name" in device_info:
         self.name = device_info["name"]
      if "id" in device_info:
         self.id = device_info["id"]
      if "ip_addr" in device_info:
         self.ip_addr = device_info["ip_addr"]
      if "linked" in device_info:
         self.linked = device_info["linked"]
      if "connected" in device_info:
         self.connected = device_info["connected"]
   
   def update_device_name():
      pass

   def update_device_ip_addr():
      pass

   def update_device_link_status():
      pass

   def update_device_connection_status():
      pass

   def update_device_id():
      pass

   def add_device_to_db():
      pass

   def rem_device_from_db():
      pass

   def update_device_in_db():
      pass
