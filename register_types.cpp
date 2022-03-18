#include "register_types.h"
#include "core/class_db.h"
#include "VDInventory.h"

void register_vdinventory_types(){
	ClassDB::register_class<VDInventory>();
    ClassDB::register_class<VDInventoryStock>();
}

void unregister_vdinventory_types() {}
