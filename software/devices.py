
import pymongo
import json

class server_device_class:
   name = "noname"
   id_num = 0
   ip_addr = 0
   linked = 0
   connected = 0
   db_id = 0

   def __init__(self, **device_info):
      for key in device_info:
         setattr(self, key, device_info[key])

   def update_device_info(self, **device_info):
      for key in device_info:
         if((key == 'id_num') or (key == 'db_id')):
            printf("Can't modify id or db_id number")
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

