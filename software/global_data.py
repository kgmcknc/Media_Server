
server_major_version = 1
server_minor_version = 0

import queue
import json
main_queue = queue.Queue()
main_queue_timeout = 4
heartbeat_queue = queue.Queue()
network_queue = queue.Queue()
network_queue_timeout = 4
media_queue = queue.Queue()
media_queue_timeout = 4

class instruction_class:
   is_local = 0
   is_global = 0
   global_done = 0
   global_id = 0
   src = 0
   dst = 0
   port = 0
   socket = 0
   command = 0
   data = 0

   def __init__(self, **instruction_info):
      for data_select in instruction_info:
         setattr(self, data_select, instruction_info[data_select])
   
   def data_to_json(self):
      return json.dumps(self.data)

   def copy(self):
      current_vars = self.dump_dict(self)
      return instruction_class(**current_vars)
   
   def dump_dict(self):
      data_dict = {}
      self_dir = dir(self)
      for item in self_dir:
         if(item.startswith('_') == False):
            new_item = getattr(self, item)
            if(callable(new_item) == False):
               data_dict[item] = new_item
      data_dict.pop("socket")
      return data_dict.copy()
   
   def load_dict(self, **input_dict):
      for data_select in input_dict:
         setattr(self, data_select, input_dict[data_select])

   def dump_global(self):
      data_dict = {"global_port":self.port, "global_command":self.command, "global_data":self.data}
      return data_dict

   def load_global(self, global_dict):
      self.port = global_dict["global_port"]
      self.command = global_dict["global_command"]
      self.data = global_dict["global_data"]

instruction_map = [
   "/heartbeat/reload_config",
   "/heartbeat/ip_changed",
   "/heartbeat/new_packet",
   "/heartbeat/device_connected",
   "/heartbeat/device_disconnected",

   "/network/reload_config",
   "/network/received_unknown_connection"

   "/system/shutdown_delay",

   "/media/set_display",
   "/media/play",
   "/media/pause",

   "/database/update_config",
   "/database/link_device",
   "/database/add_media_folder",
   "/database/rem_media_folder",
   "/database/index_media_folder",
   "/database/get_media_folders",
   "/database/get_media_data",
   "/database/add_media_link",
   "/database/rem_media_link",
   "/database/get_media_links",
   "/database/get_media_link",
   "/database/add_user",
   "/database/rem_user",
   "/database/get_users",
   "/database/get_user_data",
   "/database/set_user_data"
]