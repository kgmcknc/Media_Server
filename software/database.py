
import pymongo
import global_data
import networking
import random
import datetime
import devices

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

   server_devices = server_db["devices"]
   server_devices.insert_one({"name":"none", "id":"none", "ip":"none"})
   
   media_folder = server_db["media"]
   media_folder_list = media_folder["media_folder_list"]
   media_folder_list.insert_one({"name":"none", "path":"none"})

def init_config():
   random.seed()
   server_config = server_db["config"]
   this_ip = networking.get_my_ip();
   new_num = random.randint(1,999)
   current_date = datetime.datetime.now()
   id_string = str(new_num) + str(current_date.month) + str(current_date.day) + str(current_date.year)
   device_id = int(id_string)
   config_info = devices.server_device_class()
   device_init = {
      "_id": device_id,
      "name": "devicename",
      "major_version": global_data.server_major_version,
      "minor_version": global_data.server_minor_version,
      "address": this_ip,
      "linked": 0,
      "connected": 0,
      "device_id": device_id
      }
   config_info.update_device_info(**device_init)
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
   return config_data
   