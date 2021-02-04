
import pymongo
import json

class server_device_class:
   _id = 0
   device_id = 0
   name = "noname"
   ip_addr = 0
   port = 0
   linked = 0
   connected = 0
   hb_period = 0

   def __init__(self, **device_info):
      for key in device_info:
         setattr(self, key, device_info[key])

   def update_device_info(self, **device_info):
      #TODO make this a try with except
      for key in device_info:
         if((key == 'device_id') or (key == '_id')):
            print("Can't modify id or _id number")
         else:
            setattr(self, key, device_info[key])
            
   def update_device_name(self):
      pass

   def update_device_ip_addr(self):
      pass

   def update_device_link_status(self):
      pass

   def update_device_connection_status(self):
      pass

   def update_device_id(self):
      pass

   def add_device_to_db(self):
      pass

   def rem_device_from_db(self):
      pass

   def update_device_in_db(self):
      pass
   
   def load_device_from_db(self):
      pass

   def to_json(self):
      return json.dumps(vars(self))

