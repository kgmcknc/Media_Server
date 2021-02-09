
import pymongo
import global_data
import networking
import random
import datetime
import devices
import json

server_db = 0

def exists():
   db_client = pymongo.MongoClient("mongodb://localhost:27017/")
   try:
      dblist = db_client.list_database_names()
   except:
      dblist = db_client.database_names()
   if ("server_db" in dblist):
      return 1
   else:
      return 0

def open_server_db():
   global server_db
   db_client = pymongo.MongoClient("mongodb://localhost:27017/")
   server_db = db_client["server_db"]

def init_server_db():
   global server_db
   print("Initializing Device and Server Database")
   db_client = pymongo.MongoClient("mongodb://localhost:27017/")
   server_db = db_client["server_db"]
   init_config()
   
   update_features()
   
   media_folder = server_db["media"]
   media_folder_list = media_folder["media_folder_list"]
   media_folder_list.insert_one({"name":"none", "path":"none"})

def init_config():
   random.seed()
   server_config = server_db["config"]
   this_ip = networking.get_my_ip();
   new_num = random.randint(1,999)
   current_date = datetime.datetime.now()
   day_string = str(current_date.day)
   month_string = str(current_date.month)
   id_string = str(new_num) + day_string.zfill(2) + month_string.zfill(2) + str(current_date.year)
   device_id = int(id_string)
   device_init = {
      "_id": device_id,
      "name": "devicename",
      "major_version": global_data.server_major_version,
      "minor_version": global_data.server_minor_version,
      "ip_addr": this_ip,
      "hb_period": 60,
      "port": 50000,
      "linked_devices": [],
      "connected": 0,
      "device_id": device_id,
      "is_server": 1
      }
   config_info = devices.server_device_class(**device_init)
   server_config.insert_one(vars(config_info))

def update_features():
   server_features = server_db["features"]
   feature_list = {"media_player":"false", "door_sensors":"false", "alarm":"false"}
   server_features.insert_one(feature_list)

def add_media_folder():
   pass

def rem_media_folder():
   pass

def index_media_folder():
   pass

def get_device_config():
   device_config = server_db["config"]
   config_data = device_config.find_one()
   config_data = devices.server_device_class(**config_data)
   return config_data

def update_db_device_config(config_data):
   config_db = server_db["config"]
   config_query = {"device_id": config_data.device_id}
   config_vars = vars(config_data)
   config_data_copy = config_vars.copy()
   config_data_copy.pop("_id")
   config_data_copy.pop("device_id")
   new_config = {"$set": config_data_copy}
   config_db.find_one_and_update(config_query, new_config)

def update_db_device_in_list(device_data):
   devices = server_db["devices"]
   found = devices.find_one()
   if(found == None):
      devices.insert_one(vars(device_data))
   else:
      device_query = {"device_id": device_data.device_id}
      device_vars = vars(device_data)
      device_vars_copy = device_vars.copy()
      device_vars_copy.pop("_id")
      device_vars_copy.pop("device_id")
      device_update = {"$set": device_vars_copy}
      updated = devices.find_one_and_update(device_query, device_update)

def remove_unlinked_db_devices(this_id):
   devices = server_db["devices"]
   device_query = {}
   db_devices = devices.find(device_query)
   db_device_list = list(db_devices)
   for dev in db_device_list:
      link_list = dev["linked_devices"]
      for index in link_list:
         if(index == this_id):
            break
      else:
         # remove this device
         device_query = {"device_id": dev["device_id"]}
         devices.delete_one(device_query)

def get_db_devices():
   db_devices = server_db["devices"]
   db_device_list = db_devices.find()
   db_list = list(db_device_list)
   new_list = []
   for index in db_list:
      new_device = devices.server_device_class(**index)
      new_list.append(new_device)
   return new_list

def get_filtered_db_devices(**filters):
   db_devices = server_db["devices"]
   if(len(filters) > 0):
      device_query = filters
   else:
      device_query = {}
   db_device_list = db_devices.find(device_query)
   db_list = list(db_device_list)
   new_list = []
   for index in db_list:
      new_device = devices.server_device_class(**index)
      new_list.append(new_device)
   return new_list

def add_linked_device(device_id):
   device_config = server_db["config"]
   config_data = device_config.find_one()
   config = devices.server_device_class(**config_data)
   config.linked_devices.append(device_id)
   update_db_device_config(config)

def set_db_device_fields(device_id, updates):
   pass
