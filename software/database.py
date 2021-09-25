
import pymongo
import global_data
import networking
import random
import datetime
import devices
import json

from pathlib import Path

server_db = 0

file_type_list = [
   ".M2V", ".M4V", ".MPEG1", ".MPEG2", ".MTS", ".AAC", ".DIVX", ".DV", ".FLV", ".M1V", ".M2TS",
   ".MKV", ".MOV", ".MPEG4", ".OMA", ".SPX", ".DAT", ".3G2", ".AVI", ".MPEG", ".MPG", ".FLAC",
   ".M4A", ".MP1", ".OGG", ".WAV", ".XM", ".3GP", ".WMV", ".AC3", ".ASF", ".MOD", ".MP2",
   ".MP3", ".MP4", ".WMA", ".MKA", ".M4P"
]

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
   
   init_features()
   init_media()

def init_config():
   random.seed()
   server_config = server_db["config"]
   this_ip = networking.get_my_ip();
   new_num = random.randint(1,999)
   current_date = datetime.datetime.now()
   day_string = str(current_date.day)
   month_string = str(current_date.month)
   id_string = str(current_date.year) + month_string.zfill(2) + day_string.zfill(2) + str(new_num)
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

def init_features():
   server_features = server_db["features"]
   feature_list = {"media_player":"false", "door_sensors":"false", "alarm":"false"}
   server_features.insert_one(feature_list)

def init_media():
   media_folder = server_db["media"]
   media_folder_list = media_folder["media_folder_list"]
   media_folder_list.insert_one({"path":"empty"})

def add_user(user_data):
   if(len(user_data["user_name"]) > 0):
      media_db = server_db["media"]
      user_list_db = media_db["user_list"]
      db_query = {"user_name":user_data["user_name"]}
      db_entry = {"user_name":user_data["user_name"]}
      results = user_list_db.find_one(db_query)
      if(results == None):
         user_list_db.insert_one(db_entry)
         return 1
      else:
         return "DB_ERR"
   else:
      return "DB_ERR"

def rem_user(user_data):
   media_db = server_db["media"]
   user_list_db = media_db["user_list"]
   db_query = {"user_name":user_data["user_name"]}
   removed = user_list_db.delete_one(db_query)
   if(removed):
      return 1
   else:
      return "DB_ERR"

def get_users():
   user_list = []
   media_db = server_db["media"]
   user_list_db = media_db["user_list"]
   users_db = user_list_db.find({},{"_id":0})
   db_list = list(users_db)
   if(db_list):
      for user_data in db_list:
         user_list.append(user_data["user_name"])
      return user_list
   else:
      return "DB_ERR"

def get_user_data(user):
   user_data_list = []
   media_db = server_db["media"]
   user_list_db = media_db["user_list"]
   db_query = {"user_name":user["user_name"]}
   user_data = user_list_db.find_one(db_query, {"_id":0})
   if(user_data != None):
      return user_data
   else:
      return "DB_ERR"

def add_media_folder(folder_data):
   path_obj = Path(folder_data["path"])
   if(path_obj.exists):
      media_folder = server_db["media"]
      media_folder_list = media_folder["media_folder_list"]
      path_string = path_obj.as_posix()
      db_query = {"path":path_string}
      db_entry = {"path":path_string, "data":"empty"}
      results = media_folder_list.find_one(db_query)
      if(results == None):
         media_folder_list.insert_one(db_entry)
         return 1
      else:
         return "DB_ERR"
   else:
      return "DB_ERR"

def rem_media_folder(folder_data):
   path_obj = Path(folder_data["path"])
   media_folder = server_db["media"]
   media_folder_list = media_folder["media_folder_list"]
   path_string = path_obj.as_posix()
   folder_query = {"path":path_string}
   removed = media_folder_list.delete_one(folder_query)
   if(removed):
      return 1
   else:
      return "DB_ERR"

def get_media_folders():
   folder_list = []
   db_media = server_db["media"]
   db_media_folder_list = db_media["media_folder_list"]
   db_folder_list = db_media_folder_list.find({},{"_id":0})
   db_list = list(db_folder_list)
   if(db_list):
      for folders in db_list:
         folder_list.append(folders["path"])
      return folder_list
   else:
      return "DB_ERR"

def get_media_data(folder_data):
   path_obj = Path(folder_data["path"])
   folder_list = []
   db_media = server_db["media"]
   db_media_folder_list = db_media["media_folder_list"]
   path_string = path_obj.as_posix()
   db_query = {"path":path_string}
   folder_data = db_media_folder_list.find_one(db_query, {"_id":0})
   if(folder_data != None):
      return folder_data
   else:
      return "DB_ERR"

# should redo this to read stored media index, then update and add/remove change anything that's needed
# currently it just does a whole new index and replaces stuff
def index_media_folder(folder_path):
   path_obj = Path(folder_path)
   db_media = server_db["media"]
   db_media_folder_list = db_media["media_folder_list"]
   path_string = path_obj.as_posix()
   db_query = {"path": path_string}
   if(path_obj):
      db_object = {}
      db_object["path"] = path_obj.as_posix()
      db_object["data"] = index_all_media(path_obj)
      return_data = db_media_folder_list.find_one_and_replace(db_query, db_object)
      return return_data

def index_all_media(current_path):
   media_dict = {}
   for data in current_path.iterdir():
      if(data.is_dir()):
         child_data = index_all_media(data)
         if(len(child_data) > 0):
            media_dict[data.stem] = child_data
      else:
         if(is_valid_file_type(data)):
            new_data = {}
            new_data["name"] = data.stem
            new_data["type"] = data.suffix
            pathname = data.as_posix()
            pathname = pathname[pathname.find(data.parts[1]):]
            new_data["path"] = pathname
            fullname = data.stem + data.suffix
            media_dict[fullname] = new_data
   return media_dict

def is_valid_file_type(data):
   valid_type = 0
   this_type = str(data.suffix)
   this_type = this_type.upper()
   for type in file_type_list:
      if(type == this_type):
         valid_type = 1
         break
   return valid_type

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
