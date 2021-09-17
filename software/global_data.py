
server_major_version = 1
server_minor_version = 0

import queue
main_queue = queue.Queue()
main_queue_timeout = 4
heartbeat_queue = queue.Queue()
network_queue = queue.Queue()
network_queue_timeout = 4

class instruction_class:
   src = 0
   dst = 0
   group = 0
   task = 0
   data = 0

   def __init__(self, **instruction_info):
      for data_select in instruction_info:
         setattr(self, data_select, instruction_info[key])

instruction_map = [
   [
      "system_tasks",
      "shutdown_delay"
   ],
   [
      "device_tasks",
      "update_device"
   ],
   [
      "heartbeat_tasks",
      "reload_config",
      "ip_changed",
      "new_packet"
   ],
   [ # Network Tasks
      "add_media_folder"
   ],
   [ # Media Tasks
      "nothing"
   ],
   [ # Alarm Taks
      "nothing"
   ]
]