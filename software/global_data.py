
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
   src = 0
   dst = 0
   socket = 0
   command = 0
   data = 0

   def __init__(self, **instruction_info):
      for data_select in instruction_info:
         setattr(self, data_select, instruction_info[key])
   
   def data_to_json(self):
      return json.dumps(self.data)

instruction_map = [
   "/heartbeat/reload_config",
   "/heartbeat/ip_changed",
   "/heartbeat/new_packet",

   "/network/reload_config",

   "/system/shutdown_delay",

   "/media/play",
   "/media/pause",

   "/local/update_config",
   "/local/link_device",
   "/local/add_media_folder",
   "/local/rem_media_folder",
   "/local/index_media_folder",
   "/local/get_media_folders",
   "/local/get_media_data",
   "/local/add_media_link",
   "/local/rem_media_link",
   "/local/get_media_links",
   "/local/get_media_link",
   "/local/add_user",
   "/local/rem_user",
   "/local/get_users",
   "/local/get_user_data",
   "/local/set_user_data"
]