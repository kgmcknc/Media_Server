
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